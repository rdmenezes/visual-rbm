// c
#include <assert.h>

// c++
#include <string>
#include <limits>
using std::string;
#include <memory>
using std::auto_ptr;

// windows
#include <intrin.h>

// extern
#include <cJSON.h>
#include <cppJSONStream.hpp>
using namespace cppJSONStream;

// OMLT
#include "Common.h"
#include "RestrictedBoltzmannMachine.h"

namespace OMLT
{
	RestrictedBoltzmannMachine::RestrictedBoltzmannMachine( uint32_t in_visible_count, uint32_t in_hidden_count, ActivationFunction_t in_visible_type, ActivationFunction_t in_hidden_type )
		: visible_count(in_visible_count)
		, hidden_count(in_hidden_count)
		, visible_type(in_visible_type)
		, hidden_type(in_hidden_type)
		, hidden(visible_count, hidden_count)
		, visible(hidden_count, visible_count)
	{

	}

	void RestrictedBoltzmannMachine::CalcHidden( const float* in_visible, float* out_hidden ) const
	{
		hidden.CalcFeatureVector(in_visible, out_hidden, hidden_type);
	}

	void RestrictedBoltzmannMachine::CalcVisible( const float* in_hidden, float* out_visible ) const
	{
		visible.CalcFeatureVector(in_hidden, out_visible, visible_type);
	}

	extern __m128 _mm_ln_1_plus_e_x_ps(__m128 x);
	float RestrictedBoltzmannMachine::CalcFreeEnergy( const float* in_visible ) const
	{
		assert(visible_type == ActivationFunction::Sigmoid || visible_type == ActivationFunction::RectifiedLinear);
		assert(hidden_type == ActivationFunction::Sigmoid || hidden_type == ActivationFunction::RectifiedLinear);
		
		// free energy
		// F(v) = - sum (v_i * a_i) - sum ln(1 + e^x_j)
		// 
		// v_i = visible i
		// a_i = visible bias a
		// x_j = b_j + sum (v_i * w_ij) ; hidden accumulation
		// b_j = hidden bias b

		float bias_sum = 0.0f;

		// calc dot product between visible and visible biases
		for(uint32_t i = 0; i < visible_count; i++)
		{
			bias_sum += in_visible[i] * visible.biases()[i];
		}

		// now calculate that second part
		static AlignedMemoryBlock<float> hidden_buffer;
		hidden_buffer.Acquire(hidden_count);

		// calc hidden accumulations
		hidden.CalcFeatureVector(in_visible, (float*)hidden_buffer, ActivationFunction::Linear);

		// set the last dangling floats to -4 so that ln(1 + e^x) maps them to 0 (see _mm_ln_1_plus_e_x_ps for details)
		// otherwise, we'll have random garbage being appended
		for(uint32_t k = hidden_count; k < 4 * hidden_buffer.BlockCount(); k++)
		{
			hidden_buffer[k] = -4.0f;
		}

		// calc all the ln(1 + e^x) and add them together
		float* head = hidden_buffer;
		__m128 sum = _mm_setzero_ps();
		for(uint32_t k = 0; k < hidden_buffer.BlockCount(); k++)
		{
			sum = _mm_add_ps(sum, _mm_ln_1_plus_e_x_ps(_mm_load_ps(head)));
		}
		
		sum = _mm_hadd_ps(sum, sum);
		sum = _mm_hadd_ps(sum, sum);

		float log_sum;
		_mm_store_ss(&log_sum, sum);

		return (-bias_sum - log_sum);
	}

	std::string RestrictedBoltzmannMachine::ToJSON() const
	{
		cJSON* root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "Type", "RestrictedBoltzmannMachine");
		cJSON_AddNumberToObject(root, "VisibleCount", visible_count);
		cJSON_AddNumberToObject(root, "HiddenCount", hidden_count);
		cJSON_AddStringToObject(root, "VisibleType", ActivationFunctionNames[visible_type]);
		cJSON_AddStringToObject(root, "HiddenType", ActivationFunctionNames[hidden_type]);
		cJSON_AddItemToObject(root, "VisibleBiases", cJSON_CreateFloatArray((float*)visible.biases(), visible_count));
		cJSON_AddItemToObject(root, "HiddenBiases", cJSON_CreateFloatArray((float*)hidden.biases(), hidden_count));

		cJSON* root_weights = cJSON_CreateArray();
		cJSON_AddItemToObject(root, "Weights", root_weights);

		for(uint32_t j = 0; j < hidden_count; j++)
		{
			cJSON_AddItemToArray(root_weights, cJSON_CreateFloatArray((float*)hidden.feature(j), visible_count));
		}

		char* json_buffer = cJSON_Print(root);
		std::string result(json_buffer);

		//cleanup
		free(json_buffer);
		cJSON_Delete(root);

