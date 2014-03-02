#include <stdint.h>

#include <random>
#include <algorithm>
using std::swap;
#include <IDX.hpp>
using OMLT::IDX;

void printHelp()
{
	const char* Usage =
		"Shuffle an IDX file into a new order.  Shuffling is deterministic based\n"
		"off of the length of the IDX file (different IDX files with identical\n"
		"length will be shuffled into the same order).\n"
		"\n"
		"Usage: shuffleidx [INPUT] [OUTPUT]\n"
		"  INPUT     An input IDX data file\n"
		"  OUTPUT    Destination to save shuffled IDX file";
	printf(Usage);
}

int main(int argc, char** argv)
{
	int result = -1;
	// our filenames
	const char* input_filename = argv[1];
	const char* shuffled_filename = argv[2];
	// idx files
	IDX* input = nullptr;
	IDX* shuffled = nullptr;
	// buffer of indices for shuffling
	uint32_t* index_buffer = nullptr;
	// buffer to read and write data rwos
	void* row_buffer = nullptr;
	if(argc != 3)
	{
		printHelp();
		goto CLEANUP;
	}

	input = IDX::Load(input_filename);
	if(!input)
	{
		printf("Problem loading \"%s\" as IDX file.\n", input_filename);
		goto CLEANUP;
	}

	shuffled = IDX::Create(shuffled_filename, input->GetEndianness(), input->GetDataFormat(), input->GetRowLength());
	if(!shuffled)
	{
		printf("Could not create new IDX file \"%s\"\n", shuffled_filename);
		goto CLEANUP;
	}

	printf("Shuffling %s to %s...\n", input_filename, shuffled_filename);

	index_buffer = new uint32_t[input->GetRowCount()];
	// Fisher–Yates shuffle
	{
		std::mt19937_64 random;
		random.seed(1);	// deterministic
		for(uint32_t k = 0; k < input->GetRowCount(); k++)
		{
			index_buffer[k] = k;
		}
		for(uint32_t k = input->GetRowCount() - 1; k > 0; k--)
		{
			std::uniform_int_distribution<uint32_t> uniform(0, k);
			uint32_t j = uniform(random);
			swap(index_buffer[j], index_buffer[k]);
		}
	}

	// now write shuffled idx file
	row_buffer = malloc(input->GetRowLengthBytes());
	printf("Writing %s to disk ... \n", shuffled_filename);
	const uint32_t row_count = input->GetRowCount();
	for(uint32_t k = 0; k < row_count; k++)
	{
		uint32_t new_index  = index_buffer[k];
		input->ReadRow(new_index, row_buffer);
		shuffled->AddRow(row_buffer);
	}

	printf("Done!\n");

	result = 0;
CLEANUP:

	if(input)
	{
		input->Close();
	}
	if(shuffled)
	{
		shuffled->Close();
	}

	delete input;
	delete shuffled;
	delete[] index_buffer;
	free(row_buffer);

	return result;
}