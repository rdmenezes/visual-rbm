#pragma once

// c
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
// for aligned malloc/free
#include <malloc.h>

// OMLT
#include "Enums.h"

namespace OMLT
{
	bool ReadTextFile(const std::string& in_filename, std::string& out_text);

	/// Util Methods
	template<typename T, size_t N>
	size_t ArraySize(const T (&)[N])
	{
		return N;
	};

	template<typename T>
	void SafeDelete(T*& ptr)
	{
		delete ptr;
		ptr = nullptr;
	}

	inline void* AlignedMalloc(size_t count, size_t alignment)
	{
		return _aligned_malloc(count, alignment);
	}

	inline void AlignedFree(void* ptr)
	{
		_aligned_free(ptr);
	}

	// returns number of 4 float blocks required to store the given number of floats
	inline uint32_t BlockCount(const uint32_t float_count)
	{
		return (float_count % 4 == 0 ? float_count / 4 : float_count / 4 + 1);
	}

	/// Allocates aligned memory for us
	template <typename T, uint32_t Alignment=16>
	class AlignedMemoryBlock
	{
	public:
		AlignedMemoryBlock() : _pointer(0), _head(0), _block_count(0), _user_size(0), _total_size(0)
		{ }
		~AlignedMemoryBlock()
		{
			Release();
		}

		void Acquire(uint32_t in_count)
		{
			uint32_t alignment_offset = Alignment;
			uint32_t byte_count = in_count * sizeof(T);
			uint32_t back_padding = byte_count % Alignment == 0 ? 0 : Alignment;

			const uint32_t total_size = alignment_offset + byte_count + back_padding;
			if(total_size == _total_size)
			{
				return;
			}
			
			Release();

			_total_size = total_size;
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

	struct FeatureMap
	{
	public:
		FeatureMap(uint32_t input_length, uint32_t feature_count);
		~FeatureMap();
		void CalcFeatureVector(const float* input_vector, float* output_vector, ActivationFunction_t function) const;
		
		inline float* biases() {return _biases;};
		inline float* feature(uint32_t k) {assert(k < feature_count); return _features[k];}

		inline const float* biases() const {return _biases;};
		inline const float* feature(uint32_t k) const {assert(k < feature_count); return _features[k];}


		const uint32_t input_length;
		const uint32_t feature_count;
	private:

		const uint32_t _feature_blocks;
		const uint32_t _input_blocks;

		float* _biases;
		float** _features;
		mutable float* _accumulations;
	};
}

