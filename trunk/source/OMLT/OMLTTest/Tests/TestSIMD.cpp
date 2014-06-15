// std
#include <stdint.h>
#include <random>
#include <iostream>
#include <cmath>

// windows
#include <malloc.h>
#include <intrin.h>
#include <windows.h>

namespace OMLT
{
	extern __m128 _mm_sigmoid_ps(__m128 x0);
	extern __m128 _mm_ln_1_plus_e_x_ps(__m128 x);
	extern __m128 _mm_exp_ps(__m128 x);
	extern __m128 _mm_abs_ps(__m128 x);
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

bool VerifySigmoid(int argc, char** argv)
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

	std::cout << "Verifying negative edge cases" << std::endl;

	// verify edges work properly
	{
		float neg_vals[4] = {-16, -8, -6, -4};
		float results[4];

		_mm_storeu_ps(results, OMLT::_mm_sigmoid_ps(_mm_loadu_ps(neg_vals)));

		for(uint32_t k = 0; k < 4; k++)
		{
			if(results[k] != 0.0f)
			{
				std::cout << "Calculated sigmoid( " <<neg_vals[k] << ") == " << results[k] << std::endl;
				return false;
			}
		}
	}

	std::cout << "Verifying positive edge cases" << std::endl;

	// verify edges work properly
	{
		float pos_vals[4] = {4, 6, 8, 16};
		float results[4];

		_mm_storeu_ps(results, OMLT::_mm_sigmoid_ps(_mm_loadu_ps(pos_vals)));

		for(uint32_t k = 0; k < 4; k++)
		{
			if(results[k] != 1.0f)
			{
				std::cout << "Calculated sigmoid( " <<pos_vals[k] << ") == " << results[k] << std::endl;
				return false;
			}
		}
	}

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
		const float& x = buffer[k];
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

	if(abs_err > 0.011034f)
	{
		std::cout << "Mean absolute error is too large";
		return false;
	}

	return true;
}

bool VerifyLn1PlusEx(int argc, char** argv)
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

	std::cout << "Verifying negative edge cases" << std::endl;

	// verify edges work properly
	{
		float neg_vals[4] = {-16, -8, -6, -4};
		float results[4];

		_mm_storeu_ps(results, OMLT::_mm_ln_1_plus_e_x_ps(_mm_loadu_ps(neg_vals)));

		for(uint32_t k = 0; k < 4; k++)
		{
			if(results[k] != 0.0f)
			{
				std::cout << "Calculated sigmoid( " <<neg_vals[k] << ") == " << results[k] << std::endl;
				return false;
			}
		}
	}

	std::cout << "Verifying positive edge cases" << std::endl;

	// verify edges work properly
	{
		float pos_vals[4] = {4, 6, 8, 16};
		float results[4];

		_mm_storeu_ps(results, OMLT::_mm_ln_1_plus_e_x_ps(_mm_loadu_ps(pos_vals)));

		for(uint32_t k = 0; k < 4; k++)
		{
			if(results[k] != pos_vals[k])
			{
				std::cout << "Calculated sigmoid( " <<pos_vals[k] << ") == " << results[k] << std::endl;
				return false;
			}
		}
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&simd_start);

	// simd approximate sigmoid calculation
	for(uint32_t k = 0; k < block_count; k++)
	{
		__m128 in = _mm_load_ps(buffer_head);
		__m128 sigm = OMLT::_mm_ln_1_plus_e_x_ps(in);
		_mm_store_ps(simd_head, sigm);
		buffer_head += 4;
		simd_head += 4;
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&simd_end);

	// fpu approximate sigmoid calculation
	for(uint32_t k = 0; k < block_count * 4; k++)
	{
		const float x = max(-4.0, buffer[k]);

		float y0 = (64.0f + x * (12 + (4 + x) - x*x * sign(x))) * (1.0f / 96.0f);

		float sign_4_sans_x = sign(4.0f - x);

		float y1 = (y0 + y0 * sign_4_sans_x) * (1.0f / 2.0f) + (x - x * sign_4_sans_x) * (1.0f / 2.0f);

		normal_result[k] = y1;
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&normal_end);

	// fpu accurate slow calculation
	for(uint32_t k = 0; k < block_count * 4; k++)
	{
		slow_result[k] = std::logf(1.0f + std::expf(buffer[k]));
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&slow_end);

	std::cout << "Performance: ln(1.0 + exp(x))" << std::endl;
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

	if(abs_err > 0.0343875f)
	{
		std::cout << "Mean absolute error is too large";
		return false;
	}

	return true;
}


bool VerifyExp(int argc, char** argv)
{
	std::mt19937_64 random;
	random.seed(1);
	// we're only using exp for softmax, whose accumlations are going to be in this range
	std::uniform_real<float> uniform(-87.336540f, 0.0f);	

	const uint32_t block_count = 500000;

	float* buffer = (float*)_aligned_malloc(sizeof(float) * 4 * block_count, 16);
	float* simd_result = (float*)_aligned_malloc(sizeof(float) * 4 * block_count, 16);
	float* slow_result = (float*)_aligned_malloc(sizeof(float) * 4 * block_count, 16);

	// initialize
	for(uint32_t k = 0; k < block_count * 4; k++)
	{
		float f = uniform(random);
		slow_result[k] = std::exp(f);
		buffer[k] = f;
	}

	// calculate results
	float* buffer_head = buffer;
	float* simd_head = simd_result;
	for(uint32_t k = 0; k < block_count; k++)
	{
		__m128 in = _mm_load_ps(buffer_head);
		__m128 exp = OMLT::_mm_exp_ps(in);
		_mm_store_ps(simd_head, exp);
		buffer_head += 4;
		simd_head += 4;
	}

	// calculate mean absolute error
	simd_head = simd_result;
	float* slow_head = slow_result;
	float re;
	{
		__m128 err = _mm_setzero_ps();
		for(uint32_t k = 0; k < block_count; k++)
		{
			__m128 simd = _mm_load_ps(simd_head);
			__m128 slow = _mm_load_ps(slow_head);

			__m128 relative_err = _mm_sub_ps(_mm_div_ps(slow, simd), _mm_set1_ps(1.0f));
			err = _mm_add_ps(err, relative_err);

			simd_head += 4;
			slow_head += 4;
		}
		err = _mm_hadd_ps(err, err);
		err = _mm_hadd_ps(err, err);

		err = _mm_div_ps(err, _mm_set1_ps(block_count * 4.0f));
		
		_mm_store_ss(&re, err);

		re *= 100;
	}

	_aligned_free(buffer);
	_aligned_free(simd_result);
	_aligned_free(slow_result);

	printf("Average Percent Error: %.9f\n", re);
	if(re > -3.878319636)
	{
		printf("Too Large\n");
		return false;
	}
	return true;
}