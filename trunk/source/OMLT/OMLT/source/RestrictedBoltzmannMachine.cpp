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