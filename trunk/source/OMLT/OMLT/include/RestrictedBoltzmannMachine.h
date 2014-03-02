#pragma once

// stdlib
#include <stdint.h>
#include <string>

#include "Enums.h"
#include "Model.h"

struct cJSON;
namespace OMLT
{
	class RestrictedBoltzmannMachine
	{
	public:
		RestrictedBoltzmannMachine(uint32_t in_visible_count, uint32_t in_hidden_count, ActivationFunction_t in_visible_type, ActivationFunction_t in_hidden_type);
		~RestrictedBoltzmannMachine();

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

		float* visible_biases;
		float* hidden_biases;
		// v length h features
		float** visible_features;
		// h length v features
		float** hidden_features;
	private:
		// properly aligned scratch buffers
		float* _visible_buffer;
		float* _hidden_buffer;
		// private parse method
		static RestrictedBoltzmannMachine* FromJSON(struct cJSON* root);
		friend bool Model::FromJSON(const std::string& in_json, struct Model& out_model);
	};
	typedef RestrictedBoltzmannMachine RBM;
}