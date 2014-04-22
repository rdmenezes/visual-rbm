#pragma once

// extern
#include <SiCKL.h>

namespace OMLT
{
	extern void NextSeed(const SiCKL::UInt4& in_seed, SiCKL::UInt4& out_seed);
	extern void NextFloat(const SiCKL::UInt4& in_seed, SiCKL::UInt4& out_seed, SiCKL::Float& out_float);
	extern void NextGaussian(const SiCKL::UInt4& in_seed, SiCKL::UInt4& out_seed, SiCKL::Float& out_gaussian);

	extern SiCKL::Float Linear(const SiCKL::Float& in_x);
	extern SiCKL::Float Sigmoid(const SiCKL::Float& in_x); 
	extern SiCKL::Float RectifiedLinear(const SiCKL::Float& in_x);

	extern SiCKL::Float CalcActivation(ActivationFunction_t in_func, const SiCKL::Float& in_accumulation);
	// this functiont takes in the result of CalcActivation, not the accumulation
	extern SiCKL::Float CalcActivationPrime(ActivationFunction_t in_func, const SiCKL::Float& in_activation);
}