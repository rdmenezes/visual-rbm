#include <stdint.h>
#include <stdio.h>
#include <iostream>

#include "OMLTTest.h"

template<typename T, size_t N>
size_t ArraySize(const T (&)[N])
{
	return N;
};

int main(int argc, char** argv)
{
	for(uint32_t k = 0; k < ArraySize(TestList); k++)
	{
		std::cout << "Begin Test \"" << TestList[k].Name << "\"\n";
		std::cout << (TestList[k].Func(argc, argv) ? " Success!" : " Failure") << std::endl << std::endl;
	}

	getc(stdin);
}