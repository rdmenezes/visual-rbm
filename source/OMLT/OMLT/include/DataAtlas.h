#include <stdint.h>
#include <SiCKL.h>

namespace OMLT
{
	class IDX;

	class DataAtlas
	{
	public:
		DataAtlas(IDX* in_Data);
		~DataAtlas();

		// constructs internal texture page used to store the data
		// in_BatchSize -> number of rows in buffer returned by next
		// in_MaxPageSize -> maximum size of _atlas in megabytes
		void Build(uint32_t in_BatchSize, uint32_t in_MaxPageSize);
		
		bool Next(SiCKL::OpenGLBuffer2D& inout_Buffer);
	private:
		SiCKL::OpenGLBuffer2D _atlas;
		SiCKL::OpenGLBuffer2D _batch;
		// in terms of minibatches
		uint32_t _atlas_width;
		uint32_t _atlas_height;
		uint32_t _batch_width;
		uint32_t _batch_height;
		uint32_t _total_batches;
		int32_t _current_batch;

		SiCKL::OpenGLProgram* _texture_copy;

		int64_t _max_page_size;
		IDX* _idx;
	};
}