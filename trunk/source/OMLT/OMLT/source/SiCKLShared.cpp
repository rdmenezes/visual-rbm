// extern
#include <SiCKL.h>
using namespace SiCKL;

#include "Enums.h"

namespace OMLT
{

	/// SiCKL Kernel Methods

	extern void NextSeed(const SiCKL::UInt4& in_seed, SiCKL::UInt4& out_seed)
	{
		UInt t = in_seed.X ^ (in_seed.X << 11u);
		out_seed.X = in_seed.Y;
		out_seed.Y = in_seed.Z;
		out_seed.Z = in_seed.W;
		out_seed.W = in_seed.W ^ (in_seed.W >> 19u) ^ t ^ (t >> 8u);
	}
	extern void NextFloat(const SiCKL::UInt4& in_seed, SiCKL::UInt4& out_seed, SiCKL::Float& out_float)
	{
		NextSeed(in_seed, out_seed);

		// top 24 bits 
		UInt next24 = out_seed.X >> 8u;

		out_float = (Float)next24 / (float)(1 << 24);
	}
	extern void NextGaussian(const SiCKL::UInt4& in_seed, SiCKL::UInt4& out_seed, SiCKL::Float& out_gaussian)
	{
		Float u1 = 0.0f;
		Float u2 = 0.0f;

		UInt4 seed0 = in_seed;
		UInt4 seed1(0,0,0,0);

		// get our random values
		NextFloat(seed0, seed1, u1);
		u1 = Max(u1, 0.00000005960464478f);
		seed0 = seed1;		
		NextFloat(seed0, seed1, u2);
		out_seed = seed1;

		// calculate a normally distributed variable
		const float PI = 3.14159265359f;
		out_gaussian = Sqrt(-2.0f * Log(u1)) * Sin(2.0f * PI * u2);
	}

	Float Sigmoid( const Float& in_x )
	{
		return 1.0f / (1.0f + Exp(-in_x));
	}

	Float RectifiedLinear(const Float& in_x)
	{
		return Max(in_x, 0.0f);
	}
	
	Float CalcActivation(ActivationFunction_t in_func, const Float& in_accumulation)
	{
		COMPUTE_ASSERT(in_func == ActivationFunction::Linear ||
		               in_func == ActivationFunction::Sigmoid ||
					   in_func == ActivationFunction::RectifiedLinear ||
					   in_func == ActivationFunction::Softmax);

		switch(in_func)
		{
		case ActivationFunction::Softmax:
		case ActivationFunction::Linear:
			return in_accumulation;
		case ActivationFunction::Sigmoid:
			return Sigmoid(in_accumulation);
		case ActivationFunction::RectifiedLinear:
			return RectifiedLinear(in_accumulation);
		}
		
		return Float(0.0f);
	}

	Float CalcActivationPrime(ActivationFunction_t in_func, const Float& in_activation)
	{
		COMPUTE_ASSERT(in_func == ActivationFunction::Linear ||
			in_func == ActivationFunction::Sigmoid ||
			in_func == ActivationFunction::RectifiedLinear ||
			in_func == ActivationFunction::Softmax);

		switch(in_func)
		{
		case ActivationFunction::Linear:
			return Float(1.0f);
		case ActivationFunction::Sigmoid:
		case ActivationFunction::Softmax:
			return (1.0f - in_activation) * in_activation;
		case ActivationFunction::RectifiedLinear:
			return Max(0.0f, Sign(in_activation));
		}

		return Float(0.0f);
	}
}