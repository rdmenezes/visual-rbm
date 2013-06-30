#include <stdint.h>
#include <SiCKL.h>

namespace OMLT
{
	class IDX;

	class DataAtlas
	{
	public:
		DataAtlas(IDX* in_Data);
		// constructs internal texture page used to store the data
		// in_BatchSize -> number of rows in buffer returned by next
		// in_MaxPageSize -> maximum size of _atlas in megabytes
		void Build(uint32_t in_BatchSize, uint32_t in_MaxPageSize);
		
		bool Next(SiCKL::OpenGLBuffer2D& inout_Buffer);
	private:
		SiCKL::OpenGLBuffer2D _atlas;
		SiCKL::OpenGLProgram* _texture_copy;
	};
}