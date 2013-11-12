// c
#include <assert.h>

// windows
#include <malloc.h>
#include <intrin.h>

// extern
#include <cJSON.h>

// OMLT
#include "Common.h"
#include "RestrictedBoltzmannMachine.h"

namespace OMLT
{
	inline static void allocate_features(float**& ptr, const uint32_t width, const size_t size)
	{
		ptr = new float*[width];
		for(uint32_t i = 0; i < width; i++)
		{
			ptr[i] = (float*)_aligned_malloc(size, 16);
			memset(ptr[i], 0x00, size);
		}
	}

	inline static void free_features(float** const buff, uint32_t width)
	{
		for(uint32_t i = 0; i < width; i++)
		{
			_aligned_free(buff[i]);
		}

		delete[] buff;
	}

	RestrictedBoltzmannMachine::RestrictedBoltzmannMachine( uint32_t in_visible_count, uint32_t in_hidden_count, ActivationFunction_t in_visible_type, ActivationFunction_t in_hidden_type )
		: visible_count(in_visible_count)
		, hidden_count(in_hidden_count)
		, visible_type(in_visible_type)
		, hidden_type(in_hidden_type)
	{
		// precompute allocation sizes
		const size_t visible_allocsize = sizeof(float) * 4 * BlockCount(visible_count);
		const size_t hidden_allocsize = sizeof(float) * 4 * BlockCount(hidden_count);

		// biases
		visible_biases = (float*)_aligned_malloc(visible_allocsize, 16);
		hidden_biases = (float*)_aligned_malloc(hidden_allocsize, 16);

		memset(visible_biases, 0x00, visible_allocsize);
		memset(hidden_biases, 0x00, hidden_allocsize);

		// weights
		allocate_features(hidden_features, hidden_count, visible_allocsize);
		allocate_features(visible_features, visible_count, hidden_allocsize);

		// aligned scratch space
		_visible_buffer = (float*)_aligned_malloc(visible_allocsize, 16);
		_hidden_buffer = (float*)_aligned_malloc(hidden_allocsize, 16);

		memset(_visible_buffer, 0x00, visible_allocsize);
		memset(_hidden_buffer, 0x00, hidden_allocsize);
	}

	RestrictedBoltzmannMachine::~RestrictedBoltzmannMachine()
	{
		_aligned_free(visible_biases);
		_aligned_free(hidden_biases);

		free_features(hidden_features, hidden_count);
		free_features(visible_features, visible_count);

		_aligned_free(_visible_buffer);
		_aligned_free(_hidden_buffer);
	}

	static void calc_activation(const float* biases, float* inout_buffer, const uint32_t blocks, ActivationFunction_t func)
	{
		switch(func)
		{
		case ActivationFunction::Linear:
			break;
		case ActivationFunction::RectifiedLinear:
			extern __m128 _mm_rectifiedlinear_ps(__m128 x0);
			for(uint32_t i = 0; i < blocks; i++)
			{
				// load bias and accumulation
				__m128 bias = _mm_load_ps(biases);
				__m128 accum = _mm_load_ps(inout_buffer);
				
				// add bias, calculate rectified linear
				accum = _mm_add_ps(accum, bias);
				__m128 activ = _mm_rectifiedlinear_ps(accum);

				_mm_store_ps(inout_buffer, activ);

				biases += 4;
				inout_buffer += 4;
			}
			break;
		case ActivationFunction::Sigmoid:
			extern __m128 _mm_sigmoid_ps(__m128 x0);
			for(uint32_t i = 0; i < blocks; i++)
			{
				// load bias and accumulation
				__m128 bias = _mm_load_ps(biases);
				__m128 accum = _mm_load_ps(inout_buffer);

				// add bias, calculate sigmoid
				accum = _mm_add_ps(accum, bias);
				__m128 activ = _mm_sigmoid_ps(accum);

				_mm_store_ps(inout_buffer, activ);

				biases += 4;
				inout_buffer += 4;
			}
			break;
		}
	}

	static float calc_accumulation(const float* input_vector, const float* feature_vector, const uint32_t blockcount)
	{
		// calculate dot product between input ad feature vector
		__m128 dp = _mm_setzero_ps();
		for(uint32_t i = 0; i < blockcount; i++)
		{
			__m128 v = _mm_load_ps(input_vector);
			__m128 w = _mm_load_ps(feature_vector);

			dp = _mm_add_ps(dp, _mm_mul_ps(v, w));

			input_vector += 4;
			feature_vector += 4;
		}

		dp = _mm_hadd_ps(dp , dp);
		dp = _mm_hadd_ps(dp , dp);

		// store it back aligned output buffer
		float result;
		_mm_store_ss(&result, dp);

		return result;
	}

