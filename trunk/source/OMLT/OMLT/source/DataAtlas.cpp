// std
#include <assert.h>
#include <math.h>

// OMLT
#include <DataAtlas.h>
#include <IDX.hpp>

using namespace SiCKL;

OMLT::DataAtlas::DataAtlas(uint32_t in_atlas_size)
	: _idx(nullptr)
	, _row_length(-1)
	, _max_rows(-1)
	, _current_row(-1)
	, _total_rows(-1)
	, _streaming(false)
	, _minibatch_size(-1)
	, _texture_copy(nullptr)
{
	// to bytes
	in_atlas_size *= 1024*1024;
	const uint32_t float_count = in_atlas_size/sizeof(float);
	_atlas_width = (int)sqrt((double)float_count);
	_atlas_size = _atlas_width * _atlas_width;

	_atlas_buffer = new float[_atlas_size];
	memset(_atlas_buffer, 0x00, sizeof(float) * _atlas_size);

	_atlas = OpenGLBuffer2D(_atlas_width, _atlas_width, ReturnType::Float, _atlas_buffer);
}

OMLT::DataAtlas::~DataAtlas()
{
	delete[] _atlas_buffer;
	delete _idx;
	delete _texture_copy;
}

bool OMLT::DataAtlas::Initialize(IDX* in_data, uint32_t in_minibatch_size)
{
	if(_idx != in_data)
	{
		delete _idx;
		_idx = in_data;
	}
	
	_minibatch_size = in_minibatch_size;

	_row_length = _idx->GetRowLength();
	_max_rows = _atlas_size / _row_length;
	_current_row = 0;
	_total_rows = _idx->GetRowCount();
	_total_batches = _total_rows / _minibatch_size;

	_max_rows = _atlas_size / _row_length;
	if(_max_rows < _total_rows)
	{
		_streaming = true;
	}

	
	if(_streaming)
	{
		_batches_per_page = _atlas_size / (_minibatch_size * _row_length);
	}
	else
	{
		_batches_per_page = _total_batches;
	}

	// allocate batch texture
	_batch = OpenGLBuffer2D(_row_length, _minibatch_size, ReturnType::Float, nullptr);

	PopulateAtlas();

	class CopyDataSource : public SiCKL::Source
	{
	public:
		int32_t atlas_width;
		int32_t row_length;
		int32_t minibatch_size;

		BEGIN_SOURCE
			BEGIN_CONST_DATA
				CONST_DATA(Buffer2D<Float>, atlas)
				CONST_DATA(Int, batch)
			END_CONST_DATA

			BEGIN_OUT_DATA
				OUT_DATA(Float, output)
			END_OUT_DATA

			BEGIN_MAIN
				Int2 index = Index();

				Int flat_index = (batch * row_length * minibatch_size) + index.X + (index.Y * row_length);

				Int x = flat_index % atlas_width;
				Int y = flat_index / atlas_width;

				output = atlas(x, y);
			END_MAIN
		END_SOURCE
	} source;

	source.atlas_width = _atlas_width;
	source.row_length = _row_length;
	source.minibatch_size = _minibatch_size;

	source.Parse();
	OpenGLCompiler comp;
	delete _texture_copy;
	_texture_copy = comp.Build(source);
	_texture_copy->Initialize(_row_length, _minibatch_size);

	return true;
}

void OMLT::DataAtlas::PopulateAtlas()
{
	float* atlas_head = _atlas_buffer;

	for(uint32_t  k = 0; k < _batches_per_page; k++)
	{
		for(uint32_t j = 0; j < _minibatch_size; j++)
		{
			_idx->ReadRow(_current_row % _total_rows, atlas_head);
			_current_row = (_current_row + 1) % _total_rows;
			atlas_head += _row_length;
		}
	}

	_current_batch = 0;

	_atlas.SetData(_atlas_buffer);
}

bool OMLT::DataAtlas::Next(SiCKL::OpenGLBuffer2D& inout_minibatch)
{
	_texture_copy->SetInput(0, _atlas);
	_texture_copy->SetInput(1, (int32_t)_current_batch);
	_texture_copy->BindOutput(0, _batch);
	_texture_copy->Run();

	inout_minibatch = _batch;

	_current_batch = (_current_batch + 1) % _batches_per_page;
	if(_current_batch == 0 && _streaming)
	{
		PopulateAtlas();
	}

	return true;
}
