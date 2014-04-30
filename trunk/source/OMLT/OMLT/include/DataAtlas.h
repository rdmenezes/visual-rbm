#include <stdint.h>
#include <SiCKL.h>

namespace OMLT
{
	class IDX;

	class DataAtlas
	{
	public:
		// in_AtlasSize -> size of the atlas in megabytes
		DataAtlas(uint32_t in_atlas_size);
		~DataAtlas();
		bool Initialize(IDX* in_data, uint32_t in_minibatch_size);
		bool Next(SiCKL::OpenGLBuffer2D& inout_minibatch);
		uint32_t GetTotalBatches() { return _total_rows / _minibatch_size; }
	private:
		void PopulateAtlas();

		// our atlas texture on the GPU
		SiCKL::OpenGLBuffer2D _atlas;
		// CPU side buffer we keep around for streaming
		float* _atlas_buffer;
		// number of floats per dimension of _atlas
		uint32_t _atlas_width;
		// size of our atlas (float count)
		uint32_t _atlas_size;

		// our backing IDX file
		IDX* _idx;
		// length of a single data row
		uint32_t _row_length;
		// maximum number of rows we can store in our atlas
		uint32_t _max_rows;
		// current row at head of our atlas
		uint32_t _current_row;
		// total number of rows in our IDX
		uint32_t _total_rows;
		// total number of batches in our IDX
		uint32_t _total_batches;
		
		// flag determines if we are streaming from an IDX file
		// or if all of our data is loaded
		bool _streaming;
		// number of rows in a mini batch
		uint32_t _minibatch_size;
		// batches per page
		uint32_t _batches_per_page;

		// the current minibatch we're on
		uint32_t _current_batch;
		// small 2d texture populated in Next();
		SiCKL::OpenGLBuffer2D _batch;
		// shader that copies data from our atlas to the batch
		SiCKL::OpenGLProgram* _texture_copy;
	};
}