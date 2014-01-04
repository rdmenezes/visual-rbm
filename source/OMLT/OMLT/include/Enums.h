#pragma once

namespace OMLT
{
	// a general parsing method
	namespace ModelType
	{
		enum Enum
		{
			Invalid = -1,
			NotSet = 0,
			MultilayerPerceptron,
			RestrictedBoltzmannMachine,

			MLP = MultilayerPerceptron,
			RBM = RestrictedBoltzmannMachine,
		};
	}
	typedef ModelType::Enum ModelType_t;

	namespace ActivationFunction
	{
		enum Enum
		{
			// linear: f(x) = x
			Linear,
			// rectified linear function : f(x) = max(0, x)
			RectifiedLinear,
			// sigmoid function: f(x) == 1 / (1 + exp(-x))
			Sigmoid,
			// the total number of function
			Count,
		};
	}
	typedef ActivationFunction::Enum ActivationFunction_t;

	extern const char* ActivationFunctionNames[];
	
}