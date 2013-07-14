#pragma once

namespace OMLT
{
	enum ActivationFunction
	{
		// linear: f(x) -> x
		Linear,
		// rectified linear function : f(x) -> max(0, x)
		RectifiedLinear,
		// sigmoid function: f(x) -> 1 / (1 + exp(-x))
		Sigmoid,
		// the total number of function
		Count,
	};

	extern const char* ActivationFunctionNames[];
	
}