// std
#include <assert.h>
#include <math.h>

// OMLT
#include <DataAtlas.h>
#include <IDX.hpp>

using namespace SiCKL;

OMLT::DataAtlas::DataAtlas( IDX* in_Data )
{
	assert(in_Data != nullptr);

	_idx = in_Data;
}

OMLT::DataAtlas::~DataAtlas()
{

}

template<typename T>
class GetBatchSource : public SiCKL::Source
{
public:
	int32_t atlas_width;
	int32_t batch_width;
	int32_t batch_height;
	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<T>, atlas)
			CONST_DATA(Int, batch)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(T, output)
		END_OUT_DATA

		BEGIN_MAIN
			Int2 index = Index();

			Int batch_x = batch % atlas_width;
			Int batch_y = batch / atlas_width;

			output = atlas(batch_x * batch_width + index.X, batch_y * batch_height + index.Y);
		END_MAIN
	END_SOURCE
};

void OMLT::DataAtlas::Build( uint32_t in_BatchSize, uint32_t in_MaxPageSize )
{
	_max_page_size = int64_t(in_MaxPageSize) * 1024 * 1024;
	int64_t total_idx_size = _idx->GetRowLengthBytes() * _idx->GetRowCount();

	// the number of 'pixels' in the data file
	int64_t total_idx_cells = _idx->GetRowLength() * _idx->GetRowCount();

	_batch_width = _idx->GetRowLength();
	_batch_height = in_BatchSize;

	// we can load the whole thing into memory
	// a lot of this will probably transfer over to streaming 
	if(total_idx_size < _max_page_size)
	{
		// total number of batches in this atlas
		_total_batches = _idx->GetRowCount() / in_BatchSize;

		/// figuring out the atlas texture dimensions

		// number of minibatches stored horizontally in a page texture
		_atlas_width = ((uint32_t)sqrt((double)total_idx_cells)) / _idx->GetRowLength();
		if(_atlas_width == 0)
		{
			_atlas_width = 1;
		}
		// number of minibatches stored vertically in page texture
		_atlas_height = _total_batches / _atlas_width;
		if(_atlas_height * _atlas_width < _total_batches)
		{
			++_atlas_height;
		}

		uint32_t atlas_byte_width = _atlas_width * _idx->GetRowLengthBytes();
		uint32_t atlas_byte_size = atlas_byte_width * _atlas_height * _batch_height;
		uint8_t* atlas_buffer = (uint8_t*)malloc(atlas_byte_size);
		
		// make a copy of the head buffer
		uint8_t* head = atlas_buffer;
		uint32_t current_row = 0;
		uint32_t current_batch = 0;
		// over each row of minibatches
		for(uint32_t i = 0; i < _atlas_height; i++)
		{
			// over each minibatch in a row
			for(uint32_t j = 0; j < _atlas_width; j++)
			{
				if(current_batch < _total_batches)
				{
					// copy in each row 
					for(uint32_t k = 0; k < _batch_height; k++)
					{
						_idx->ReadRow(current_row++, atlas_buffer + k * atlas_byte_width);
					}
				}
				// empty spaces
				else
				{
					// zero out each row
					for(uint32_t k = 0; k < _batch_height; k++)
					{
						memset(atlas_buffer + k * atlas_byte_width, 0x00, _idx->GetRowLengthBytes());
					}
				}
				// increment batch we're on
				current_batch++;
				// scoot pointer over to the next minibatch
				atlas_buffer += _idx->GetRowLengthBytes();
			}
			// move pointer back to beginning of this row of minibatches
			atlas_buffer -= _batch_height * _idx->GetRowLengthBytes();
			// now move it up to the next row of minibatches
			atlas_buffer += _batch_height * atlas_byte_width;
		}
		// finally reset the pointer to head
		atlas_buffer = head;

		/// allocating the texture
		
		SiCKL::ReturnType::Type sickle_type = SiCKL::ReturnType::Invalid;
		switch(_idx->GetDataFormat())
		{
		case SInt32:
			sickle_type = SiCKL::ReturnType::Int;
			break;
		case Single:
			sickle_type = SiCKL::ReturnType::Float;
			break;
		}
		assert(sickle_type != SiCKL::ReturnType::Invalid);

		// now allocate our texture
		_atlas = SiCKL::OpenGLBuffer2D(_atlas_width * _idx->GetRowLength(), _atlas_height * _batch_height, sickle_type, atlas_buffer);
		_batch = SiCKL::OpenGLBuffer2D(_batch_width, _batch_height, sickle_type, nullptr);


		// and cleanup cpu-side memory
		free(atlas_buffer); 
		atlas_buffer = nullptr;
	}
	// gonna have to stream 
	else
	{
		assert(false);
	}

	SiCKL::Source* source = nullptr;
	if(_atlas.Type == SiCKL::ReturnType::Float)
	{
		auto fsource = new GetBatchSource<SiCKL::Float>();
		
		// set required data
		fsource->atlas_width = _atlas_width;
		fsource->batch_width = _idx->GetRowLength();
		fsource->batch_height = _batch_height;

		source = fsource;
	}
	else if(_atlas.Type == SiCKL::ReturnType::Int)
	{
		auto isource = new GetBatchSource<SiCKL::Int>();

		// set required data
		isource->atlas_width = _atlas_width;
		isource->batch_width = _idx->GetRowLength();
		isource->batch_height = _batch_height;

		source = isource;
	}

	source->Parse();
	
	SiCKL::OpenGLCompiler compiler;
	_texture_copy = compiler.Build(*source);
	_texture_copy->Initialize(_batch_width, _batch_height);

	///printf("%s\n", _texture_copy->GetSource().c_str());

	delete source;
	
	_current_batch = 0;
}

bool OMLT::DataAtlas::Next( SiCKL::OpenGLBuffer2D& inout_Buffer )
{
	_texture_copy->SetInput(0, _atlas);
	_texture_copy->SetInput(1, _current_batch);

	_texture_copy->BindOutput(0, _batch);
	
	_texture_copy->Run();

	// copy this guy in
	inout_Buffer = _batch;

	_current_batch = (_current_batch + 1) % _total_batches;

	return true;
}


