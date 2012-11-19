#include <stdio.h>

#include "IDX.hpp"

const char* Usage =
	"Prints the header info of the given IDX files.\n"
	"\n"
	"Usage: idxinfo [IDXFILES]\n"
	"  IDXFILES   A list of 1 or more IDX files\n";

int main(int argc, char** argv)
{
	if(argc == 1)
	{
		printf(Usage);
		return -1;
	}

	for(uint32_t k = 1; k < argc; k++)
	{
		const char* idx_filename = argv[k];
		IDX* idx = IDX::Load(idx_filename, false);

		if(idx == NULL)
		{
			printf("Could not load file \"%s\"\n\n", idx_filename);
			continue;
		}

		printf("IDX Header for \"%s\"\n", idx_filename);
		printf(" Endianness: %s\n", idx->GetEndianness() == BigEndian ? "Big Endian" : "Little Endian");
		printf(" Data Format: ");
		switch(idx->GetDataFormat())
		{
		case DataFormat::UInt8:
			printf("UInt8\n");
			break;
		case DataFormat::SInt8:
			printf("SInt8\n");
			break;
		case DataFormat::SInt16:
			printf("SInt16\n");
			break;
		case DataFormat::SInt32:
			printf("SInt32\n");
			break;
		case DataFormat::Single:
			printf("Single\n");
			break;
		case DataFormat::Double:
			printf("Double\n");
			break;
		}
		uint32_t rdc = idx->GetRowDimensionsCount();
		printf(" Dimensions: %u\n", rdc);
		uint32_t* dimensions = new uint32_t[rdc];
		idx->GetRowDimensions(dimensions);
		for(uint32_t k = 0; k < rdc; k++)
		{
			printf("  Dim %u: %u\n", (k+1), dimensions[k]);
		}
		printf("\n");
		delete idx;
	}

	return 0;
}