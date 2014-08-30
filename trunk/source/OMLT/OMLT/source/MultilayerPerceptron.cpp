// c
#include <assert.h>

// c++
#include <memory>
using std::auto_ptr;

 // windows
#include <malloc.h>
#include <intrin.h>

// extern
#include <cJSON.h>
#include <cppJSONStream.hpp>
using namespace cppJSONStream;

// OMLT
#include "MultilayerPerceptron.h"
#include "Common.h"

namespace OMLT
{
	MultilayerPerceptron::MultilayerPerceptron()
	{
		_activations.push_back(nullptr);
		_activations.push_back(nullptr);
	}

	MultilayerPerceptron::~MultilayerPerceptron()
	{
		for(uint32_t k = 0; k < _layers.size(); k++)
		{
			delete _layers[k];
		}

		for(uint32_t k = 0; k < _activations.size(); k++)
		{
			AlignedFree(_activations[k]);
		}
	}

	void MultilayerPerceptron::FeedForward(float* input_vector, float* output_vector) const
	{
		FeedForward(input_vector, output_vector, _layers.size() - 1);
	}

	void MultilayerPerceptron::FeedForward(float* input_vector, float* output_vector, uint32_t last_layer) const
	{
		assert(last_layer < _layers.size());

		_activations.front() = input_vector;
		_activations.back() = output_vector;
		for(size_t k = 0; k <= last_layer; k++)
		{
			_layers[k]->weights.CalcFeatureVector(_activations[k], _activations[k+1], _layers[k]->function);
		}
		_activations.front() = nullptr;
		_activations.back() = nullptr;
	}

	MultilayerPerceptron::Layer* MultilayerPerceptron::GetLayer( uint32_t index )
	{
		assert(index < _layers.size());
		return _layers[index];
	}

	bool MultilayerPerceptron::AddLayer( Layer* in_layer)
	{
		if(_layers.size() > 0)
		{
			if(_layers.back()->outputs != in_layer->inputs)
			{
				return false;
			}

			size_t input_alloc_size = sizeof(float) * BlockCount(in_layer->inputs) * 4;
			
			float* buffer = (float*)AlignedMalloc(input_alloc_size, 16);
			memset(buffer, 0x00, input_alloc_size);

			_activations.back() = buffer;
			_activations.push_back(nullptr);
		}

		_layers.push_back(in_layer);

		return true;
	}

	void MultilayerPerceptron::ToJSON(std::ostream& stream) const
	{
		cppJSONStream::Writer w(stream, true);

		w.begin_object();
			w.write_namevalue("Type", "MultilayerPerceptron");
			w.write_name("Layers");
			w.begin_array();
				for(auto it = _layers.begin(); it < _layers.end(); ++it)
				{
					w.begin_object();
						w.write_namevalue("Inputs", (uint64_t)(*it)->inputs);
						w.write_namevalue("Outputs", (uint64_t)(*it)->outputs);
						w.write_namevalue("Function", ActivationFunctionNames[(*it)->function]);
						w.write_name("Biases");
						w.write_array((*it)->weights.biases(), (*it)->outputs);
						w.write_name("Weights");
						w.begin_array();
							for(uint32_t j = 0; j < (*it)->outputs; j++)
							{
								w.write_array((*it)->weights.feature(j), (*it)->inputs);
							}
						w.end_array();
					w.end_object();
				}
			w.end_array();
		w.end_object();
	}

	MultilayerPerceptron* MultilayerPerceptron::FromJSON(std::istream& stream)
	{
		Reader r(stream);

		SetReader(r);
		SetErrorResult(nullptr);
		TryGetToken(Token::BeginObject);

		// ensure we're parsing an MLP
		TryGetNameValuePair("Type", Token::String);
		VerifyEqual(r.readString(), "MultilayerPerceptron");

		TryGetNameValuePair("Layers", Token::BeginArray);

		// create our MLP
		auto_ptr<MLP> mlp(new MLP());

		// read each layer
		for(Token_t t = r.next(); t != Token::EndArray; t = r.next())
		{
			VerifyEqual(t, Token::BeginObject);
			
			TryGetNameValuePair("Inputs", Token::Number);
			uint64_t inputs = r.readUInt();
			TryGetNameValuePair("Outputs", Token::Number);
			uint64_t outputs = r.readUInt();
			TryGetNameValuePair("Function", Token::String);
			ActivationFunction_t function = ParseFunction(r.readString().c_str());
			
			// validate these parameters
			if(inputs > (uint64_t)std::numeric_limits<uint32_t>::max() ||
			  inputs == 0 ||
			  outputs > (uint64_t)std::numeric_limits<uint32_t>::max() ||
			  outputs == 0 ||
			  function == ActivationFunction::Invalid)
			{
				return nullptr;
			}

			auto_ptr<Layer> layer(new Layer(inputs, outputs, function));

			// set biases
			TryGetNameValuePair("Biases", Token::BeginArray);
			for(uint32_t j = 0; j < outputs; j++)
			{
				TryGetToken(Token::Number);
				layer->weights.biases()[j] = (float)r.readDouble();
			}
			TryGetToken(Token::EndArray);

			// set weights
			TryGetNameValuePair("Weights", Token::BeginArray);
			for(uint32_t j = 0; j < outputs; j++)
			{
				TryGetToken(Token::BeginArray);
				for(uint32_t i = 0; i < inputs; i++)
				{
					TryGetToken(Token::Number);
					layer->weights.feature(j)[i] = (float)r.readDouble();
				}
				TryGetToken(Token::EndArray);
			}
			TryGetToken(Token::EndArray);
			mlp->AddLayer(layer.release());
		}
		TryGetToken(Token::EndObject);

		return mlp.release();
	}

	MultilayerPerceptron::Layer::Layer( uint32_t in_inputs, uint32_t in_outputs, ActivationFunction_t in_function )
		: inputs(in_inputs)
		, outputs(in_outputs)
		, function(in_function)
		, weights(inputs, outputs)
	{ }
}