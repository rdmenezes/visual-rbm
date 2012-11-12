#include "../QuickBoltzmannNative/RBMTrainer.h"
#include "../QuickBoltzmannNative/IDX.h"
#include "../QuickBoltzmannNative/RBM.h"


#include <stdint.h>
#include <stdio.h>
#include <vector>
// idx file to train on
IDX* training_data = NULL;
// idx file to validate with (optional)
IDX* validation_data = NULL;

// training parameters
struct
{
	Model model;
	UnitType visible_type;
	uint32_t hiddden_units;
	float learning_rate;
	float momentum;
	float l1_regularization;
	float l2_regularization;
	float visible_dropout_probability;
	float hidden_dropout_probability;
	uint32_t minibatch_size;
	uint32_t epochs;
	uint32_t print_interval;
} parameters;

// imported rbm to start with (optional)
RBM* imported = NULL;
// file to save rbm to
char* export_file = NULL;
// in quiet mode, reconstruction error and free energy are not calculated
bool quiet = false;


void print_help()
{
	printf("\nUsage: clrbm [ARGS]\n");
	printf("Train an RBM using OpenGL\n\n");
	printf("Required Arguments:\n");
	printf("  -train=IDX        Specifies the input idx training data file.\n");
	printf("  -params=PARAMS    Load training parameters to use during training.\n");
	printf("  -export=OUT       Specifies filename to save trained RBM as.\n\n");
	printf(" Optional Arguments:\n");
	printf("  -valid=IDX        Specifies an optional validation data file.\n");
	printf("  -import=RBM       Specifies filename of optional RBM to import and train.\n");
	printf("  -quiet            Suppresses all stdout output\n");
	printf("  -defaults         Save a default configuration file to default.vrbmparameters\n");

}

const char* params[] = 
{
	"model=",
	"visible_type=",
	"hidden_units=",
	"learning_rate=",
	"momentum=",
	"l1_regularization=",
	"l2_regularization=",
	"visible_dropout=",
	"hidden_dropout=",
	"minibatch_size=",
	"epochs=",
	"print_interval="
};

const char* param_defaults[] =
{
	"rbm",
	"binary",
	"100",
	"0.001",
	"0.5",
	"0.0",
	"0.0",
	"0.0",
	"0.5",
	"10",
	"100",
	"100",
};

