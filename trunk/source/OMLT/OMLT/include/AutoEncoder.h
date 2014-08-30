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

		void ToJSON(std::ostream& stream) const;

		static AutoEncoder* FromJSON(std::istream& stream);

		const uint32_t hidden_count;
		const uint32_t visible_count;

		const ActivationFunction_t hidden_type;
		const ActivationFunction_t output_type;

		FeatureMap encoder;
		FeatureMap decoder;
	private:
		AutoEncoder(uint32_t in_visible_count, uint32_t in_hidden_count, ActivationFunction_t in_hidden_type, ActivationFunction_t in_output_type);

		friend class AutoEncoderBackPropagation;
	};
	typedef AutoEncoder AE;
}