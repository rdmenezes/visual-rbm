#include "IDX.hpp"

#include <cstdint>

const char* Usage = 
	"Joins together multiple IDX files with the same number of rows into a\n"
	"single IDX file.\n"

	"Usage: joinidx [INPUTS] [OUTPUT]\n"
	"  INPUTS      A list of 1 or more IDX files\n"
	"  OUTPUT      Output joined IDX file\n";

int main(int argc, char** argv)
{
	if(argc < 3)
	{
		printf(Usage);
		return -1;
	}

	uint32_t idx_count = argc - 2;

	IDX** inputs = new IDX*[idx_count];
	DataFormat data_format;
	uint32_t rows;
	uint32_t row_length = 0;

	// iterate through each idx file, and verify data is consistent
	for(uint32_t k = 0; k < idx_count; k++)
	{
		IDX* idx = IDX::Load(argv[k + 1]);
		if(idx == NULL)
		{
			printf("Unable to open IDX file %s\n", argv[k + 1]);
			return -1;
		}


		if(k == 0)
		{
			data_format = idx->GetDataFormat();
			rows = idx->GetRowCount();
		}
		else
		{
			if(data_format != idx->GetDataFormat())
			{
				printf("Data type in %s is inconsistent with the other files\n", argv[k + 1]);
				return -1;
			}
			else if(rows != idx->GetRowCount())
			{
				printf("Number of rows in %s is inconsistent with the other files\n", argv[k + 1]);
				return -1;
			}
		}
		row_length += idx->GetRowLength();
		inputs[k] = idx;
	}

	// get the number of bytes per row
	uint32_t data_width = 0;
	switch(data_format)
	{
	case DataFormat::SInt8:
	case DataFormat::UInt8:
		data_width = 1;
		break;
	case DataFormat::SInt16:
		data_width = 2;
		break;
	case DataFormat::SInt32:
	case DataFormat::Single:
		data_width = 4;
		break;
	case DataFormat::Double:
		data_width = 8;
		break;
	}
	
	uint8_t* row_buffer = new uint8_t[row_length * data_width];

	// last filename is the result
	IDX* output = IDX::Create(argv[argc - 1], LittleEndian, data_format, row_length);
	if(output == NULL)
	{
		printf("Unable to create output file %s\n", argv[argc - 1]);
		return -1;
	}

	printf("Writing %s to disk ... \n", argv[argc-1]);

	for(uint32_t k = 0; k < rows; k++)
	{
		uint32_t offset = 0;
		for(uint32_t i = 0 ; i < idx_count; i++)
		{
			inputs[i]->ReadRow(k, row_buffer + offset);
			offset += inputs[i]->GetRowLength() * data_width;
		}
		output->AddRow(row_buffer);
	}
	// write out header
	output->Close();

	// cleanup
	delete output;
	for(uint32_t i = 0; i < idx_count; i++)
	{
		delete inputs[i];
	}
	delete[] inputs;

	printf("Done!\n");

	return 0;
}