	inline size_t DanglingBytes(const uint32_t block_count, uint32_t input_count)
	{
		return (block_count * 4 * sizeof(float)) - input_count * sizeof(float);
	}

	static void calc_feature_map(
		const float* input_buffer, float* output_buffer, 
		float* input_scratch, float* output_scratch, 
		const uint32_t input_count, const uint32_t output_count,
		const float* biases, float** const features, const ActivationFunction_t activation)
	{
		assert(((intptr_t)input_scratch % 16) == 0);
		assert(((intptr_t)output_scratch % 16) == 0);

		memcpy(input_scratch, input_buffer, sizeof(float) * input_count);
		const uint32_t input_blockcount = BlockCount(input_count);
		
		// set dangling floats to 0 so we don't screw up any calculations
		const size_t dangling_bytes = DanglingBytes(input_blockcount, input_count);
		if(dangling_bytes > 0)
		{
			memset(input_scratch + input_count, 0x00, dangling_bytes);
		}

		for(uint32_t j = 0; j < output_count; j++)
		{
			output_scratch[j] = calc_accumulation(input_scratch, features[j], input_blockcount);
		}

		calc_activation(biases, output_scratch, BlockCount(output_count), activation);

		// copy from aligned scratch space to output buffer
		if(output_buffer != output_scratch)
		{
			memcpy(output_buffer, output_scratch, output_count * sizeof(float));
		}
	}

	void RestrictedBoltzmannMachine::CalcHidden( const float* in_visible, float* out_hidden ) const
	{
		calc_feature_map(
			in_visible, out_hidden, 
			_visible_buffer, _hidden_buffer, 
			visible_count, hidden_count, 
			hidden_biases, hidden_features, hidden_type);
	}

	void RestrictedBoltzmannMachine::CalcVisible( const float* in_hidden, float* out_visible ) const
	{
		calc_feature_map(
			in_hidden, out_visible,		
			_hidden_buffer, _visible_buffer, 
			hidden_count, visible_count,
			visible_biases, visible_features, visible_type);
	}

	extern __m128 _mm_ln_1_plus_e_x_ps(__m128 x);
	float RestrictedBoltzmannMachine::CalcFreeEnergy( const float* in_visible ) const
	{
		assert(visible_type == ActivationFunction::Sigmoid && hidden_type == ActivationFunction::Sigmoid);

		// log sum calculation
		calc_feature_map(
			in_visible, _hidden_buffer,
			_visible_buffer, _hidden_buffer,
			visible_count, hidden_count,
			hidden_biases, hidden_features, ActivationFunction::Linear);

		// set the last dangling floats to -4 so that ln(1 + e^x) maps them to 0 (see _mm_ln_1_plus_e_x_ps for details)
		// otherwise, we'll have random garbage being appended
		const size_t hidden_blocks = BlockCount(hidden_count);
		for(uint32_t k = hidden_count; k < 4 * hidden_blocks; k++)
		{
			_hidden_buffer[k] = -4.0f;
		}

		// calc log(1 + e^x) and add to sum
		__m128 dp = _mm_setzero_ps();
		for(uint32_t k = 0; k < hidden_blocks; k++)
		{
			__m128 x = _mm_load_ps(_hidden_buffer + 4 * k);
			dp = _mm_add_ps(dp, _mm_ln_1_plus_e_x_ps(x));	
		}
		
		dp = _mm_hadd_ps(dp, dp);
		dp = _mm_hadd_ps(dp, dp);

		float log_dp;
		_mm_store_ss(&log_dp, dp);

		// bias sum calculation
		const uint32_t visible_blocks = BlockCount(visible_count);

		dp = _mm_setzero_ps();
		// calc_feature_map copies in visible_biases 
		for(uint32_t k = 0; k < visible_blocks; k++)
		{
			__m128 b = _mm_load_ps(visible_biases + 4 * k);
			__m128 vec = _mm_load_ps(_visible_buffer + 4 * k);

			dp = _mm_add_ps(dp, _mm_mul_ps(b, vec));
		}

		dp = _mm_hadd_ps(dp, dp);
		dp = _mm_hadd_ps(dp, dp);

		float bias_dp;
		_mm_store_ss(&bias_dp, dp);

		return -(bias_dp + log_dp);
	}

	std::string RestrictedBoltzmannMachine::ToJSON() const
	{
		cJSON* root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "Type", "RestrictedBoltzmannMachine");
		cJSON_AddNumberToObject(root, "VisibleCount", visible_count);
		cJSON_AddNumberToObject(root, "HiddenCount", hidden_count);
		cJSON_AddStringToObject(root, "VisibleType", ActivationFunctionNames[visible_type]);
		cJSON_AddStringToObject(root, "HiddenType", ActivationFunctionNames[hidden_type]);
		cJSON_AddItemToObject(root, "VisibleBiases", cJSON_CreateFloatArray(visible_biases, visible_count));
		cJSON_AddItemToObject(root, "HiddenBiases", cJSON_CreateFloatArray(hidden_biases, hidden_count));

