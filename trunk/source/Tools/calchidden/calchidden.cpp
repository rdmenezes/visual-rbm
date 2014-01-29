#include <Common.h>
#include <IDX.hpp>
#include <RestrictedBoltzmannMachine.h>
#include <MultilayerPerceptron.h>
using namespace OMLT;

#include <string.h>
#include <math.h>
#include <sstream>

const char* Usage = 
	"Calculates the hidden activations for an input IDX dataset and trained\n"
	"model.\n"
	"\n"
	"Usage: calchidden [INPUT] [OUTPUT] [MODEL]\n"
	"\n"
	"  INPUT     The input IDX dataset used as input\n"
	"  OUTPUT    Destination IDX to save hidden values\n"
	"  MODEL     A trained model json file\n";



int main(int argc, char** argv)
{
	int result = -1;

	if(argc != 4)
	{
		printf(Usage);
		return result;
	}

	const char* input_string = argv[1];
	const char* output_string = argv[2];
	const char* rbm_string = argv[3];

	IDX* input = NULL;
	IDX* output = NULL;
	std::string model_json;
	union
	{
		RBM* rbm;
		MLP* mlp;
	} model = {0};
	ModelType_t model_type = ModelType::Invalid;
	uint32_t input_count = 0;
	uint32_t output_count = 0;

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
	input_count = input->GetRowLength();


	if(OMLT::ReadTextFile(rbm_string, model_json))
	{
		if(model.rbm = RBM::FromJSON(model_json))
		{
			if(model.rbm->visible_count == input_count)
			{
				model_type = ModelType::RBM;
				output_count = model.rbm->hidden_count;
			}
			else
			{
				printf("Loaded RBM's visible unit count is %u, while input data requires %u\n", model.rbm->visible_count, input->GetRowLength());
				goto CLEANUP;
			}
		}
		else if(model.mlp = MLP::FromJSON(model_json))
		{
			if(model.mlp->InputLayer()->inputs == input_count)
			{
				model_type = ModelType::MLP;
				output_count = model.mlp->GetLayer(0)->outputs;
			}
			else
			{
				printf("Loaded MLP's visible unit count is %u, while input data requires %u\n", model.mlp->InputLayer()->inputs, input->GetRowLength());
				goto CLEANUP;
			}
		}
		else
		{
			printf("Could not parse model json from \"%s\"\n", rbm_string);
			goto CLEANUP;
		}
	}
	else
	{
		printf("Could not open input model file \"%s\"\n", rbm_string);
		goto CLEANUP;
	}
	

	output = IDX::Create(output_string, input->GetEndianness(), Single, output_count);

	if(output == NULL)
	{
		printf("Could not create output IDX file \"%s\"\n", output_string);
		goto CLEANUP;
	}

	// now calculate hidden values for visible vector
	float* visible_buffer = new float[input_count];
	float* hidden_buffer = new float[output_count];

	for(uint32_t idx = 0; idx < input->GetRowCount(); idx++)
	{
		input->ReadRow(idx, visible_buffer);
		switch(model_type)
		{
		case ModelType::RBM:
			model.rbm->CalcHidden(visible_buffer, hidden_buffer);
			break;
		case ModelType::MLP:
			model.mlp->FeedForward(visible_buffer, hidden_buffer, 0);
			break;
		}
		
		output->AddRow(hidden_buffer);
	}

	output->Close();

	delete[] visible_buffer;
	delete[] hidden_buffer;

	result = 0;
CLEANUP:

	delete input;
	delete output;
	switch(model_type)
	{
	case ModelType::RBM:
		delete model.rbm;
		break;
	case ModelType::MLP:
		delete model.mlp;
		break;
	}

	return result;
}