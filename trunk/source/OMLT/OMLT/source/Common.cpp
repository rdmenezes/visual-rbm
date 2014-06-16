// stdc
#include <string.h>
#include <assert.h>
// stdlib
#include <sstream>

// windows
#include <intrin.h>

// cjson
#include <cJSON.h>

// OMLT
#include "Common.h"
#include "RestrictedBoltzmannMachine.h"
#include "MultilayerPerceptron.h"

namespace OMLT
{
	// reads an entire text file into a std::string

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



	/// SIMD shennanigans

	__m128 _mm_negabs_ps(__m128 x)
	{
		union
		{
			float f;
			uint32_t u;
		};
		// sign bit only
		u = 0x80000000;
		return _mm_or_ps(x, _mm_set_ps1(f));
	}

	__m128 _mm_abs_ps(__m128 x)
	{
		union
		{
			float f;
			uint32_t u;
		};
		// everything but the sign bit
		u = 0x7FFFFFFF;
		return _mm_and_ps(x, _mm_set_ps1(f));
	}

	__m128 _mm_sign_ps(__m128 x)
	{
		union
		{
			float f;
			uint32_t u;
		};
		// sign bit only
		u = 0x80000000;
		return _mm_or_ps(_mm_set_ps1(1.0f), _mm_and_ps(x, _mm_set_ps1(f)));
	}

	
	__m128 _mm_rectifiedlinear_ps(__m128 x0)
	{
		return _mm_max_ps(_mm_setzero_ps(), x0);
	}
	decltype(&_mm_rectifiedlinear_ps) _mm_relu_ps = _mm_rectifiedlinear_ps;

