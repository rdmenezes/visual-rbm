// std
#include <assert.h>
#include <math.h>

// OMLT
#include <DataAtlas.h>
#include <IDX.hpp>

using namespace SiCKL;

OMLT::DataAtlas::DataAtlas( IDX* in_Data )
	: _streaming(false)
	, _atlas_buffer(nullptr)
	, _atlas_byte_width(0)
	, _atlas_batch_width(0)
	, _atlas_batch_height(0)
	, _batch_width(0)
	, _batch_height(0)
	, _total_batches(0)
	, _current_batch(0)
	, _current_row(0)
	, _texture_copy(nullptr)
	, _idx(in_Data)
{
	assert(_idx != nullptr);
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
	// dimension of each batch
	_batch_width = _idx->GetRowLength();
	_batch_height = in_BatchSize;

	// memory conversion
	int64_t max_page_size = int64_t(in_MaxPageSize) * 1024 * 1024;
	int64_t total_idx_size = _idx->GetRowLengthBytes() * _idx->GetRowCount();

	// get the number of atlas pages needed for the whole IDX given
	// the memory constraints
	uint32_t page_count = (uint32_t)(total_idx_size / max_page_size);
	if(page_count * max_page_size < total_idx_size)
	{
		++page_count;
	}

	if(page_count > 1)
	{
		_streaming = true;
	}

	_total_batches = _idx->GetRowCount() / _batch_height;
	// how many batches will we load per page?
	uint32_t batches_per_page = _total_batches / page_count;
	uint32_t page_cells = batches_per_page * _batch_height * _batch_width;

	/// figuring out the atlas texture dimensions

	// number of batches stored horizontally in a page texture
	_atlas_batch_width = ((uint32_t)sqrt((double)page_cells)) / _idx->GetRowLength();
	if(_atlas_batch_width == 0)
	{
		_atlas_batch_width = 1;
	}
	// number of minibatches stored vertically in page texture
	_atlas_batch_height = batches_per_page / _atlas_batch_width;
	if(_atlas_batch_height * _atlas_batch_width < batches_per_page)
	{
		++_atlas_batch_height;
	}

	_atlas_byte_width = _atlas_batch_width * _idx->GetRowLengthBytes();
	uint32_t atlas_byte_size = _atlas_byte_width * _atlas_batch_height * _batch_height;
	_atlas_buffer = (uint8_t*)malloc(atlas_byte_size);
		
	_current_row = 0;

	// fill in our cpu side atlas
	PopulateAtlasBuffer();

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
	_atlas = SiCKL::OpenGLBuffer2D(_atlas_batch_width * _idx->GetRowLength(), _atlas_batch_height * _batch_height, sickle_type, _atlas_buffer);
	_batch = SiCKL::OpenGLBuffer2D(_batch_width, _batch_height, sickle_type, nullptr);

	SiCKL::Source* source = nullptr;
	if(_atlas.Type == SiCKL::ReturnType::Float)
	{
		auto fsource = new GetBatchSource<SiCKL::Float>();
		
		// set required data
		fsource->atlas_width = _atlas_batch_width;
		fsource->batch_width = _batch_width;
		fsource->batch_height = _batch_height;

		source = fsource;
	}
	else if(_atlas.Type == SiCKL::ReturnType::Int)
	{
		auto isource = new GetBatchSource<SiCKL::Int>();

		// set required data
		isource->atlas_width = _atlas_batch_width;
		isource->batch_width = _batch_width;
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
	// copy data from atlas to buffer
	_texture_copy->SetInput(0, _atlas);
	_texture_copy->SetInput(1, _current_batch);
	_texture_copy->BindOutput(0, _batch);
	_texture_copy->Run();

	// copy this guy in
	inout_Buffer = _batch;

	// handle streaming
	if(_streaming)
	{
		// check to see if we need to bring in more IDX data 
		_current_batch++;
		uint32_t batches_per_page = _atlas_batch_width * _atlas_batch_height;
		if(_current_batch == batches_per_page)
		{
			// fill in our CPU side buffer
			PopulateAtlasBuffer();
			// send it over to GPU
			_atlas.SetData(_atlas_buffer);

			// and reset the batch index to 0
			_current_batch = 0;
		}
	}

	return true;
}

void OMLT::DataAtlas::PopulateAtlasBuffer()
{	
	// make a copy of the head buffer
	uint8_t* head = _atlas_buffer;
	uint32_t current_batch = 0;
	// over each row of batches
	for(uint32_t i = 0; i < _atlas_batch_height; i++)
	{
		// over each batch in a row
		for(uint32_t j = 0; j < _atlas_batch_width; j++)
		{
			// only want to leave empty slots if we are not streaming
			if(!_streaming && current_batch >= _total_batches)
			{
				// zero out each row
				for(uint32_t k = 0; k < _batch_height; k++)
				{
					memset(_atlas_buffer + k * _atlas_byte_width, 0x00, _idx->GetRowLengthBytes());
				}
			}
			else
			{
				// copy in each row 
				for(uint32_t k = 0; k < _batch_height; k++)
				{
					_idx->ReadRow(_current_row, _atlas_buffer + k * _atlas_byte_width);
					_current_row = (_current_row + 1) % _idx->GetRowCount();
				}
			}

			// increment batch we're on
			current_batch++;
			// scoot pointer over to the next batch
			_atlas_buffer += _idx->GetRowLengthBytes();
		}
		// move pointer back to beginning of this row of batches
		_atlas_buffer -= _atlas_batch_width * _idx->GetRowLengthBytes();
		// now move it forward to the next row of batches
		_atlas_buffer += _batch_height * _atlas_byte_width;
	}
	// finally reset the pointer to head
	_atlas_buffer = head;
}


