#pragma once

#include "SiCKL.h"

namespace SiCKL
{
	class Program
	{
	public:
		virtual void Run() = 0;
		virtual void Initialize() {};
	};
}