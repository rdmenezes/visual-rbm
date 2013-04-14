#pragma once

#include "SiCKL.h"

namespace SiCKL
{
	template<typename PROGRAM>
	class Compiler
	{
	public:
		virtual PROGRAM* Build(const Source&) = 0;
	};
}