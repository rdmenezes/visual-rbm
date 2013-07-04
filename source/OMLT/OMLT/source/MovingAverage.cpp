//std
#include <stdlib.h>
#include <string.h>

// OMLT
#include "MovingAverage.h"

namespace OMLT
{
	MovingAverage* MovingAverage::Build( const uint32_t in_size )
	{
		MovingAverage* result = (MovingAverage*)malloc(sizeof(MovingAverage) + sizeof(float) * in_size);
		memset(&result->_value_buffer[0], 0x00, sizeof(float) * in_size);
		result->_size = in_size;
		result->_index = 0;
		result->_sum = 0.0f;
		
		return result;
	}

	void MovingAverage::operator delete(void* in_ptr)
	{
		free(in_ptr);
	}


	float MovingAverage::GetAverage() const
	{
		return _sum / (float)_size;
	}

	void MovingAverage::AddEntry( float in_val )
	{
		_sum -= _value_buffer[_index];
		_value_buffer[_index] = in_val;
		_sum += in_val;

		_index = (_index + 1) % _size;
	}

}