bool load_parameters(const char* filename)
{
	bool result = false;

	FILE* file = fopen(filename, "rb");
	if(file == 0)
	{
		return result;
	}

	// get length of file
	fseek(file, 0, SEEK_END);
	int32_t length = ftell(file);
	rewind(file);

	// config file parameters
	enum
	{
		Model,
		VisibleType,
		HiddenUnits,
		LearningRate,
		Momentum,
		L1Regularization,
		L2Regularization,
		VisibleDropout,
		HiddenDropout,
		MinibatchSize,
		Epochs,
		PrintInterval,
		Count
	};

	char* values[Count] = {0};

	char* data = new char[length+1];
	uint32_t index = 0;
	uint8_t prev_char = 0;

	std::vector<char*> line_list;
	char* line_front = data;

	

	// get the lines
	for(int i = 0; i < length; i++)
	{
		char val = fgetc(file);
		// trim whitespace, ignore carriage return, don't write sequential new line, don't write percent
		if((val == '\n' && prev_char != '\n') || (val != ' ' && val != '\t' && val != '\r' && val != '\n' && val != '%'))
		{
			// lowercase capital letters
			if(val >= 'A' && val <= 'Z')
				val += 'a' - 'A';

			prev_char = val;
			

			if(val == '\n')
			{
				data[index++] = 0;
				line_list.push_back(line_front);
				line_front = data + index;
			}
			else if(i == (length-1))
			{
				data[index++] = val;
				data[index++] = 0;
				line_list.push_back(line_front);
			}
			else
			{
				data[index++] = val;
			}
		}
	}
	// close the parameters file
	fclose(file);

	// find the values
	for(int k = 0; k < line_list.size(); k++)
	{
		char* line = line_list[k];
		for(int i = 0; i < Count; i++)
		{
			if(strncmp(line, params[i], strlen(params[i])) == 0)
			{
				if(values[i] == NULL)
				{
					values[i] = line + strlen(params[i]);
				}
				else
				{
					printf("Duplicate parameter found:\n  \"%s\"", line);
					goto ERROR;
				}
			}
		}
	}

	// parse the values

#pragma region Parameter Parsing

	if(values[Model] && strcmp(values[Model], "rbm") != 0)
	{
		printf("Problem parsing \"%s\" as model", values[Model]);
		goto ERROR;
	}
	else
	{
		parameters.model = Model_RBM;
	}

	if(values[VisibleType])
	{
		if(strcmp(values[VisibleType], "binary") == 0)
		{
			parameters.visible_type = Binary;
		}
		else if(strcmp(values[VisibleType], "gaussian") == 0)
		{
			parameters.visible_type = Gaussian;
		}
		else
		{
			printf("Problem parsing \"%s\" as unit type; must be either 'binary' or 'gaussian'", values[VisibleType]);
			goto ERROR;
		}
	}
	else
	{
		parameters.visible_type = Binary;
	}

	if(values[HiddenUnits])
	{
		if(sscanf(values[HiddenUnits], "%u", &parameters.hiddden_units) != 1)
		{
			printf("Problem parsing \"%s\" as hidden unit count", values[HiddenUnits]);
			goto ERROR;
		}
		else if(parameters.hiddden_units == 0)
		{
			printf("Invalid hidden unit count value \"%s;\" must be a positive integer", values[HiddenUnits]);
			goto ERROR;
		}
	}
	else
	{
		parameters.hiddden_units = 100;
	}

	// learnign rate is a positive float
	if(values[LearningRate])
	{
		if(sscanf(values[LearningRate], "%f", &parameters.learning_rate) != 1)
		{
			printf("Problem parsing \"%s\" as learning rate", values[LearningRate]);
			goto ERROR;
		}
		else if(parameters.learning_rate <= 0)
		{
			printf("Invalid learning rate value \"%s;\" must be a positive real value", values[LearningRate]);
			goto ERROR;
		}
	}
	else
	{
		parameters.learning_rate = 0.001f;
	}

	// momentum value is a float on [0,1]
	if(values[Momentum])
	{
		if(sscanf(values[Momentum], "%f", &parameters.momentum) != 1)
		{
			printf("Problem parsing \"%s\" as momentum", values[Momentum]);
			goto ERROR;
		}
		else if(parameters.momentum < 0.0f || parameters.momentum > 1.0f)
		{
			printf("Invalid momentum value \"%s;\" must be real value on [0,1]", values[Momentum]);
			goto ERROR;
		}
	}
	else
	{
		parameters.momentum = 0.5f;
	}

	// non-neggative real value
	if(values[L1Regularization])
	{
		if(sscanf(values[L1Regularization], "%f", &parameters.l1_regularization) != 1)
		{
			printf("Problem parsing \"%s\" as L1 regularization", values[L1Regularization]);
			goto ERROR;
		}
		else if(parameters.l1_regularization < 0.0f)
		{
			printf("Invalid L1 regularization value \"%s;\" must be a non-negative real value", values[L1Regularization]);
			goto ERROR;
		}
	}
	else
	{
		parameters.l1_regularization = 0.0f;
	}
	
	// non-negative real value
	if(values[L2Regularization])
	{
		if(sscanf(values[L2Regularization], "%f", &parameters.l2_regularization) != 1)
		{
			printf("Problem parsing \"%s\" as L2 regularization", values[L2Regularization]);
			goto ERROR;
		}
		else if(parameters.l2_regularization < 0.0f)
		{
			printf("Invalid L2 regularization value \"%s;\" must be a non-negative real value", values[L2Regularization]);
			goto ERROR;
		}
	}
	else
	{
		parameters.l2_regularization = 0.0f;
	}

	// real value on (0,1]
	if(values[VisibleDropout])
	{
		if(sscanf(values[VisibleDropout], "%f", &parameters.visible_dropout_probability) != 1)
		{
			printf("Problem parsing \"%s\" as visible dropout probability", values[VisibleDropout]);
			goto ERROR;
		}
		else if(parameters.visible_dropout_probability < 0.0f || parameters.visible_dropout_probability >= 1.0f)
		{
			printf("Invalid visible dropout probability value \"%s;\" must be a non-negative real value less than 1.0", values[VisibleDropout]);
			goto ERROR;
		}
	}
	else
	{
		parameters.visible_dropout_probability = 0.0f;
	}

	// real value on (0,1]
	if(values[HiddenDropout])
	{
		if(sscanf(values[HiddenDropout], "%f", &parameters.hidden_dropout_probability) != 1)
		{
			printf("Problem parsing \"%s\" as hidden dropout probability", values[HiddenDropout]);
			goto ERROR;
		}
		else if(parameters.hidden_dropout_probability < 0.0f || parameters.hidden_dropout_probability >= 1.0f)
		{
			printf("Invalid hidden dropout probability value \"%s;\" must be a non-negative real value less than 1.0", values[HiddenDropout]);
			goto ERROR;
		}
	}
	else
	{
		parameters.hidden_dropout_probability = 0.5f;
	}

	// positive integer
	if(values[MinibatchSize])
	{
		if(sscanf(values[MinibatchSize], "%u", &parameters.minibatch_size) != 1)
		{
			printf("Problem parsing \"%s\" as minibatch size", values[MinibatchSize]);
			goto ERROR;
		}
		else if(parameters.minibatch_size == 0)
		{
			printf("Minibatch size must be greater than 0");
			goto ERROR;
		}
	}
	else
	{
		parameters.minibatch_size = 10;
	}

	if(values[Epochs])
	{
		if(sscanf(values[Epochs], "%u", &parameters.epochs) != 1)
		{
			printf("Problem parsing \"%s\" as epoch count", values[Epochs]);
			goto ERROR;
		}
		else if(parameters.epochs == 0)
		{
			printf("Epoch count must be greater than 0");
			goto ERROR;
		}
	}
	else
	{
		parameters.epochs = 100;
	}

	if(values[PrintInterval])
	{
		if(sscanf(values[PrintInterval], "%u", &parameters.print_interval) != 1)
		{
			printf("Problem parsing \"%s\" as print interval", values[PrintInterval]);
			goto ERROR;
		}
		else if(parameters.print_interval == 0)
		{
			printf("Print interval must be greater than 0");
			goto ERROR;
		}
	}
	else
	{
		parameters.print_interval = 500;
	}


#pragma endregion

	result = true;
ERROR:
	delete[] data;

	return result;
}