		cJSON* root_weights = cJSON_CreateArray();
		cJSON_AddItemToObject(root, "Weights", root_weights);

		for(uint32_t j = 0; j < hidden_count; j++)
		{
			cJSON_AddItemToArray(root_weights, cJSON_CreateFloatArray(hidden_features[j], visible_count));
		}

		char* json_buffer = cJSON_Print(root);
		std::string result(json_buffer);

		//cleanup
		free(json_buffer);
		cJSON_Delete(root);

		return result;
	}

	RestrictedBoltzmannMachine* RestrictedBoltzmannMachine::FromJSON( const std::string& in_JSON )
	{
		cJSON* root = cJSON_Parse(in_JSON.c_str());
		if(root)
		{
			RestrictedBoltzmannMachine* rbm = nullptr;

			cJSON* cj_type = cJSON_GetObjectItem(root, "Type");
			cJSON* cj_visible_count = cJSON_GetObjectItem(root, "VisibleCount");
			cJSON* cj_hidden_count = cJSON_GetObjectItem(root, "HiddenCount");
			cJSON* cj_visible_type = cJSON_GetObjectItem(root, "VisibleType");
			cJSON* cj_hidden_type = cJSON_GetObjectItem(root, "HiddenType");
			cJSON* cj_visible_biases = cJSON_GetObjectItem(root, "VisibleBiases");
			cJSON* cj_hidden_biases = cJSON_GetObjectItem(root, "HiddenBiases");
			cJSON* cj_weights = cJSON_GetObjectItem(root, "Weights");

			if(cj_visible_count && cj_hidden_count &&
				cj_visible_type && cj_hidden_type &&
				cj_visible_biases && cj_hidden_biases &&
				cj_weights && cj_type)
			{
				if(strcmp(cj_type->valuestring, "RestrictedBoltzmannMachine") != 0)
				{
					goto Malformed;
				}

				uint32_t visible_count = cj_visible_count->valueint;
				uint32_t hidden_count = cj_hidden_count->valueint;
				ActivationFunction_t visible_type = (ActivationFunction_t)-1;
				ActivationFunction_t hidden_type = (ActivationFunction_t)-1;

				for(int func = 0; func < ActivationFunction::Count; func++)
				{
					if(strcmp(cj_visible_type->valuestring, ActivationFunctionNames[func]) == 0)
					{
						visible_type = (ActivationFunction_t)func;
					}

					if(strcmp(cj_hidden_type->valuestring, ActivationFunctionNames[func]) == 0)
					{
						hidden_type = (ActivationFunction_t)func;
					}
				}

				// make sure we found an activation function
				if(visible_type == -1 || hidden_type == -1)
				{
					goto Malformed;
				}

				// verify these arrays are the right size
				if( cJSON_GetArraySize(cj_visible_biases) == visible_count &&
					cJSON_GetArraySize(cj_hidden_biases) == hidden_count &&
					cJSON_GetArraySize(cj_weights) == hidden_count)
				{
					rbm = new RestrictedBoltzmannMachine(visible_count, hidden_count, visible_type, hidden_type);
					
					// copy in visible biases
					for(uint32_t i = 0; i < visible_count; i++)
					{
						rbm->visible_biases[i] = (float)cJSON_GetArrayItem(cj_visible_biases, i)->valuedouble;
					}

					// copy in hidden biases
					for(uint32_t j = 0; j < hidden_count; j++)
					{
						rbm->hidden_biases[j] = (float)cJSON_GetArrayItem(cj_hidden_biases, j)->valuedouble;
					}

					// copy in weights
					for(uint32_t j = 0; j < hidden_count; j++)
					{
						cJSON* cj_feature_vector = cJSON_GetArrayItem(cj_weights, j);
						if(cJSON_GetArraySize(cj_feature_vector) != visible_count)
						{
							delete rbm;
							goto Malformed;
						}
						else
						{
							for(uint32_t i = 0; i < visible_count; i++)
							{
								float w_ij = (float)cJSON_GetArrayItem(cj_feature_vector, i)->valuedouble;
								rbm->hidden_features[j][i] = w_ij;
								rbm->visible_features[i][j] = w_ij;
							}
						}
					}
				}
				else
				{
					goto Malformed;
				}
			}
			else 
			{
				goto Malformed;
			}

			cJSON_Delete(root);
			return rbm;
Malformed:
			cJSON_Delete(root);
			return nullptr;
		}
		return nullptr;
	}


}