	/*
	 * sigmoid(x) = 1/(1 + exp(-x))
	 * sigmoid(x) ~ 1/32 * (abs(x) - 4)^2 * -sign(x) + (sign(x) + 1) / 2
	 */
	__m128 _mm_sigmoid_ps(__m128 x0)
	{
		 // first calculate abs
		__m128 x;
		__m128 sign;
		{
			x = _mm_negabs_ps(x0);
			{
				// now clamp it to -4, 0
				__m128 neg_4 = _mm_set_ps1(-4.0f);
				x = _mm_max_ps(neg_4, x);
				{
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

	__m128 _mm_noop_ps(__m128 x)
	{
		return x;
	}

	// returns a __m128 filled with the max value of x:
	__m128 _mm_hmax_ps(__m128 x)
	{
		auto& max = x;
		max = _mm_max_ps(max, _mm_shuffle_ps(max, max, _MM_SHUFFLE(0, 3, 2, 1)));
		max = _mm_max_ps(max, _mm_shuffle_ps(max, max, _MM_SHUFFLE(1, 0, 3, 2)));
		return max;
	}

	// a fast approximation of exp adopted for single precision floats
	// A Fast, Compact Approximation of the Exponential Function
	// http://nic.schraudolph.org/pubs/Schraudolph99.pdf
	// absolute error increases as x increases, should only use for negative values
	// of x
	//
	// exp(x) ~ reinterpret_cast<float>((int)(2^23 / ln(2) * x + 127 * 2^23))
	// only valid on the range: (-87.336540, 88.722816)
	__m128 _mm_exp_ps(__m128 x)
	{
		// clamp x within valid range
		x = _mm_max_ps(x, _mm_set_ps1(-87.336540));
		x = _mm_min_ps(x, _mm_set_ps1(88.722816));

		// 127 * 2^23
		__m128 b = _mm_set_ps1(1065353216);
		// ln(2)
		__m128 ln_2 = _mm_set_ps1(0.693147180559945309f);

		auto a_times_x = _mm_mul_ps(_mm_set_ps1(1 << 23), x);
		a_times_x = _mm_div_ps(a_times_x, ln_2);

		auto sum = _mm_add_ps(a_times_x, b);

		// cast to ints
		__m128i i= _mm_cvtps_epi32(sum);
		// reinterpret as floats
		__m128 f = _mm_castsi128_ps(i);

		return f;
	}

	// feature map class
	FeatureMap::FeatureMap(uint32_t in_input_length, uint32_t in_feature_count)
		: input_length(in_input_length),
		feature_count(in_feature_count),
		_input_blocks(BlockCount(input_length)),
		_feature_blocks(BlockCount(feature_count))
	{
		assert(input_length > 0);
		assert(feature_count > 0);
		// allocate space for bias vector
		const uint32_t feature_bytes = _feature_blocks * 4 * sizeof(float);
		_biases = (float*)AlignedMalloc(feature_bytes, 16);
		memset(_biases, 0x00, feature_bytes);

		_accumulations = (float*)AlignedMalloc(feature_bytes, 16);
		memset(_accumulations, 0x00, feature_bytes);

		// allocate space for feature vectors
		const uint32_t input_bytes = _input_blocks * 4 * sizeof(float);
		_features = new float*[feature_count];
		for(uint32_t k = 0; k < feature_count; k++)
		{
			_features[k] = (float*)AlignedMalloc(input_bytes, 16);
			memset(_features[k], 0x00, input_bytes);
		}
	}

	FeatureMap::~FeatureMap()
	{
		AlignedFree(_biases);
		_biases = nullptr;
		AlignedFree(_accumulations);
		_accumulations = nullptr;
		for(uint32_t k = 0; k < feature_count; k++)
		{
			AlignedFree(_features[k]);
			_features[k] = nullptr;
		}
		delete[] _features;
		_features = nullptr;
	}

	template<typename FUNC>
	static void CalcActivation(float* accumulations, float* biases, uint32_t blocks, float* activations, FUNC _mm_activation)
	{
		for(uint32_t k = 0; k < blocks; k++)
		{
			__m128 acc = _mm_add_ps(_mm_load_ps(biases), _mm_load_ps(accumulations));
			__m128 act = _mm_activation(acc);
			_mm_store_ps(activations, act);

			biases += 4;
			accumulations += 4;
			activations += 4;
		}
	}

	void FeatureMap::CalcFeatureVector(const float* input_vector, float* output_vector, ActivationFunction_t function) const
	{
		// verify alignment
		assert((intptr_t(input_vector) % 16) == 0);
		assert((intptr_t(output_vector) % 16) == 0);

		for(uint32_t k = 0; k < feature_count; k++)
		{
			// copy of our vectors so we can increment ptr
			const float* input = input_vector;
			float* feature = _features[k];
			// initialize dot product to zero
			__m128 dp = _mm_setzero_ps();
			for(uint32_t j = 0; j < _input_blocks; j++)
			{
				__m128 val = _mm_load_ps(input);
				__m128 feat = _mm_load_ps(feature);

				dp = _mm_add_ps(dp, _mm_mul_ps(val, feat));

				input += 4;
				feature += 4;
			}

			dp = _mm_hadd_ps(dp , dp);
			dp = _mm_hadd_ps(dp , dp);

			// store back to intermediate buffer
			_mm_store_ss(_accumulations + k, dp);
		}

		switch(function)
		{
		case ActivationFunction::Linear:
			CalcActivation(_accumulations, _biases, _feature_blocks, output_vector, _mm_noop_ps);
			break;
		case ActivationFunction::RectifiedLinear:
			CalcActivation(_accumulations, _biases, _feature_blocks, output_vector, _mm_relu_ps);
			break;
		case ActivationFunction::Sigmoid:
			CalcActivation(_accumulations, _biases, _feature_blocks, output_vector, _mm_sigmoid_ps);
			break;
		case ActivationFunction::Softmax:
			CalcActivation(_accumulations, _biases, _feature_blocks, output_vector, _mm_noop_ps);
			{
				// to avoid overflow, we can subtract the max accumulation from all the exp(x) calls
				// http://deeplearning.stanford.edu/wiki/index.php/Exercise:Softmax_Regression

				// set dangling values to -FLT_MAX
				for(uint32_t k = feature_count; k < _feature_blocks * 4; k++)
				{
					output_vector[k] = -FLT_MAX;
				}

				// calculate the max value
				__m128 max = _mm_set1_ps(-FLT_MAX);
				float* accumulation_head = output_vector;
				for(uint32_t k = 0; k < _feature_blocks; k++)
				{
					__m128 acc = _mm_load_ps(accumulation_head);
					max = _mm_max_ps(max, acc);

					accumulation_head += 4;
				}
				max = _mm_hmax_ps(max);

				// calculate the divisor
				__m128 divisor = _mm_setzero_ps();
				accumulation_head = output_vector;
				for(uint32_t k = 0; k < _feature_blocks; k++)
				{
					__m128 acc = _mm_load_ps(accumulation_head);
					__m128 val = _mm_exp_ps(_mm_sub_ps(acc, max));
					divisor = _mm_add_ps(divisor, val);

					accumulation_head += 4;
				}
				divisor = _mm_hadd_ps(divisor, divisor);
				divisor = _mm_hadd_ps(divisor, divisor);

				// now calculate softmax
				accumulation_head = output_vector;
				float* output_head = output_vector;
				for(uint32_t k = 0; k < _feature_blocks; k++)
				{
					__m128 acc = _mm_load_ps(accumulation_head);
					__m128 val = _mm_exp_ps(_mm_sub_ps(acc, max));
					val = _mm_div_ps(val, divisor);

					_mm_store_ps(output_head, val);

					accumulation_head += 4;
					output_head += 4;
				}
			}
			break;
		}

		// finally, set dangling values to 0
		for(uint32_t k = feature_count; k < _feature_blocks * 4; k++)
		{
			output_vector[k] = 0.0f;
		}
	}
}