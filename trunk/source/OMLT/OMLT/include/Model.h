#pragma once

#include <string>

namespace OMLT
{
	namespace ModelType
	{
		enum Enum
		{
			Invalid = -1,
			NotSet = 0,
			MultilayerPerceptron,
			RestrictedBoltzmannMachine,
			AutoEncoder,

			MLP = MultilayerPerceptron,
			RBM = RestrictedBoltzmannMachine,
			AE = AutoEncoder,
		};
	}
	typedef ModelType::Enum ModelType_t;

	struct Model
	{
		ModelType_t type;
		union
		{
			void* ptr;
			class MultilayerPerceptron* mlp;
			class RestrictedBoltzmannMachine* rbm;
			class AutoEncoder* ae;
		};
		Model() : type(ModelType::NotSet), ptr(nullptr) {}

		static bool FromJSON(const std::string& in_json, Model& out_model);
	};
}