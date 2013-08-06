// std
#include <stdint.h>
#include <random>
#include <iostream>

// windows
#include <malloc.h>
#include <intrin.h>
#include <windows.h>

namespace OMLT
{
	extern __m128 _mm_sigmoid_ps(__m128 x0);
	extern __m128 _mm_ln_1_plus_e_x_ps(__m128 x);
}

inline float sign(const float x)
{
	union
	{
		float f;
		uint32_t u;
	};
	f = x;
	uint32_t abs = u & 0x80000000;
	f = 1.0f;
	u = abs | u;
	return f;
}

bool TestSigmoid(int argc, char** argv)
{
	std::mt19937_64 random;
	random.seed(1);
	std::uniform_real<float> uniform(-4.0f, 4.0f);

	const uint32_t block_count = 50000;
	float* buffer = (float*)_aligned_malloc(sizeof(float) * 4 * block_count, 16);

	float* simd_result = (float*)_aligned_malloc(sizeof(float) * 4 * block_count, 16);
	float* normal_result = (float*)_aligned_malloc(sizeof(float) * 4 * block_count, 16);
	float* slow_result = (float*)_aligned_malloc(sizeof(float) * 4 * block_count, 16);
	uint64_t simd_start;
	uint64_t simd_end;
	uint64_t normal_end;
	uint64_t slow_end;

	for(uint32_t k = 0; k < block_count * 4; k++)
	{
		buffer[k] = uniform(random);
	}

	float* buffer_head = buffer;
	float* simd_head = simd_result;

	QueryPerformanceCounter((LARGE_INTEGER*)&simd_start);

	// simd approximate sigmoid calculation
	for(uint32_t k = 0; k < block_count; k++)
	{
		__m128 in = _mm_load_ps(buffer_head);
		__m128 sigm = OMLT::_mm_sigmoid_ps(in);
		_mm_store_ps(simd_head, sigm);
		buffer_head += 4;
		simd_head += 4;
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&simd_end);

	// fpu approximate sigmoid calculation
	for(uint32_t k = 0; k < block_count * 4; k++)
	{
		float& x = buffer[k];
		float diff = (std::abs(x) - 4.0f);

		float s = sign(x);

		normal_result[k] = -1.0f/32.0f * diff * diff * s + (s + 1.0f) / 2.0f;
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&normal_end);

	// fpu accurate slow calculation
	for(uint32_t k = 0; k < block_count * 4; k++)
	{
		slow_result[k] = 1.0f / (1.0f + std::expf(-buffer[k]));
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&slow_end);

	std::cout << "Performance: 1.0 / (1.0 + exp(-x))" << std::endl;
	std::cout << "SIMD  " << (simd_end - simd_start) << std::endl;
	std::cout << "Fast  " << (normal_end - simd_end) << std::endl;
	std::cout << "Slow  " << (slow_end - normal_end) << std::endl;

	// calc error
	float abs_err = 0.0f;
	for(uint32_t k = 0; k < block_count * 4; k++)
	{
		float diff = simd_result[k] - slow_result[k];
		abs_err += std::abs(diff);
	}

	abs_err /= float(block_count * 4);
	std::cout << "Mean Absolute Error: " << abs_err << std::endl;

	if(abs_err > 0.011034)
	{
		std::cout << "Mean absolute error is too large";
		return false;
	}

	return true;
}

