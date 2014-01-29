#pragma once

// extern
#include <SiCKL.h>

namespace OMLT
{
	/// Some Common SiCKL methods
	extern void NextSeed(const SiCKL::UInt& in_seed, SiCKL::UInt& out_seed);
	extern void NextFloat(const SiCKL::UInt& in_seed, SiCKL::UInt& out_seed, SiCKL::Float& out_float);
	extern void NextGaussian(const SiCKL::UInt& in_seed, SiCKL::UInt& out_seed, SiCKL::Float& out_gaussian);
	extern SiCKL::Float Sigmoid(const SiCKL::Float& in_x); 
}