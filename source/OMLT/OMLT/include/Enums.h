#pragma once

namespace OMLT
{


	namespace ActivationFunction
	{
		enum Enum
		{
			Invalid = -1,
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

	extern ActivationFunction_t ParseFunction(const char* name);
	
}