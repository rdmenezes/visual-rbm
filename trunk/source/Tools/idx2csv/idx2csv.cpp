#include <stdio.h>
#include <stdint.h>

#include "IDX.hpp"

const char* Usage = 
	"Prints the given IDX data file to a CSV file.\n"
	"\n"
	"Usage: idx2csv [INPUT] [OUTPUT]\n"
	"  INPUT   An IDX data file\n"
	"  OUTPUT  Destination to save CSV.  If none\n"
	"          is specified, data printed to stdout.\n";

int main(int argc, char** argv)
{
	if(argc != 3 && argc != 2)
	{
		printf(Usage);
		return -1;
	}

	const char* idx_filename = argv[1];
	const char* csv_filename = argc == 3 ? argv[2] : NULL;

	// we should only print messages if we are writing to file, not stdout
	enum
	{
		StdOut,
		File
	} write_dest;

	IDX* idx = NULL;
	idx = IDX::Load(argv[1]);
	if(idx == NULL)
	{
		printf("Could not load file \"%s\"\n", idx_filename);
		goto CLEANUP;
	}

	FILE* dest = NULL;
	if(csv_filename != NULL)
	{
		dest = fopen(csv_filename, "wb");
		if(dest == NULL)
		{
			printf("Could not open destination file \"%s\"\n", csv_filename);
			goto CLEANUP;
		}
		write_dest = File;
	}
	else
	{
		dest = stdout;
		write_dest = StdOut;
	}

	
	if(write_dest == File)
	{
		printf("Writing CSV ... ");
	}

	void* row_buffer = (uint8_t*)malloc(idx->GetRowLengthBytes());
	for(uint32_t row = 0; row < idx->GetRowCount(); row++)
	{
		idx->ReadRow(row, row_buffer);
		for(uint32_t i = 0; i < idx->GetRowLength(); i++)
		{
			switch(idx->GetDataFormat())
			{
			case UInt8:
				fprintf(dest, "%u,", (uint32_t)*((uint8_t*)row_buffer + i));
				break;
			case SInt8:
				fprintf(dest, "%i,", (int32_t)*((int8_t*)row_buffer + i));
				break;
			case SInt16:
				fprintf(dest, "%i,", (int32_t)*((int16_t*)row_buffer + i));
				break;
			case SInt32:
				fprintf(dest, "%i,", (int32_t)*((int32_t*)row_buffer + i));
				break;
			case Single:
				fprintf(dest, "%.8g,", (float)*((float*)row_buffer + i));
				break;
			case Double:
				fprintf(dest, "%.16g,", (double)*((double*)row_buffer + i));
				break;
			}
		}
		fprintf(dest, "\n");
	}

	if(write_dest == File)
	{
		printf("Done!");
	}

CLEANUP:
	if(idx != NULL)
	{
		delete idx;
	}

	if(dest != NULL)
	{
		fclose(dest);
		dest = NULL;
	}
}