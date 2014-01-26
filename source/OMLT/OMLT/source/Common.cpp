// stdlib
#include <cstring>
#include <sstream>

// windows
#include <intrin.h>

// extern
#include <SiCKL.h>
using namespace SiCKL;
#include <cJSON.h>

// OMLT
#include "Common.h"
#include "RestrictedBoltzmannMachine.h"
#include "MultilayerPerceptron.h"

namespace OMLT
{
	/// Parse Methods

	bool ReadTextFile(const std::string& in_filename, std::string& out_text)
	{
		FILE* file = fopen(in_filename.c_str(), "rb");
		if(file == nullptr)
		{
			return false;
		}

		// 1K of space to read to
		char buffer[1024];
		auto readbytes = [&] () -> size_t
		{
			return fread(buffer, sizeof(char), ArraySize(buffer), file);
		};

		std::stringstream ss;
		for(size_t bytes_read = readbytes(); bytes_read > 0; bytes_read = readbytes())
		{
			ss.write(buffer, bytes_read);
		}
		fclose(file);

		out_text = ss.str();
		return true;
	}

	bool FromJSON(const std::string& in_json, Model& out_model)
	{
		bool result = false;
		out_model.type = ModelType::Invalid;
		out_model.ptr = nullptr;

		cJSON* cj_root = cJSON_Parse(in_json.c_str());
		if(cj_root)
		{
			cJSON* cj_type = cJSON_GetObjectItem(cj_root, "Type");
			if(cj_type && strcmp(cj_type->valuestring, "RestrictedBoltzmannMachine") == 0)
			{
				RBM* rbm = RBM::FromJSON(cj_root);
				if(rbm)
				{
					out_model.type = ModelType::RBM;
					out_model.rbm = rbm;
					result = true;
				}
			}
			else if(cj_type && strcmp(cj_type->valuestring, "MultilayerPerceptron") == 0)
			{
				MLP* mlp = MLP::FromJSON(cj_root);
				if(mlp)
				{
					out_model.type = ModelType::MLP;
					out_model.mlp = mlp;
					result = true;
				}
			}
		}

		cJSON_Delete(cj_root);
		return result;
	}

	/// SiCKL Kernel Methods

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

	/*
	 * ln(1 + e^x) ~ (64 + x * (12 * (4+x)-x^2 * sign(x))) / 96 on -4, 4; 0 when x < -4, x when x > 4
	 */
	__m128 _mm_ln_1_plus_e_x_ps(__m128 x)
	{
		// union for setting bitwise constants
		union
		{
			float f;
			uint32_t u;
		};

		// first, set the min of x to be -4, everything to the left of that is 0 which is f(-4)
		x = _mm_max_ps(_mm_set_ps1(-4.0f), x);
		
		// need this constant for _mm_sign_ps
		u = 0x80000000;
		__m128 x80000000 = _mm_set_ps1(f);

		//y0 = (64 + x * (12 * (4+x)-x^2 * sign(x))) / 96

		__m128 y0 = _mm_add_ps(_mm_set_ps1(4.0f), x);
		y0 = _mm_mul_ps(_mm_set_ps1(12.0f), y0);
		y0 = _mm_sub_ps(y0, _mm_mul_ps(_mm_mul_ps(x, x), _mm_sign_ps(x)));
		y0 = _mm_mul_ps(x, y0);
		y0 = _mm_add_ps(_mm_set_ps1(64.0f), y0);
		y0 = _mm_mul_ps(y0, _mm_set_ps1( 1.0f / 96.0f));

		// y1 = [y0 + y0 * sign(4.0 - x)] / 2.0f + [x - x * sign(4.0 - x)] / 2.0f
		__m128 sign_4_minus_x = _mm_sign_ps(_mm_sub_ps(_mm_set_ps1(4.0f), x));

		// either y0 or 0.0f based on sign 4 minux x (ie, x < 4)
		__m128 left = _mm_mul_ps(_mm_add_ps(y0, _mm_mul_ps(y0, sign_4_minus_x)), _mm_set_ps1(0.5f));

		// either x or 0.0f based 
		__m128 right = _mm_mul_ps(_mm_sub_ps(x, _mm_mul_ps(x, sign_4_minus_x)), _mm_set_ps1(0.5f));

		// either left will be 0, or right will be 0, so this comes out to same as adding them
		__m128 y1 = _mm_max_ps(left, right);
		return y1;
	}
}