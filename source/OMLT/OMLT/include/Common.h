#pragma once

// c
#include <stdlib.h>

// extern
#include <SiCKL.h>

namespace OMLT
{
	// a general parsing method
	enum ModelType
	{
		MT_Invalid = -1,
		MT_NotSet = 0,
		MT_MultilayerPerceptron,
		MT_RestrictedBoltzmannMachine,

		MT_MLP = MT_MultilayerPerceptron,
		MT_RBM = MT_RestrictedBoltzmannMachine,
	};

	struct Model
	{
		ModelType type;
		union
		{
			void* ptr;
			class MultilayerPerceptron* mlp;
			class RestrictedBoltzmannMachine* rbm;
		};
		Model() : type(MT_NotSet), ptr(nullptr) {}
	};

	// true on successful parse, false on failure
	bool FromJSON(const std::string& in_json, Model& out_model);

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

	/// Some Common SiCKL methods
	extern void NextSeed(const SiCKL::UInt& in_seed, SiCKL::UInt& out_seed);
	extern void NextFloat(const SiCKL::UInt& in_seed, SiCKL::UInt& out_seed, SiCKL::Float& out_float);
	extern void NextGaussian(const SiCKL::UInt& in_seed, SiCKL::UInt& out_seed, SiCKL::Float& out_gaussian);
	extern SiCKL::Float Sigmoid(const SiCKL::Float& in_x); 
}