		return result;
	}

	void RestrictedBoltzmannMachine::ToJSON(std::ostream& stream) const
	{
		cppJSONStream::Writer w(stream, true);

		w.begin_object();
			w.write_namevalue("Type", "RestrictedBoltzmannMachine");
			w.write_namevalue("VisibleCount", (uint64_t)visible_count);
			w.write_namevalue("HiddenCount", (uint64_t)hidden_count);
			w.write_namevalue("VisibleType", ActivationFunctionNames[visible_type]);
			w.write_namevalue("HiddenType", ActivationFunctionNames[hidden_type]);
			w.write_name("VisibleBiases");
				w.write_array(visible.biases(), visible_count);
			w.write_name("HiddenBiases");
				w.write_array(hidden.biases(), hidden_count);
			w.write_name("Weights");
				w.begin_array();
				for(uint32_t j = 0; j < hidden_count; j++)
				{
					w.write_array(hidden.feature(j), visible_count);
				}
				w.end_array();
		w.end_object();
	}

	RestrictedBoltzmannMachine* RestrictedBoltzmannMachine::FromJSON( cJSON* in_root )
	{
		if(in_root)
		{
			RestrictedBoltzmannMachine* rbm = nullptr;

			cJSON* cj_type = cJSON_GetObjectItem(in_root, "Type");
			cJSON* cj_visible_count = cJSON_GetObjectItem(in_root, "VisibleCount");
			cJSON* cj_hidden_count = cJSON_GetObjectItem(in_root, "HiddenCount");
			cJSON* cj_visible_type = cJSON_GetObjectItem(in_root, "VisibleType");
			cJSON* cj_hidden_type = cJSON_GetObjectItem(in_root, "HiddenType");
			cJSON* cj_visible_biases = cJSON_GetObjectItem(in_root, "VisibleBiases");
			cJSON* cj_hidden_biases = cJSON_GetObjectItem(in_root, "HiddenBiases");
			cJSON* cj_weights = cJSON_GetObjectItem(in_root, "Weights");

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
					
					cJSON* vb_it = cJSON_CreateArrayIterator(cj_visible_biases);

					// copy in visible biases
					for(uint32_t i = 0; i < visible_count; i++)
					{
						cJSON_ArrayIteratorMoveNext(vb_it);
						rbm->visible.biases()[i] = (float)cJSON_ArrayIteratorCurrent(vb_it)->valuedouble;
					}
					cJSON_Delete(vb_it);

					cJSON* hb_it = cJSON_CreateArrayIterator(cj_hidden_biases);

					// copy in hidden biases
					for(uint32_t j = 0; j < hidden_count; j++)
					{
						cJSON_ArrayIteratorMoveNext(hb_it);
						rbm->hidden.biases()[j] = (float)cJSON_ArrayIteratorCurrent(hb_it)->valuedouble;
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
							delete rbm;
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
								rbm->hidden.feature(j)[i] = w_ij;
								rbm->visible.feature(i)[j] = w_ij;
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

			return rbm;
Malformed:
			return nullptr;
		}
		return nullptr;
	}

	RestrictedBoltzmannMachine* RestrictedBoltzmannMachine::FromJSON(const std::string& in_json)
	{
		cJSON* root = cJSON_Parse(in_json.c_str());
		RBM* rbm = FromJSON(root);
		cJSON_Delete(root);
		return rbm;
	}

	RestrictedBoltzmannMachine* RestrictedBoltzmannMachine::FromJSON(std::istream& stream)
	{
		Reader r(stream);

		SetReader(r);
		SetErrorResult(nullptr);
		TryGetToken(Token::BeginObject);
		// make sure we're actually parsing an RBM
		TryGetNameValuePair("Type", Token::String);
		VerifyEqual(r.readString(), "RestrictedBoltzmannMachine");
			
		// get visible, hidden counts and activations
		TryGetNameValuePair("VisibleCount", Token::Number);
		uint64_t visible_count = r.readUInt();
		TryGetNameValuePair("HiddenCount", Token::Number);
		uint64_t hidden_count = r.readUInt();
		TryGetNameValuePair("VisibleType", Token::String);
		ActivationFunction_t visible_type = ParseFunction(r.readString().c_str());
		TryGetNameValuePair("HiddenType", Token::String);
		ActivationFunction_t hidden_type = ParseFunction(r.readString().c_str());

		// validate these parameters
		if(visible_count > (uint64_t)std::numeric_limits<uint32_t>::max() ||
		   visible_count == 0 ||
		   hidden_count > (uint64_t)std::numeric_limits<uint32_t>::max() ||
		   hidden_count == 0 ||
		   visible_type == ActivationFunction::Invalid ||
		   hidden_type == ActivationFunction::Invalid)
		{
			return nullptr;
		}

		// ok, create our RBM
		auto_ptr<RBM> rbm(new RBM(visible_count, hidden_count, visible_type, hidden_type));

		// now load our biases

		TryGetNameValuePair("VisibleBiases", Token::BeginArray);
		for(size_t i = 0; i < rbm->visible_count; i++)
		{
			TryGetToken(Token::Number);
			rbm->visible.biases()[i] = (float)r.readDouble();
		}
		TryGetToken(Token::EndArray);

		TryGetNameValuePair("HiddenBiases", Token::BeginArray);
		for(size_t j = 0; j < rbm->hidden_count; j++)
		{
			TryGetToken(Token::Number);
			rbm->hidden.biases()[j] = (float)r.readDouble();
		}
		TryGetToken(Token::EndArray);

		// now load our weights

		TryGetNameValuePair("Weights", Token::BeginArray);
		for(size_t j = 0; j < rbm->hidden_count; j++)
		{
			TryGetToken(Token::BeginArray);
			for(size_t i = 0; i < rbm->visible_count; i++)
			{
				TryGetToken(Token::Number);
				float w_ij = (float)r.readDouble();
				rbm->hidden.feature(j)[i] = w_ij;
				rbm->visible.feature(i)[j] = w_ij;
			}
			TryGetToken(Token::EndArray);
		}
		TryGetToken(Token::EndArray);
		TryGetToken(Token::EndObject);

		return rbm.release();
	}
}