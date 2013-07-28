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

		// get our random values
		While(u1 == 0.0f)
			NextFloat(in_seed, out_seed, u1);
		EndWhile
		NextFloat(out_seed, out_seed, u2);

		// calculate a normally distributed variable
		const float PI = 3.14159265359f;
		out_gaussian = Sqrt(-2.0f * Log(u1)) * Sin(2.0f * PI * u2);
	}

	extern SiCKL::Float Sigmoid( const SiCKL::Float& in_x )
	{
		return 1.0f / (1.0f + Exp(-in_x));
	}

}