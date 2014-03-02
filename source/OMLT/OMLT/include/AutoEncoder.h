#pragma once

// stdlib
#include <stdint.h>
#include <string>

#include "Enums.h"
#include "Model.h"
#include "Common.h"

struct cJSON;
namespace OMLT
{
	class AutoEncoder
	{
	public:
		void Encode(const float* in_raw, float* out_encoded) const;
		void Decode(const float* in_decoded, float* out_raw) const;

		std::string ToJSON() const;
		static AutoEncoder* FromJSON(const std::string& in_json);

		const uint32_t hidden_count;
		const uint32_t visible_count;

		const ActivationFunction_t hidden_type;
		const ActivationFunction_t output_type;
	private:
		FeatureMap encoder;
		FeatureMap decoder;
		AutoEncoder(uint32_t in_visible_count, uint32_t in_hidden_count, ActivationFunction_t in_hidden_type, ActivationFunction_t in_output_type);

		// private parse method
		static AutoEncoder* FromJSON(struct cJSON* root);

		friend struct Model;
		friend class AutoEncoderBackPropagation;
	};
	typedef AutoEncoder AE;
}