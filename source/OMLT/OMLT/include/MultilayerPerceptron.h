#pragma once

// std
#include <stdint.h>
#include <vector>

#include "Common.h"
#include "Enums.h"
#include "Model.h"

struct cJSON;
namespace OMLT
{
	class MultilayerPerceptron
	{
	public:
		MultilayerPerceptron();
		~MultilayerPerceptron();
		void FeedForward(float* input_vector, float* output_vector) const;
		// feedforward to the given layer
		void FeedForward(float* input_vector, float* output_vector, uint32_t layer) const;

		struct Layer
		{
			uint32_t inputs;
			uint32_t outputs;
			ActivationFunction_t function;
			FeatureMap weights;

			Layer(uint32_t in_inputs, uint32_t in_outputs, ActivationFunction_t in_function);
		};

		Layer* GetLayer(uint32_t index);
		Layer* InputLayer() {return GetLayer(0);}
		Layer* OutputLayer() {return GetLayer(_layers.size() - 1);}

		bool AddLayer(Layer*);
		uint32_t LayerCount() const {return _layers.size();}

		// serialization
		std::string ToJSON() const;
		static MultilayerPerceptron* FromJSON(const std::string& in_JSON);
	private: 
		std::vector<Layer*> _layers;
		// properly aligned scratch buffers
		mutable std::vector<float*> _activations;
		// private parse method
		static MultilayerPerceptron* FromJSON(cJSON* root);
		friend bool Model::FromJSON(const std::string& in_json, struct Model& out_model);
		friend class BackPropagation;
	};
	typedef MultilayerPerceptron MLP;
}