#include <IDX.hpp>
using namespace OMLT;

#include <cstdint>

const char* Usage = 
	"Joins together multiple IDX files with the same number of rows into a\n"
	"single IDX file.\n"
	"\n"
	"Usage: joinidx [INPUTS] [OUTPUT]\n"
	"  INPUTS      A list of 1 or more IDX files\n"
	"  OUTPUT      Output joined IDX file\n";

int main(int argc, char** argv)
{
	int result = -1;

	if(argc < 4)
	{
		printf(Usage);
		return result;
	}

	uint32_t idx_count = argc - 2;
	IDX** inputs = new IDX*[idx_count];
	memset(inputs, NULL, sizeof(IDX*) * idx_count);
	DataFormat data_format;
	uint32_t rows;
	uint32_t row_length = 0;
	// our output IDX file
	IDX* output = NULL;
	// temp buffer for reading/writing rows
	uint8_t* row_buffer = NULL;

	// iterate through each idx file, and verify data is consistent
	for(uint32_t k = 0; k < idx_count; k++)
	{
		const char* idx_filename = argv[k+1];
		IDX* idx = IDX::Load(idx_filename);
		inputs[k] = idx;

		if(idx == NULL)
		{
			printf("Unable to open IDX file \"%s\" for reading\n", idx_filename);
			goto CLEANUP;
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
				printf("Data type in %s is inconsistent with the other files\n", idx_filename);
				goto CLEANUP;
			}
			else if(rows != idx->GetRowCount())
			{
				printf("Number of rows in %s is inconsistent with the other files\n", idx_filename);
				goto CLEANUP;
			}
		}

		row_length += idx->GetRowLength();
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
	
	const char* output_filename = argv[argc - 1];
	// last filename is the result
	output = IDX::Create(output_filename, LittleEndian, data_format, row_length);
	if(output == NULL)
	{
		printf("Unable to create output file %s\n", output_filename);
		goto CLEANUP;
	}

	row_buffer = (uint8_t*)malloc(output->GetRowLengthBytes());

	printf("Writing %s to disk ... \n", output_filename);

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

	printf("Done!\n");

	// success!
	result = 0;

CLEANUP:

	for(uint32_t i = 0; i < idx_count; i++)
	{
		if(inputs[i])
		{
			delete inputs[i];
		}
	}
	delete[] inputs;

	if(output)
	{
		delete output;
	}
	if(row_buffer)
	{
		free(row_buffer);
	}

	return result;
}