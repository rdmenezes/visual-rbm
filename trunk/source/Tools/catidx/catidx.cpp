#include "IDX.hpp"

const char* Usage = 
	"Concatenates together multiple IDX files with identical row dimensions\n"
	"and differing row count into a single IDX file containing all of the\n"
	"input IDX files' rows.\n"
	"\n"
	"Usage: catidx [INPUTS] [OUTPUT]\n"
	"  INPUTS      A list of 1 or more IDX files\n"
	"  OUTPUT      Output concatenated IDX file\n";

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
	uint32_t* row_dimensions = NULL;
	uint8_t row_dimensions_count;
	uint32_t* temp_row_dimensions = NULL;
	// our output IDX file
	IDX* output = NULL;
	// temp buffer for reading/writing rows
	void* row_buffer = NULL;
	// iterate through each idx file, and verify data is consistent
	for(uint32_t  k = 0; k < idx_count; k++)
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
			row_dimensions_count = idx->GetRowDimensionsCount();
			row_dimensions = new uint32_t[row_dimensions_count];
			temp_row_dimensions = new uint32_t[row_dimensions_count];
			idx->GetRowDimensions(row_dimensions);
		}
		else
		{
			if(data_format != idx->GetDataFormat())
			{
				printf("Data type in %s is inconsistent with the other files\n", idx_filename);
				goto CLEANUP;
			}
			else if(row_dimensions_count != idx->GetRowDimensionsCount())
			{
				printf("%s has different number of dimensions than the other files\n", idx_filename);
				goto CLEANUP;
			}
			else
			{
				idx->GetRowDimensions(temp_row_dimensions);
				for(uint32_t k = 1; k < row_dimensions_count; k++)
				{
					if(temp_row_dimensions[k] != row_dimensions[k])
					{
						printf("%s has dimensions sizes different from the other files\n", idx_filename);
						goto CLEANUP;
					}
				}
			}
		}
	}

	// create output file
	const char* output_filename = argv[argc - 1];
	// increment row_dimensions + 1 to skip over row count
	// decrement row_dimensions_count - 1 to remove row count 
	output = IDX::Create(output_filename, LittleEndian, data_format, row_dimensions + 1, row_dimensions_count - 1);
	if(output == NULL)
	{
		printf("Unable to create output file %s\n", output_filename);
		goto CLEANUP;
	}
	printf("Writing %s to disk ... \n", output_filename);

	row_buffer = malloc(output->GetRowLengthBytes());
	// now concatenate all these guys together
	for(uint32_t i = 0; i < idx_count; i++)
	{
		const char* input_filename = argv[i + 1];
		printf(" Concatenating %s...\n", input_filename);
		for(uint32_t k = 0; k < inputs[i]->GetRowCount(); k++)
		{
			inputs[i]->ReadRow(k, row_buffer);
			output->AddRow(row_buffer);
		}
	}
	// write out header
	output->Close();

	printf("Done!\n");

	// success!
	result = 0;

	// cleanup
CLEANUP:

	for(uint32_t i = 0; i < idx_count; i++)
	{
		if(inputs[i])
		{
			delete inputs[i];
		}
	}
	delete[] inputs;

	if(row_dimensions)
	{
		delete row_dimensions;
	}
	if(temp_row_dimensions)
	{
		delete temp_row_dimensions;
	}
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