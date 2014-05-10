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
	class RestrictedBoltzmannMachine
	{
	public:
		void CalcHidden(const float* in_visible, float* out_hidden) const;
		void CalcVisible(const float* in_hidden, float* out_visible) const;

		// assumption that both the visible and hidden types are sigmoid
		float CalcFreeEnergy(const float* in_visible) const;

		std::string ToJSON() const;
		static RestrictedBoltzmannMachine* FromJSON(const std::string& in_JSON);

		const uint32_t visible_count;
		const uint32_t hidden_count;

		ActivationFunction_t visible_type;
		ActivationFunction_t hidden_type;

		// used to calculate the hidden feature vector
		FeatureMap hidden;
		// used to calculate visible feature vector
		FeatureMap visible;
	private:

		
		RestrictedBoltzmannMachine(uint32_t in_visible_count, uint32_t in_hidden_count, ActivationFunction_t in_visible_type, ActivationFunction_t in_hidden_type);

		// private parse method
		static RestrictedBoltzmannMachine* FromJSON(struct cJSON* root);
		
		friend struct Model;
		friend class ContrastiveDivergence;
	};
	typedef RestrictedBoltzmannMachine RBM;
}