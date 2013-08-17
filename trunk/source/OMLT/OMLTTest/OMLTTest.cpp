#include <stdint.h>
#include <stdio.h>

#include "OMLTTest.h"

template<typename T, size_t N>
size_t ArraySize(const T (&)[N])
{
	return N;
};

void print_tests()
{
	printf("Available Tests:\n");
	for(uint32_t k = 0; k < ArraySize(TestList); k++)
	{
		printf(" %u : %s\n", k, TestList[k].Name);
	}
}

int main(int argc, char** argv)
{
	setbuf(stdout, nullptr);

	if(argc == 1)
	{
		print_tests();
	}
	else if(argc >= 2)
	{
		uint32_t index;
		if(sscanf(argv[1], "%u", &index) == 1)
		{
			if(index < ArraySize(TestList))
			{
				printf("Running %s...\n", TestList[index].Name);
				printf("%s\n", (TestList[index].Func(argc - 2, argv + 2) ? "Success!" : "Failure!"));
			}
			else
			{
				print_tests();
			}
		}
		else
		{
			printf("Could not parse \"%s\" as index\n", argv[1]);
		}
	}

	getc(stdin);
	return 0;
}