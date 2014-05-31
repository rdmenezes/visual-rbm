#include <stdint.h>

#include "Common.h"
#include "Enums.h"


namespace OMLT
{
	const char* ActivationFunctionNames[] =
	{
		"Linear",
		"RectifiedLinear",
		"Sigmoid",
		"Softmax",
	};

	ActivationFunction_t ParseFunction(const char* name)
	{
		for(uint32_t k = 0; k < ArraySize(ActivationFunctionNames); k++)
		{
			if(strcmp(name, ActivationFunctionNames[k]) == 0)
			{
				return (ActivationFunction_t)k;
			}
		}

		return ActivationFunction::Invalid;
	}
}