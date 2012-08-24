#include "Common.h"
#include "math.h"
#include "stdlib.h"
#include <omp.h>

double NextGaussian()
{
	static bool HaveNextNextGaussian = false;
	static double NextNextGaussian = 0.0;

	static auto next_double = []() -> double { return double(rand()) / double(RAND_MAX); }; 

	if (HaveNextNextGaussian) 
	{
		HaveNextNextGaussian = false;
		return NextNextGaussian;
	}
	else
	{
		double v1, v2, s;
		do 
		{ 
			v1 = 2 * next_double() - 1;   // between -1.0 and 1.0
			v2 = 2 * next_double() - 1;   // between -1.0 and 1.0
			s = v1 * v1 + v2 * v2;
		}
		while (s >= 1 || s == 0);
		double multiplier = sqrt(-2 * log(s)/s);
		NextNextGaussian = v2 * multiplier;
		HaveNextNextGaussian = true;
		return v1 * multiplier;
	}
}

// simd mean absolute value
// assumes buffer is 16 byte ealigned and that trailing floats are 0.0f
float CalcMeanAbsValue(float* buffer, int count)
{
	int m128count = count / 4;
	m128count += count % 4 ? 1 : 0;

	// store mask for abs_value

	// sets sign bit to 0
	unsigned int umask = 0x7FFFFFFF;
	float fmask = *(float*)&umask;
	__m128 MASK = _mm_set_ps1(fmask);
	__m128 M = _mm_setzero_ps();

	for(int k = 0; k < m128count; k++)
	{
		__m128 val = _mm_load_ps(buffer);
		// absolute value
		val = _mm_and_ps(val, MASK);
		// accumulate
		M = _mm_add_ps(M, val);

		buffer += 4;
	}

	M = _mm_hadd_ps(M, M);
	M = _mm_hadd_ps(M, M);

	float result;
	_mm_store_ss(&result, M);

	return std::max(result / float(count), 0.0000001f);
}

float CalcMeanAbsValueParallel(float* buffer, int count)
{
	int m128count = count / 4;
	m128count += count % 4 ? 1 : 0;

	// number of physical processors
	const int CORES = omp_get_num_procs();
	// number of iterations each core should do
	const int BLOCKS = m128count / CORES;
	if(BLOCKS == 0)
	{
		return CalcMeanAbsValue(buffer, count);
	}

	omp_set_num_threads(CORES);

	float result = 0.0f;

#pragma omp parallel for
	for(int b = 0; b < CORES; b++)
	{

		float* my_buffer = buffer + BLOCKS * b * 4;

		// store mask for abs_value
		// sets sign bit to 0
		unsigned int umask = 0x7FFFFFFF;
		float fmask = *(float*)&umask;
		__m128 MASK = _mm_set_ps1(fmask);
		__m128 M = _mm_setzero_ps();

		// get how large our block is
		const int my_blocksize = b == CORES - 1 ? m128count - BLOCKS * (CORES - 1) : BLOCKS;

		for(int k = 0; k < my_blocksize; k++)
		{
			__m128 val = _mm_load_ps(my_buffer);
			// absolute value
			val = _mm_and_ps(val, MASK);
			// accumulate
			M = _mm_add_ps(M, val);

			my_buffer += 4;
		}

		M = _mm_hadd_ps(M, M);
		M = _mm_hadd_ps(M, M);

		float my_result;
		_mm_store_ss(&my_result, M);

#pragma omp atomic
		result += my_result;
	}

	return std::max(result / float(count), 0.0000001f);
}