enum HandleArgumentsResults
{
	Success,
	Error,
	CreateDefaults
};

HandleArgumentsResults handle_arguments(int argc, char** argv)
{
	enum Arguments
	{
		TrainingData,
		ValidationData,
		Parameters,
		Import,
		Export,
		Quiet,
		Defaults,
		Count
	};

	const char* flags[Count] = {"-train=", "-valid=", "-params=", "-import=", "-export=", "-quiet", "-defaults"};
	char* arguments[Count] = {0};

	for(int i = 1; i < argc; i++)
	{
		int index = -1;
		for(int j = 0; j < Count; j++)
		{
			// find argument that contains each flag
			if(strncmp(argv[i], flags[j], strlen(flags[j])) == 0)
			{
				index = j;
				break;
			}
		}

		if(index == -1)
		{
			printf("Unknown argument \"%s\" found\n", argv[i]);
			return Error;
		}
		else if(arguments[index] == NULL)
		{
			// set string 
			arguments[index] = argv[i] + strlen(flags[index]);
			
		}
		else
		{
			// duplicate flag detected
			printf("Duplicate flag \"%s\" found", flags[index]);
			return Error;
		}
	}

	if(arguments[Defaults] != 0)
	{
		return CreateDefaults;
	}

	// error handling
	if(arguments[TrainingData] == NULL)
	{
		printf("Need training data\n");
		return Error;
	}

	if(arguments[Parameters] == NULL)
	{
		printf("Need parameters\n");
		return Error;
	}

	if(arguments[Export] == NULL)
	{
		printf("Need export destination filename\n");
		return Error;
	}

	// get training data file
	training_data = IDX::Load(arguments[TrainingData]);
	if(training_data == NULL)
	{
		printf("Problem loading idx training data \"%s\"\n", arguments[TrainingData]);
		return Error;
	}

	// get optional validation data file
	if(arguments[ValidationData])
	{
		validation_data = IDX::Load(arguments[ValidationData]);
		printf("Problem loading idx validation data \"%s\"\n", arguments[ValidationData]);
	}

	// load parameters here
	if(!load_parameters(arguments[Parameters]))
	{
		printf("\nProblem parsing parameters file:\n  \"%s\"\n", arguments[Parameters]);
		return Error;
	}

	// get rbm to start from
	if(arguments[Import])
	{
		imported = RBM::Load(arguments[Import]);
		if(imported == NULL)
		{
			printf("Problem loading RBM file:\n  \"%s\"\n", arguments[Import]);
			return Error;
		}
	}

	// filename to export to
	export_file = arguments[Export];
	if(export_file == NULL)
	{
		printf("No export filename given for RBM\n");
		return Error;
	}


	// should we calculate error/free energy
	quiet = arguments[Quiet] != NULL;

	return Success;
}

RBMTrainer* trainer = NULL;

