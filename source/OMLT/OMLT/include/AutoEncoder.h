#pragma once

// stdlib
#include <stdint.h>
#include <string>

#include "Enums.h"

struct cJSON;
namespace OMLT
{
	class AutoEncoder
	{
	public:
		AutoEncoder(uint32_t in_visible_count, uint32_t in_hidden_count, ActivationFunction_t in_hidden_type, ActivationFunction_t in_output_type);
		~AutoEncoder();

		void Encode(const float* in_raw, float* out_encoded) const;
		void Decode(const float* in_decoded, float* out_raw) const;

		const uint32_t hidden_count;
		const uint32_t visible_count;

		const ActivationFunction_t hidden_type;
		const ActivationFunction_t output_type;

		float* hidden_biases;
		float* output_biases;

		float** encoding_features;
		float** decoding_features;
	private:
		// aligned scratch buffers
		float** _hidden_buffer;
		float** _output_buffer;

		static AutoEncoder* FromJSON(struct cJSON* root);
		friend bool FromJSON(const std::string& in_json, struct Model& out_model);
	};
}