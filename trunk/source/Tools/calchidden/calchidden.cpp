#include "IDX.hpp"
#include "RBM.hpp"

#include <string.h>
#include <math.h>

const char* Usage = 
	"Calculates the hidden activations or the hidden probabilities for an\n"
	"input IDX dataset and trained RBM.\n"
	"\n"
	"Usage: calchidden [MODE] [INPUT] [OUTPUT] [RBM] [DROPOUT]\n"
	"\n"
	"  MODE      Either 'A' or 'P'\n"
	"   A        Calculates the hidden activations\n"
	"   P        Calculates the hidden probabilities\n"
	"  INPUT     The input IDX dataset used as input\n"
	"  OUTPUT    Destination IDX to save hidden values\n"
	"  RBM       A trained RBM file\n"
	"  DROPOUT   The 'visible dropout' probability used when training the\n"
	"            input RBM model\n";

int main(int argc, char** argv)
{
	int result = -1;

	if(argc != 6)
	{
		printf(Usage);
		return result;
	}

	const char* mode_string = argv[1];
	const char* input_string = argv[2];
	const char* output_string = argv[3];
	const char* rbm_string = argv[4];
	const char* dropout_string = argv[5];

	enum
	{
		Activations, Probabilities
	} mode;
	IDX* input = NULL;
	IDX* output = NULL;
	RBM* rbm =  NULL;
	float dropout;

	if(strcmp(mode_string, "A") == 0)
	{
		mode = Activations;
	}
	else if(strcmp(mode_string, "P") == 0)
	{
		mode = Probabilities;
	}
	else
	{
		printf("Could not parse given MODE \"%s\", must be either A or P\n", mode_string);
		goto CLEANUP;
	}

	if(sscanf(dropout_string, "%f", &dropout) != 1 || dropout < 0.0f || dropout > 1.0f)
	{
		printf("Could not parse given DROPOUT \"%s\", must be a value between 0 and 1 inclusive");
		goto CLEANUP;
	}

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

	rbm = RBM::Load(rbm_string);
	if(rbm == NULL)
	{
		printf("Could not open input RBM file \"%s\"\n", rbm_string);
		goto CLEANUP;
	}

	output = IDX::Create(output_string, input->GetEndianness(), Single, rbm->_hidden_count);
	if(output == NULL)
	{
		printf("Could not create output IDX file \"%s\"\n", output_string);
		goto CLEANUP;
	}

	// copy weights into more cache friendly layout
	float** weights = new float*[rbm->_hidden_count];
	for(uint32_t j = 0; j < rbm->_hidden_count; j++)
	{
		weights[j] = new float[rbm->_visible_count];
		for(uint32_t i = 0; i < rbm->_visible_count; i++)
		{
			weights[j][i] = rbm->GetWeight(i,j);
		}
	}

	// now calculate hidden values for visible vector
	float* visible_buffer = new float[rbm->_visible_count];
	float* hidden_buffer = new float[rbm->_hidden_count];
	for(uint32_t idx = 0; idx < input->GetRowCount(); idx++)
	{
		// get the input vectot
		input->ReadRow(idx, visible_buffer);
		// calculate each state
		for(uint32_t j = 0; j < rbm->_hidden_count; j++)
		{
			hidden_buffer[j] = 0.0f;
			for(uint32_t i = 0; i < rbm->_visible_count; i++)
			{
				hidden_buffer[j] += weights[j][i] * visible_buffer[i];
			}
			hidden_buffer[j] *= (1.0f - dropout);
			hidden_buffer[j] += rbm->_hidden_biases[j];
		}

		if(mode == Probabilities)
		{
			for(uint32_t j = 0; j < rbm->_hidden_count; j++)
			{
				float& x = hidden_buffer[j];
				hidden_buffer[j] = (float)(1.0f / (1.0 + exp(-x)));
			}
		}

		output->AddRow(hidden_buffer);
	}

	output->Close();

	delete[] visible_buffer;
	delete[] hidden_buffer;

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

	if(rbm != NULL)
	{
		delete rbm;
	}

	return result;
}