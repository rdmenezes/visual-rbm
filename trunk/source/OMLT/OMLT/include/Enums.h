#pragma once

namespace OMLT
{
	enum ActivationFunction
	{
		// no activation function
		Linear,
		// sigmoid function
		Sigmoid,
		// gaussian noise added to unit accumulation
		NoisySigmoid,
	};

	extern const char* ActivationFunctionNames[];
	
}