int main(int argc, char** argv)
{
	if(argc == 1)
	{
		goto ERROR;
	}

	switch(handle_arguments(argc, argv))
	{
	case Success:
		{		
			// print out the used training parameters
			printf("Training Parameters:\n");
			printf(" Model = %s\n", parameters.model == Model_RBM ? "RBM" : "SRBM");
			printf(" Visible Type = %s\n", parameters.visible_type== Binary ? "Binary" : "Gaussian");
			printf(" Hidden Units = %u\n", parameters.hiddden_units);
			printf(" Learning Rate = %f\n", parameters.learning_rate);
			printf(" Momentum = %f\n", parameters.momentum);
			printf(" L1 Regularization = %f\n", parameters.l1_regularization);
			printf(" L2 Regularization = %f\n", parameters.l2_regularization);
			printf(" Visible Dropout Probability = %f\n", parameters.visible_dropout_probability);
			printf(" Hidden Dropout Probability = %f\n", parameters.hidden_dropout_probability);
			printf(" Minibatch Size = %u\n", parameters.minibatch_size);
			printf(" Training Epochs = %u\n", parameters.epochs);
			
			fflush(stdout);
			
			trainer = new RBMTrainer();
			if(trainer->GetLastErrorCode() == RequiredOpenGLVersionUnsupported)
			{
				printf("OpenGL version 3.3 is required to run this program\n");
				goto ERROR;
			}

				trainer->SetVisibleType(parameters.visible_type);
				trainer->SetHiddenCount(parameters.hiddden_units);
				trainer->SetLearningRate(parameters.learning_rate);
				trainer->SetMomentum(parameters.momentum);
				trainer->SetL1Regularization(parameters.l1_regularization);
				trainer->SetL2Regularization(parameters.l2_regularization);
				trainer->SetVisibleDropout(parameters.visible_dropout_probability);
				trainer->SetHiddenDropout(parameters.hidden_dropout_probability);
				trainer->SetMinibatchSize(parameters.minibatch_size);
			
			// load the training data
			if(!trainer->SetTrainingData(training_data))
			{
				printf("Training Data contains invalid values");
				goto ERROR;
			}


			// load the validation data
			if(!trainer->SetValidationData(validation_data))
			{
				if(trainer->GetLastErrorCode() == DataHasIncorrectNumberOfVisibleInputs)
				{
					printf("Validation data has different number of visible inputs as training data");
				}
				else
				{
					printf("Validation Data contains invalid values");
				}
				goto ERROR;
			}

			// load rbm
			if(imported != NULL)
			{
				trainer->SetRBM(imported);
				if(trainer->GetLastErrorCode() == ImportedRBMHasIncorrectNumberOfVisibleInputs)
				{
					printf("Imported RBM has different number of visible inputs as training data");
					goto ERROR;
				}
			}

			uint64_t total_iterations = 0;
		
			if (!quiet)
			{
				if(validation_data)
				{
					printf("interval; training reconstruction; validation reconstruction\n");
				}
				else
				{
					printf("interval; training reconstruction\n");
				}
			}

			trainer->Initialize();

			while(parameters.epochs)
			{
				trainer->Train();

				total_iterations++;

				// decrement
				if((total_iterations % trainer->GetMinibatches()) == 0)
				{
					--parameters.epochs;
				}

				// print our update
				if( !quiet)
				{
					if((total_iterations % parameters.print_interval) == 0)
					{
						float train_error = trainer->GetReconstructionError();
						if(validation_data)
						{
							float valid_error = trainer->GetValidationReconstructionError();

							printf("%llu; %f; %f\n", total_iterations, train_error, valid_error);
						}
						else
						{
							printf("%llu; %f\n", total_iterations, train_error);
						}

						fflush(stdout);
					}
				}
			}
		
			printf("Exporting trained RBM to \"%s\"\n", export_file);
			fflush(stdout);
			RBM* rbm = trainer->GetRBM();
			rbm->Save(export_file);
			delete rbm;
			goto FINISHED;
		}
	case Error:
		goto ERROR;
	case CreateDefaults:
		{
			printf("Generating \"default.vrbmparameters\" file\n");
			FILE* defaults = fopen("default.vrbmparameters", "wb");

			for(int k = 0; k < sizeof(params)/sizeof(params[0]); k++)
			{
				fprintf(defaults, "%s%s\n",params[k], param_defaults[k]);
			}

			fflush(defaults);
			fclose(defaults);
			goto FINISHED;
		}
	}

ERROR:
	print_help();
	fflush(stdout);
FINISHED:
	// cleanup
	if(training_data)
	{
		delete training_data;
	}

	if(validation_data)
	{
		delete validation_data;
	}

	if(imported)
	{
		delete imported;
	}
}