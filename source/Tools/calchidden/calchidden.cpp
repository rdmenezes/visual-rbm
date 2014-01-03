#include <IDX.hpp>
#include <RestrictedBoltzmannMachine.h>
using namespace OMLT;

#include <string.h>
#include <math.h>
#include <sstream>

const char* Usage = 
	"Calculates the hidden activations or the hidden probabilities for an\n"
	"input IDX dataset and trained RBM.\n"
	"\n"
	"Usage: calchidden [MODE] [INPUT] [OUTPUT] [RBM] [DROPOUT]\n"
	"\n"
	"  INPUT     The input IDX dataset used as input\n"
	"  OUTPUT    Destination IDX to save hidden values\n"
	"  RBM       A trained RBM json file\n";

int main(int argc, char** argv)
{
	int result = -1;

	if(argc != 6)
	{
		printf(Usage);
		return result;
	}

	const char* input_string = argv[1];
	const char* output_string = argv[2];
	const char* rbm_string = argv[3];

	IDX* input = NULL;
	IDX* output = NULL;
	RBM* rbm =  NULL;
	float dropout;

	input = IDX::Load(input_string);
	if(input == NULL)
	{
		printf("Could not open input IDX file \"%s\"\n", input_string);
		goto CLEANUP;
	}
	else if(input->GetDataFormat() != Single)
	{
		printf("Input data must have type 'Single' data format\n");
		goto CLEANUP;
	}

	FILE* f = fopen(rbm_string, "rb");
	if(f == NULL)
	{
		printf("Could not open input RBM file \"%s\"\n", rbm_string);
		goto CLEANUP;
	}
	else
	{
		std::stringstream ss;
		for(int32_t b = fgetc(f); b >= 0; b = fgetc(f))
		{
			ss << (uint8_t)b;
		}
		std::string json = ss.str();
		fclose(f);

		rbm = RBM::FromJSON(json);
		if(rbm == nullptr)
		{
			printf("Could not parse RBM json from \"%s\"\n", rbm_string);
			goto CLEANUP;
		}
	}
	

	output = IDX::Create(output_string, input->GetEndianness(), Single, rbm->hidden_count);
	if(output == NULL)
	{
		printf("Could not create output IDX file \"%s\"\n", output_string);
		goto CLEANUP;
	}

	float** weights = rbm->hidden_features;

	// now calculate hidden values for visible vector
	float* visible_buffer = new float[rbm->visible_count];
	float* hidden_buffer = new float[rbm->hidden_count];

	for(uint32_t idx = 0; idx < input->GetRowCount(); idx++)
	{
		input->ReadRow(idx, visible_buffer);
		rbm->CalcHidden(visible_buffer, hidden_buffer);
		output->AddRow(hidden_buffer);
	}

	output->Close();

	delete[] visible_buffer;
	delete[] hidden_buffer;

	result = 0;
CLEANUP:

	delete input;
	delete output;
	delete rbm;

	return result;
}