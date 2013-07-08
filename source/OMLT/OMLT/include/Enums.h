#pragma once

namespace OMLT
{
	enum ActivationFunction
	{
		// linear: f(x) -> x
		Linear,
		// sigmoid function: f(x) -> 1 / (1 + exp(-x))
		Sigmoid,
		// gaussian noise added
		NoisySigmoid,
		// rectified linear function : f(x) -> max(0, x)
		RectifiedLinear,
		// guassian noise added
		NoisyRectifiedLinear,
		// the total number of function
		Count,
	};

	extern const char* ActivationFunctionNames[];
	
}