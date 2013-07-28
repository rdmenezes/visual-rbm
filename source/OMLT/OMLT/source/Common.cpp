#include <SiCKL.h>
using namespace SiCKL;

namespace OMLT
{
	void NextSeed(const SiCKL::UInt& in_seed, SiCKL::UInt& out_seed)
	{
		// calculate next values using Numerical recipes LCRG:  
		// http://en.wikipedia.org/wiki/Linear_congruential_generator  
		const uint32_t A = 1664525;
		const uint32_t C = 1013904223;
		// automatically MOD 2^32
		out_seed = in_seed * A + C;
	}

	void NextFloat(const SiCKL::UInt& in_seed, SiCKL::UInt& out_seed, SiCKL::Float& out_float)
	{
		NextSeed(in_seed, out_seed);
		/// see http://docs.oracle.com/javase/6/docs/api/java/util/Random.html#nextFloat()

		// same as out_seed >> 8 (ie 32 bit to 24 bit int)
		SiCKL::UInt next24 = out_seed / 256u;
		out_float = (SiCKL::Float)next24 / (float)(1 << 24);
	}

	void NextGaussian(const SiCKL::UInt& in_seed, SiCKL::UInt& out_seed, SiCKL::Float& out_gaussian)
	{
		SiCKL::Float u1 = 0.0f;
		SiCKL::Float u2 = 0.0f;

		SiCKL::UInt seed0 = in_seed;
		SiCKL::UInt seed1 = 0;

		// get our random values
		While(u1 == 0.0f)
			NextFloat(seed0, seed1, u1);
			seed0 = seed1;		
		EndWhile
		NextFloat(seed0, seed1, u1);
		out_seed = seed1;

		// calculate a normally distributed variable
		const float PI = 3.14159265359f;
		out_gaussian = Sqrt(-2.0f * Log(u1)) * Sin(2.0f * PI * u2);
	}

	extern SiCKL::Float Sigmoid( const SiCKL::Float& in_x )
	{
		return 1.0f / (1.0f + Exp(-in_x));
	}

	/// SIMD shennanigans

#	define _mm_negabs_ps(x) _mm_or_ps(x, x80000000)
#	define _mm_abs_ps(x) _mm_and_ps(x, x7FFFFFFF)
#	define _mm_sign_ps(x) _mm_or_ps(_mm_set_ps1(1.0f), _mm_and_ps(x, x80000000))


	__m128 _mm_rectifiedlinear_ps(__m128 x0)
	{
		return _mm_max_ps(_mm_setzero_ps(), x0);
	}

	/*
	 * sigmoid(x) = 1/(1 + exp(-x))
	 * sigmoid(x) ~ 1/32 * (abs(x) - 4)^2 * -sign(x) + (sign(x) + 1) / 2
	 */
	__m128 _mm_sigmoid_ps(__m128 x0)
	{
		union
		{
			float f;
			uint32_t u;
		};

		 // first calculate abs
		__m128 x;
		__m128 sign;
		{
			u = 0x80000000;
			__m128 x80000000 = _mm_set_ps1(f);

			x = _mm_negabs_ps(x0);
			{
				// now clamp it to -4, 0
				__m128 neg_4 = _mm_set_ps1(-4.0f);
				x = _mm_max_ps(neg_4, x);
				{
					u = 0x7FFFFFFF;
					__m128 x7FFFFFFF = _mm_set_ps1(f);
					// and now abs it
					x = _mm_abs_ps(x);
				}

				// subtract 4
				x = _mm_add_ps(x, neg_4);
			}

			// square the difference
			x = _mm_mul_ps(x, x);

			// and divide by 32	
			x = _mm_div_ps(x, _mm_set_ps1(-32.0f));

			// now we have to fix it for positive x

			// get the sign
			sign = _mm_sign_ps(x0);

			// multiply the first part by the negative sign
			x = _mm_mul_ps(x, sign);
		}

		// calculate sign + 1
		__m128 shift = _mm_add_ps(sign, _mm_set_ps1(1.0f));
		// (sign + 1) / 2
		shift = _mm_div_ps(shift, _mm_set_ps1(2.0f));

		// now shift x5 and we'll be done
		return _mm_add_ps(shift, x);
	}
}