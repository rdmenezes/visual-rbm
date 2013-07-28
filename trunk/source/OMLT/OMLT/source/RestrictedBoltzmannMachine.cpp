// c
#include <assert.h>

// windows
#include <malloc.h>
#include <intrin.h>

// OMLT
#include "Common.h"
#include "RestrictedBoltzmannMachine.h"

namespace OMLT
{
	inline static void allocate_features(float**& ptr, const uint32_t width, const size_t size)
	{
		ptr = new float*[width];
		for(uint32_t i = 0; i < width; i++)
		{
			ptr[i] = (float*)_aligned_malloc(size, 16);
		}
	}

	inline static void free_features(float** const buff, uint32_t width)
	{
		for(uint32_t i = 0; i < width; i++)
		{
			_aligned_free(buff[i]);
		}

		delete[] buff;
	}

	RestrictedBoltzmannMachine::RestrictedBoltzmannMachine( uint32_t in_visible_count, uint32_t in_hidden_count, ActivationFunction_t in_visible_type, ActivationFunction_t in_hidden_type )
		: visible_count(in_visible_count)
		, hidden_count(in_hidden_count)
		, visible_type(in_visible_type)
		, hidden_type(in_hidden_type)
	{
		// precompute allocation sizes
		const size_t visible_allocsize = sizeof(float) * 4 * BlockCount(visible_count);
		const size_t hidden_allocsize = sizeof(float) * 4 * BlockCount(hidden_count);

		// biases
		visible_biases = (float*)_aligned_malloc(visible_allocsize, 16);
		hidden_biases = (float*)_aligned_malloc(hidden_allocsize, 16);

		// weights
		allocate_features(hidden_features, hidden_count, visible_allocsize);
		allocate_features(visible_features, visible_count, hidden_allocsize);

		// aligned scratch space
		_visible_buffer = (float*)_aligned_malloc(visible_allocsize, 16);
		_hidden_buffer = (float*)_aligned_malloc(hidden_allocsize, 16);

		memset(_visible_buffer, 0x00, visible_allocsize);
		memset(_hidden_buffer, 0x00, hidden_allocsize);
	}

	RestrictedBoltzmannMachine::~RestrictedBoltzmannMachine()
	{
		_aligned_free(visible_biases);
		_aligned_free(hidden_biases);

		free_features(hidden_features, hidden_count);
		free_features(visible_features, visible_count);
	}

	static void calc_activation(const float* biases, float* inout_buffer, const uint32_t blocks, ActivationFunction_t func)
	{
		switch(func)
		{
		case ActivationFunction::Linear:
			break;
		case ActivationFunction::RectifiedLinear:
			extern __m128 _mm_rectifiedlinear_ps(__m128 x0);
			for(uint32_t i = 0; i < blocks; i++)
			{
				// load bias and accumulation
				__m128 bias = _mm_load_ps(biases);
				__m128 accum = _mm_load_ps(inout_buffer);
				
				// add bias, calculate rectified linear
				accum = _mm_add_ps(accum, bias);
				__m128 activ = _mm_rectifiedlinear_ps(accum);

				_mm_store_ps(inout_buffer, activ);

				biases += 4;
				inout_buffer += 4;
			}
			break;
		case ActivationFunction::Sigmoid:
			extern __m128 _mm_sigmoid_ps(__m128 x0);
			for(uint32_t i = 0; i < blocks; i++)
			{
				// load bias and accumulation
				__m128 bias = _mm_load_ps(biases);
				__m128 accum = _mm_load_ps(inout_buffer);

				// add bias, calculate sigmoid
				accum = _mm_add_ps(accum, bias);
				__m128 activ = _mm_sigmoid_ps(accum);

				_mm_store_ps(inout_buffer, activ);

				biases += 4;
				inout_buffer += 4;
			}
			break;
		}
	}

	static float calc_accumulation(const float* input_vector, const float* feature_vector, const uint32_t blockcount)
	{
		// calculate dot product between input ad feature vector
		__m128 dp = _mm_setzero_ps();
		for(uint32_t i = 0; i < blockcount; i++)
		{
			__m128 v = _mm_load_ps(input_vector);
			__m128 w = _mm_load_ps(feature_vector);

			dp = _mm_add_ps(dp, _mm_mul_ps(v, w));

			input_vector += 4;
			feature_vector += 4;
		}

		dp = _mm_hadd_ps(dp , dp);
		dp = _mm_hadd_ps(dp , dp);

		// store it back aligned output buffer
		float result;
		_mm_store_ss(&result, dp);

		return result;
	}

	static void calc_feature_map(
		const float* input_buffer, float* output_buffer, 
		float* input_scratch, float* output_scratch, 
		const uint32_t input_count, const uint32_t output_count,
		const float* biases, float** const features, const ActivationFunction_t activation)
	{
		assert(((intptr_t)input_scratch % 16) == 0);
		assert(((intptr_t)output_scratch % 16) == 0);

		memcpy(input_scratch, input_buffer, sizeof(float) * input_count);
		const uint32_t input_blockcount = BlockCount(input_count);
		
		// set dangling floats to 0 so we don't screw up any calculations
		const size_t dangling_bytes = (input_blockcount * 4 * sizeof(float)) - input_count * sizeof(float);
		if(dangling_bytes > 0)
		{
			memset(input_scratch + input_count, 0x00, dangling_bytes);
		}

		for(uint32_t j = 0; j < output_count; j++)
		{
			output_scratch[j] = calc_accumulation(input_scratch, features[j], input_blockcount);
		}

		calc_activation(biases, output_scratch, BlockCount(output_count), activation);

		// copy from aligned scratch space to output buffer
		memcpy(output_buffer, output_scratch, output_count * sizeof(float));
	}

	void RestrictedBoltzmannMachine::CalcHidden( const float* in_visible, float* out_hidden ) const
	{
		calc_feature_map(
			in_visible, out_hidden, 
			_visible_buffer, _hidden_buffer, 
			visible_count, hidden_count, 
			hidden_biases, hidden_features, hidden_type);
	}

	void RestrictedBoltzmannMachine::CalcVisible( const float* in_hidden, float* out_visible ) const
	{
		calc_feature_map(
			in_hidden, out_visible, 
			_hidden_buffer, _visible_buffer, 
			hidden_count, visible_count,
			visible_biases, visible_features, visible_type);
	}

	float RestrictedBoltzmannMachine::CalcFreeEnergy( const float* in_visible ) const
	{
		return 0.0f;
	}

	std::string RestrictedBoltzmannMachine::ToJSON() const
	{
		return "";
	}

	RestrictedBoltzmannMachine* RestrictedBoltzmannMachine::FromJSON( const std::string& in_JSON )
	{
		return nullptr;
	}


}