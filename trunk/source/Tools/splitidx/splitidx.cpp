#include <IDX.hpp>
using namespace OMLT;


#include <cstdint>


const char* Usage = 
	"Split a subset of an IDX data file into a smaller IDX file.\n"
	"\n"
	"Usage: splitidx [INPUT] [OUTPUT] [FROM] [COUNT]\n"
	"  INPUT      An input IDX data file\n"
	"  OUTPUT     Destination for IDX subset file\n"
	"  FROM       The start index to start copying from the INPUT IDX\n"
	"  COUNT      The number of rows to copy to the OUTPUT IDX.  If\n"
	"             COUNT is omitted, then all of the remaining rows\n"
	"             Will be saved to OUTPUT IDX.";

int main(int argc, char** argv)
{
	int result = -1;

	if(argc < 4)
	{
		printf(Usage);
		return result;
	}
	
	const char* input_string = argv[1];
	const char* output_string = argv[2];
	const char* from_string = argv[3];

	IDX* input = NULL;
	IDX* output = NULL;
	
	input = IDX::Load(input_string, false);
	if(input == NULL)
	{
		printf("Unable to open input IDX file \"%s\" for reading\n", input_string);
		goto CLEANUP;
	}
	
	output = IDX::Create(output_string, input->GetEndianness(), input->GetDataFormat(), input->GetRowLength());
	if(output == NULL)
	{
		printf("Unable to create output IDX file \"%s\"\n", output_string);
		goto CLEANUP;
	}

	uint32_t from = -1;
	if(sscanf(from_string, "%u", &from) != 1)
	{
		printf("Could not parse \"%s\" as from index\n", from_string);
		goto CLEANUP;
	}
	else if(from >= input->GetRowCount())
	{
		printf("From index must be less than the number of rows in input file \"%s\" (%u)\n", input_string, input->GetRowCount());
		goto CLEANUP;
	}
	uint32_t count = 0;
	if(argc == 5)
	{
		const char* count_string = argv[4];
		if(sscanf(count_string, "%u", &count) != 1)
		{
			printf("Could not parse \"%s\" as count\n", count_string);
			goto CLEANUP;
		}
		else if(from + count > input->GetRowCount())
		{
			printf("From index plus count exceeds number of rows in input\n");
			goto CLEANUP;
		}
	}
	else
	{
		count = input->GetRowCount() - from;
	}

	void* row_buffer = malloc(input->GetRowLengthBytes());
	printf("Writing %s to disk ... \n", output_string);

	for(uint32_t i = 0; i < count; i++)
	{
		input->ReadRow(from + i, row_buffer);
		output->AddRow(row_buffer);
	}
	free(row_buffer);

	input->Close();
	output->Close();
	
	printf("Done!\n");

	result = 0;
CLEANUP:

	if(input != NULL)
	{
		delete input;
	}

	if(output != NULL)
	{
		delete output;
	}

	return result;
}