// stdc
#include <string.h>

// cJSON
#include <cJSON.h>

// OMLT
#include "AutoEncoder.h"
#include "Common.h"

namespace OMLT
{
	extern void allocate_features(float**& ptr, const uint32_t width, const size_t size);
	extern void free_features(float** const buff, uint32_t width);

	AutoEncoder::AutoEncoder( uint32_t in_visible_count, uint32_t in_hidden_count, ActivationFunction_t in_hidden_type, ActivationFunction_t in_output_type )
		: visible_count(in_visible_count),
		  hidden_count(in_hidden_count),
		  hidden_type(in_hidden_type),
		  output_type(in_output_type),
		  encoder(visible_count, hidden_count, hidden_type),
		  decoder(hidden_count, visible_count, output_type)
	{

	}

	void AutoEncoder::Encode( const float* in_raw, float* out_encoded ) const
	{
		encoder.CalcFeatureVector(in_raw, out_encoded);
	}

	void AutoEncoder::Decode( const float* in_decoded, float* out_raw ) const
	{
		decoder.CalcFeatureVector(in_decoded, out_raw);
	}

	std::string AutoEncoder::ToJSON() const
	{
		cJSON* root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "Type", "AutoEncoder");
		cJSON_AddNumberToObject(root, "VisibleCount", visible_count);
		cJSON_AddNumberToObject(root, "HiddenCount", hidden_count);
		cJSON_AddStringToObject(root, "OutputType", ActivationFunctionNames[output_type]);
		cJSON_AddStringToObject(root, "HiddenType", ActivationFunctionNames[hidden_type]);
		cJSON_AddItemToObject(root, "HiddenBiases", cJSON_CreateFloatArray((float*)encoder.biases(), hidden_count));
		cJSON_AddItemToObject(root, "OutputBiases", cJSON_CreateFloatArray((float*)decoder.biases(), visible_count));

		cJSON* root_weights = cJSON_CreateArray();
		cJSON_AddItemToObject(root, "Weights", root_weights);

		for(uint32_t j = 0; j < hidden_count; j++)
		{
			cJSON_AddItemToArray(root_weights, cJSON_CreateFloatArray((float*)encoder.feature(j), visible_count));
		}

		char* json_buffer = cJSON_Print(root);
		std::string result(json_buffer);

		//cleanup
		free(json_buffer);
		cJSON_Delete(root);

		return result;
	}

	AutoEncoder* AutoEncoder::FromJSON( struct cJSON* in_root )
	{
		if(in_root)
		{
			AutoEncoder* ae = nullptr;

			cJSON* cj_type = cJSON_GetObjectItem(in_root, "Type");
			cJSON* cj_visible_count = cJSON_GetObjectItem(in_root, "VisibleCount");
			cJSON* cj_hidden_count = cJSON_GetObjectItem(in_root, "HiddenCount");
			cJSON* cj_hidden_type = cJSON_GetObjectItem(in_root, "HiddenType");
			cJSON* cj_output_type = cJSON_GetObjectItem(in_root, "OutputType");
			cJSON* cj_hidden_biases = cJSON_GetObjectItem(in_root, "HiddenBiases");
			cJSON* cj_output_biases = cJSON_GetObjectItem(in_root, "OutputBiases");
			cJSON* cj_weights = cJSON_GetObjectItem(in_root, "Weights");

			if(cj_visible_count && cj_hidden_count &&
				cj_output_type && cj_hidden_type &&
				cj_output_biases && cj_hidden_biases &&
				cj_weights && cj_type)
			{
				if(strcmp(cj_type->valuestring, "AutoEncoder") != 0)
				{
					goto Malformed;
				}

				uint32_t visible_count = cj_visible_count->valueint;
				uint32_t hidden_count = cj_hidden_count->valueint;
				ActivationFunction_t output_type = (ActivationFunction_t)-1;
				ActivationFunction_t hidden_type = (ActivationFunction_t)-1;

				for(int func = 0; func < ActivationFunction::Count; func++)
				{
					if(strcmp(cj_hidden_type->valuestring, ActivationFunctionNames[func]) == 0)
					{
						hidden_type = (ActivationFunction_t)func;
					}

					if(strcmp(cj_output_type->valuestring, ActivationFunctionNames[func]) == 0)
					{
						output_type = (ActivationFunction_t)func;
					}
				}

				// make sure we found an activation function
				if(hidden_type == -1 || output_type  == -1)
				{
					goto Malformed;
				}

				// verify these arrays are the right size
				if( cJSON_GetArraySize(cj_output_biases) == visible_count &&
					cJSON_GetArraySize(cj_hidden_biases) == hidden_count &&
					cJSON_GetArraySize(cj_weights) == hidden_count)
				{
					ae = new AutoEncoder(visible_count, hidden_count, output_type, hidden_type);

					cJSON* vb_it = cJSON_CreateArrayIterator(cj_output_biases);

					// copy in visible biases
					for(uint32_t i = 0; i < visible_count; i++)
					{
						cJSON_ArrayIteratorMoveNext(vb_it);
						ae->decoder.biases()[i] = (float)cJSON_ArrayIteratorCurrent(vb_it)->valuedouble;
					}
					cJSON_Delete(vb_it);

					cJSON* hb_it = cJSON_CreateArrayIterator(cj_hidden_biases);

					// copy in hidden biases
					for(uint32_t j = 0; j < hidden_count; j++)
					{
						cJSON_ArrayIteratorMoveNext(hb_it);
						ae->encoder.biases()[j] = (float)cJSON_ArrayIteratorCurrent(hb_it)->valuedouble;
					}
					cJSON_Delete(hb_it);

					cJSON* w_it = cJSON_CreateArrayIterator(cj_weights);

					// copy in weights
					for(uint32_t j = 0; j < hidden_count; j++)
					{
						cJSON_ArrayIteratorMoveNext(w_it);
						cJSON* cj_feature_vector = cJSON_ArrayIteratorCurrent(w_it);
						if(cJSON_GetArraySize(cj_feature_vector) != visible_count)
						{
							delete ae;
							cJSON_Delete(w_it);
							goto Malformed;
						}
						else
						{
							cJSON* wj_it = cJSON_CreateArrayIterator(cj_feature_vector);
							for(uint32_t i = 0; i < visible_count; i++)
							{
								cJSON_ArrayIteratorMoveNext(wj_it);
								float w_ij = (float)cJSON_ArrayIteratorCurrent(wj_it)->valuedouble;
								ae->encoder.feature(j)[i] = w_ij;
								ae->decoder.feature(i)[j] = w_ij;
							}
							cJSON_Delete(wj_it);
						}
					}
					cJSON_Delete(w_it);
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

			return ae;
Malformed:
			return nullptr;
		}
		return nullptr;
	}
}