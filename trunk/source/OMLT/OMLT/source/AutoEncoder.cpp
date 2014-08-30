// stdc
#include <string.h>

// c++
#include <memory>
using std::auto_ptr;

// extern
#include <cJSON.h>
#include <cppJSONStream.hpp>
using namespace cppJSONStream;

// OMLT
#include "AutoEncoder.h"
#include "Common.h"

namespace OMLT
{
	AutoEncoder::AutoEncoder( uint32_t in_visible_count, uint32_t in_hidden_count, ActivationFunction_t in_hidden_type, ActivationFunction_t in_output_type )
		: visible_count(in_visible_count),
		  hidden_count(in_hidden_count),
		  hidden_type(in_hidden_type),
		  output_type(in_output_type),
		  encoder(visible_count, hidden_count),
		  decoder(hidden_count, visible_count)
	{

	}

	void AutoEncoder::Encode( const float* in_raw, float* out_encoded ) const
	{
		encoder.CalcFeatureVector(in_raw, out_encoded, hidden_type);
	}

	void AutoEncoder::Decode( const float* in_decoded, float* out_raw ) const
	{
		decoder.CalcFeatureVector(in_decoded, out_raw, output_type);
	}

	void AutoEncoder::ToJSON(std::ostream& stream) const
	{
		cppJSONStream::Writer w(stream, true);

		w.begin_object();
			w.write_namevalue("Type", "AutoEncoder");
			w.write_namevalue("VisibleCount", (uint64_t)visible_count);
			w.write_namevalue("HiddenCount", (uint64_t)hidden_count);
			w.write_namevalue("OutputType", ActivationFunctionNames[output_type]);
			w.write_namevalue("HiddenType", ActivationFunctionNames[hidden_type]);
			w.write_name("HiddenBiases");
				w.write_array(encoder.biases(), hidden_count);
			w.write_name("OutputBiases");
				w.write_array(decoder.biases(), visible_count);
			w.write_name("Weights");
				w.begin_array();
				for(uint32_t j = 0; j < hidden_count; j++)
				{
					w.write_array(encoder.feature(j), visible_count);
				}
				w.end_array();
		w.end_object();
	}


	AutoEncoder* AutoEncoder::FromJSON(std::istream& stream)
	{
		Reader r(stream);

		SetReader(r);
		SetErrorResult(nullptr);
		
		TryGetToken(Token::BeginObject);
		// make sure we're actually parsing an AutoEncoder
		TryGetNameValuePair("Type", Token::String);
		VerifyEqual(r.readString(), "AutoEncoder");

		// get visible, hidden counts and output and hidden activation function
		TryGetNameValuePair("VisibleCount", Token::Number);
		uint64_t visible_count = r.readUInt();
		TryGetNameValuePair("HiddenCount", Token::Number);
		uint64_t hidden_count = r.readUInt();
		TryGetNameValuePair("OutputType", Token::String);
		ActivationFunction_t output_type = ParseFunction(r.readString().c_str());
		TryGetNameValuePair("HiddenType", Token::String);
		ActivationFunction_t hidden_type = ParseFunction(r.readString().c_str());

		// validate these parameters
		if(visible_count > (uint64_t)std::numeric_limits<uint32_t>::max() ||
			visible_count == 0 ||
			hidden_count > (uint64_t)std::numeric_limits<uint32_t>::max() ||
			hidden_count == 0 ||
			output_type == ActivationFunction::Invalid ||
			hidden_type == ActivationFunction::Invalid)
		{
			return nullptr;
		}

		// create our AutoEncoder

		auto_ptr<AutoEncoder> ae(new AutoEncoder(visible_count, hidden_count, hidden_type, output_type));

		// load biases

		TryGetNameValuePair("HiddenBiases", Token::BeginArray);
		for(size_t j = 0; j < hidden_count; j++)
		{
			TryGetToken(Token::Number);
			ae->encoder.biases()[j] = (float)r.readDouble();
		}
		TryGetToken(Token::EndArray);

		TryGetNameValuePair("OutputBiases", Token::BeginArray);
		for(size_t i = 0; i < visible_count; i++)
		{
			TryGetToken(Token::Number);
			ae->decoder.biases()[i] = (float)r.readDouble();
		}
		TryGetToken(Token::EndArray);

		// load weights
		TryGetNameValuePair("Weights", Token::BeginArray);
		for(size_t j = 0; j < ae->hidden_count; j++)
		{
			TryGetToken(Token::BeginArray);
			for(size_t i = 0; i < ae->visible_count; i++)
			{
				TryGetToken(Token::Number);
				float w_ij = (float)r.readDouble();
				ae->encoder.feature(j)[i] = w_ij;
				ae->decoder.feature(i)[j] = w_ij;
			}
			TryGetToken(Token::EndArray);
		}
		TryGetToken(Token::EndArray);
		TryGetToken(Token::EndObject);

		return ae.release();
	}
}