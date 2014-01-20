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
		void Initialize(uint32_t in_BatchSize, uint32_t in_MaxPageSize);
		
		// resets data atlas to read from beginning of backing IDX
		void Reset();
		
		// returns size of each minibatch
		uint32_t GetBatchSize() const
		{
			return _batch_height;
		}

		bool Next(SiCKL::OpenGLBuffer2D& inout_Buffer);
		inline uint32_t GetTotalBatches() const {return _total_batches;}
		inline bool GetIsInitialized() const {return _initialized;}
	private:
		void PopulateAtlasBuffer();

		SiCKL::OpenGLBuffer2D _atlas;
		SiCKL::OpenGLBuffer2D _batch;
		
		bool _initialized;

		// flag is set when we need to stream in
		// more data at the end of the page
		bool _streaming;

		// our CPU side buffer for copying in IDX
		uint8_t* _atlas_buffer;
		// width of our atlas in bytes
		uint32_t _atlas_byte_width;
		// width of our atlas in batches
		uint32_t _atlas_batch_width;
		// height of our atlas in batches
		uint32_t _atlas_batch_height;
		// width of a batch in 'cells'
		uint32_t _batch_width;
		// height of a batch in 'cells'
		uint32_t _batch_height;

		// the total number of batches in the IDX
		uint32_t _total_batches;
		// what bathc will we give in Next()
		int32_t _current_batch;
		// what row to start loading from in PopulateAtlas)_
		uint32_t _current_row;

		// our copy shader
		SiCKL::OpenGLProgram* _texture_copy;

		// our backing data file
		IDX* _idx;
	};
}