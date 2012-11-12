#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <memory>

// get a gaussian distributed random variable
extern double NextGaussian();
extern float CalcMeanAbsValue(float* buffer, int count);
extern float CalcMeanAbsValueParallel(float* buffer, int count);

template<typename T>
void swap(T& a, T& b)
{
	T temp = a;
	a = b;
	b = temp;
}

template<typename T>
void shuffle(T* in_buffer, uint32_t in_count)
{
	// big ol' mersenne prime
	const uint64_t prime = uint64_t(1 << 31) - 1;

	for(uint32_t k = in_count - 1; k > 1; k--)
	{
		// RAND_MAX is implementation dependent, so we multiply by
		// big prime to verify it will be larger than largest uint32_t
		uint32_t index = (uint64_t(rand()) * prime) % k;

		swap(in_buffer[k], in_buffer[index]);
	}
}

template <typename T, uint32_t Alignment=16>
class AlignedMemoryBlock
{
public:
	AlignedMemoryBlock() : _pointer(0), _head(0), _block_count(0), _user_size(0)
	{ }
	~AlignedMemoryBlock()
	{
		Release();
	}

	void Acquire(uint32_t in_count)
	{
		Release();

		uint32_t alignment_offset = Alignment;
		uint32_t byte_count = in_count * sizeof(T);
		uint32_t back_padding = byte_count % Alignment == 0 ? 0 : Alignment;

		_total_size = alignment_offset + byte_count + back_padding;
		_user_size = byte_count + back_padding;

		_head = malloc(_total_size);
		memset(_head, 0, _total_size);

		uint32_t head_offset = (Alignment - ((uint32_t)_head % Alignment)) % Alignment;

		_pointer = (T*)((uint32_t)_head + head_offset);

		_block_count = (byte_count + back_padding) / Alignment;
	}

	void Release()
	{
		if(_head != 0)
		{
			free(_head);
		}

		_head = 0;
		_pointer = 0;
		_block_count = _total_size = _user_size = 0;
	}

	operator T*()
	{
		return _pointer;
	}

	// number of blocks of size Alignment
	inline uint32_t BlockCount() const
	{
		return _block_count;
	}
	
	inline uint32_t Size() const
	{
		return _user_size;
	}

private:
	T* _pointer;
	void* _head;
	uint32_t _block_count;
	uint32_t _total_size;
	uint32_t _user_size;
};

