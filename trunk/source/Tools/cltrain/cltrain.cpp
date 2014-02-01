#include <stdint.h>
#include <stdio.h>
#include <vector>

// OMLT
#include <IDX.hpp>
#include <DataAtlas.h>
#include <RestrictedBoltzmannMachine.h>
#include <ContrastiveDivergence.h>
#include <MultilayerPerceptron.h>
#include <BackPropagation.h>
#include <TrainingSchedule.h>
#include <Enums.h>

using namespace OMLT;

// idx file to train on
IDX* training_data = NULL;
// idx file to validate with (optional)
IDX* validation_data = NULL;

// training parameters

ModelType_t model_type = ModelType::Invalid;

// imported model to start with (optional)
static union
{
	RBM* rbm;
	MLP* mlp;
} loaded =  {0};


static union
{
	TrainingSchedule<CD>* cd;
	TrainingSchedule<BP>* bp;
	
} schedule = {0};

static union
{
	ContrastiveDivergence* cd;
	BackPropagation* bp;
} trainer = {0};

// file to save rbm to
FILE* export_file = NULL;
// in quiet mode, reconstruction error is not calculated
bool quiet = false;


void print_help()
{
	printf("\nUsage: cltrain [ARGS]\n");
	printf("Train a model using OpenGL\n\n");
	printf(" Required Arguments:\n");
	printf("  -train=IDX        Specifies the input idx training data file.\n");
	printf("  -sched=SCHEDULE   Load training schedule to use during training.\n");
	printf("  -export=OUT       Specifies filename to save trained model as.\n\n");
	printf(" Optional Arguments:\n");
	printf("  -valid=IDX        Specifies an optional validation data file.\n");
	printf("  -import=IN        Specifies filename of optional model to import and train.\n");
	printf("  -quiet            Suppresses all stdout output\n");
}

enum HandleArgumentsResults
{
	Success,
	Error,
};

HandleArgumentsResults handle_arguments(int argc, char** argv)
{
	enum Arguments
	{
		TrainingData,
		ValidationData,
		Schedule,
		Import,
		Export,
		Quiet,
		Count
	};

	const char* flags[Count] = {"-train=", "-valid=", "-sched=", "-import=", "-export=", "-quiet"};
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
			printf("Unknown argument \"%s\" found.\n", argv[i]);
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
			printf("Duplicate flag \"%s\" found.\n", flags[index]);
			return Error;
		}
	}

	// error handling
	if(arguments[TrainingData] == NULL)
	{
		printf("Need training data.\n");
		return Error;
	}

	if(arguments[Schedule] == NULL)
	{
		printf("Need training schedule.\n");
		return Error;
	}

	if(arguments[Export] == NULL)
	{
		printf("Need export destination filename.\n");
		return Error;
	}

	// get training data file
	training_data = IDX::Load(arguments[TrainingData]);
	if(training_data == NULL)
	{
		printf("Problem loading idx training data: \"%s\"\n", arguments[TrainingData]);
		return Error;
	}
	else if(training_data->GetRowLength() >= SiCKL::OpenGLRuntime::GetMaxTextureSize())
	{
		printf("Training data row length is too long; can only support up to length %u\n", (SiCKL::OpenGLRuntime::GetMaxTextureSize() - 1));
		return Error;
	}

	// get optional validation data file
	if(arguments[ValidationData])
	{
		validation_data = IDX::Load(arguments[ValidationData]);
		if(validation_data == nullptr)
		{
			printf("Problem loading idx validation data: \"%s\"\n", arguments[ValidationData]);
			return Error;
		}
		else if(validation_data->GetRowLength() != training_data->GetRowLength())
		{
			printf("Mismatch between training data row length and validation data row length\n");
			return Error;
		}
	}

	// load parameters here (scope schedule text so it goes away when we're done with it)
	{
		std::string schedule_json;
		if(!OMLT::ReadTextFile(arguments[Schedule], schedule_json))
		{
			printf("Problem loading training schedule: \"%s\"\n", arguments[Schedule]);
			return Error;
		}

		if(schedule.cd = TrainingSchedule<CD>::FromJSON(schedule_json))
		{
			if(schedule.cd->GetMinibatchSize() > SiCKL::OpenGLRuntime::GetMaxTextureSize())
			{
				printf("Minibatch size greater than %u is not supported.\n", SiCKL::OpenGLRuntime::GetMaxTextureSize());
				return Error;
			}
			else
			{
				model_type = ModelType::RBM;
			}
		}
		else if(schedule.bp = TrainingSchedule<BP>::FromJSON(schedule_json))
		{
			if(schedule.bp->GetMinibatchSize() > SiCKL::OpenGLRuntime::GetMaxTextureSize())
			{
				printf("Minibatch size greater than %u is not supported.\n", SiCKL::OpenGLRuntime::GetMaxTextureSize());
				return Error;
			}
			else
			{
				model_type = ModelType::MLP;
			}
		}
		else
		{
			printf("Problem parsing training schedule: \"%s\"\n", arguments[Schedule]);
			return Error;
		}
	}


	// get rbm to start from
	if(arguments[Import])
	{
		std::string model_json;
		if(!OMLT::ReadTextFile(arguments[Import], model_json))
		{
			printf("Problem loading model JSON from: \"%s\"\n", arguments[Import]);
			return Error;
		}
		else
		{
			OMLT::Model model;
			if(!FromJSON(model_json, model))
			{
				printf("Problem parsing model JSON from: \"%s\"\n", arguments[Import]);
				return Error;
			}
			else if(model.type != model_type)
			{
				printf("Loaded model from \"%s\" does not match model found in training schedule \"%s\"\n", arguments[Import], arguments[Schedule]);
				return Error;
			}
			switch(model_type)
			{
			case ModelType::RBM:
				loaded.rbm = model.rbm;
				break;
			case ModelType::MLP:
				loaded.mlp = model.mlp;
				break;
			}
		}
	}

	// filename to export to
	if(arguments[Export] == NULL)
	{
		printf("No export filename given for trained model.\n");
		return Error;
	}

	export_file = fopen(arguments[Export], "wb");
	if(export_file == nullptr)
	{
		printf("Could not open \"%s\" for writing.\n", arguments[Export]);
		return Error;
	}


	// should we calculate error/free energy
	quiet = arguments[Quiet] != NULL;

	return Success;
}

DataAtlas* training_atlas = nullptr;
DataAtlas* validation_atlas = nullptr;

template<typename TRAINER>
TRAINER* GetTrainer() { return nullptr;}

template<typename MODEL, typename TRAINER>
bool Run(MODEL* in_model, TrainingSchedule<TRAINER>* in_schedule)
{
	// load and initialize data
	training_atlas = new DataAtlas(training_data);
	training_atlas->Initialize(in_schedule->GetMinibatchSize(), 512);
	if(validation_data)
	{
		validation_atlas = new DataAtlas(validation_data);
		validation_atlas->Initialize(in_schedule->GetMinibatchSize(), 512);
	}

	in_schedule->StartTraining();
	Initialize<MODEL>();
	

	uint32_t iterations = 0;
	uint32_t epoch = 0;
	float train_error = 0.0f;
	float validation_error = 0.0f;
	SiCKL::OpenGLBuffer2D train_example;
	SiCKL::OpenGLBuffer2D validation_example;

	const uint32_t total_batches = training_atlas->GetTotalBatches();

	while(in_schedule->TrainingComplete() == false)
	{
		training_atlas->Next(train_example);

		Train<TRAINER>(train_example);

		if(!quiet)
		{
			train_error += GetError<TRAINER>();
			if(validation_atlas)
			{
				validation_atlas->Next(validation_example);
				validation_error += Validate<TRAINER>(validation_example);
			}
		}

		iterations = (iterations + 1) % total_batches;
		if(iterations == 0)
		{
			epoch++;
			train_error /= total_batches;
			validation_error /= total_batches;

			if(!quiet)
			{
				if(validation_atlas)
				{
					printf("%u;%.8f;%.8f\n", epoch, train_error, validation_error);
				}
				else
				{
					printf("%u;%.8f\n", epoch, train_error);
				}
			}
			fflush(stdout);

			// reset error
			train_error = validation_error = 0.0f;

			if(in_schedule->NextEpoch())
			{
				epoch = 0;
				if(in_schedule->TrainingComplete())
				{
					// get model JSOn and write to disk
					std::string model_json = ToJSON<TRAINER>();
					fwrite(model_json.c_str(), model_json.size(), sizeof(uint8_t), export_file);
					fflush(export_file);
					fclose(export_file);
					
					return true;
				}
				else
				{
					TRAINER::TrainingConfig train_config;
					bool populated = in_schedule->GetTrainingConfig(train_config);
					assert(populated);

					GetTrainer<TRAINER>()->SetTrainingConfig(train_config);
				}
			}
		}
	}

	return true;
}


template<typename MODEL>
bool Initialize() { return false; }

template <typename TRAINER>
void Train(const OpenGLBuffer2D& train_example) { }

template <typename TRAINER>
float GetError() {return 0.0f;}

template <typename TRAINER>
float Validate(const OpenGLBuffer2D& validation_example) { return 0.0f; }

template <typename TRAINER>
std::string ToJSON() {return "";}

#pragma region Contrastive Divergencce

template<>
CD* GetTrainer() {return trainer.cd;}

template<>
bool Initialize<RBM>()
{
	const auto model_config = schedule.cd->GetModelConfig();
	if(loaded.rbm)
	{
		// verify model config matches
		if(loaded.rbm->visible_count != model_config.VisibleUnits ||
		   loaded.rbm->visible_type != model_config.VisibleType ||
		   loaded.rbm->hidden_count != model_config.HiddenUnits ||
		   loaded.rbm->hidden_type != model_config.HiddenType)
		{
			printf("Model parameters in schedule do not match those found in loaded RBM\n");
			return false;
		}
		else if(loaded.rbm->hidden_count >= SiCKL::OpenGLRuntime::GetMaxTextureSize())
		{
			printf("Hidden unit count greater than %u is not supported.\n", (SiCKL::OpenGLRuntime::GetMaxTextureSize() - 1));
			return false;
		}

		trainer.cd = new ContrastiveDivergence(loaded.rbm, schedule.cd->GetMinibatchSize());
	}
	else
	{
		trainer.cd = new ContrastiveDivergence(model_config, schedule.cd->GetMinibatchSize());
	}
	
	CD::TrainingConfig train_config;
	bool populated = schedule.cd->GetTrainingConfig(train_config);
	assert(populated == true);
	trainer.cd->SetTrainingConfig(train_config);

	return true;
}

template<>
void Train<CD>(const OpenGLBuffer2D& train_example)
{
	trainer.cd->Train(train_example);
}

template<>
float GetError<CD>()
{
	return trainer.cd->GetLastReconstructionError();
}

template <>
float Validate<CD>(const OpenGLBuffer2D& validation_example)
{
	return trainer.cd->GetReconstructionError(validation_example);
}

template <>
std::string ToJSON<CD>()
{
	RBM* rbm = trainer.cd->GetRestrictedBoltzmannMachine();
	std::string json = rbm->ToJSON();
	delete rbm;

	return json;
}

#pragma endregion

#pragma region Back Propagation

template<>
BP* GetTrainer() {return trainer.bp;}


template<>
bool Initialize<MLP>()
{
	const auto model_config = schedule.bp->GetModelConfig();
	if(loaded.mlp)
	{
		// verify model config matches
		auto input_layer = loaded.mlp->InputLayer();
		auto output_layer = loaded.mlp->OutputLayer();
		if(input_layer->inputs != model_config.InputCount ||
		   input_layer->outputs != model_config.LayerConfigs[0].OutputUnits ||
		   input_layer->function != model_config.LayerConfigs[0].Function ||
		   output_layer->function != model_config.LayerConfigs[1].Function)
		{
			printf("Model parameters in schedule do not match those found in loaded MLP\n");
			return false;
		}
		else if(input_layer->outputs >= SiCKL::OpenGLRuntime::GetMaxTextureSize())
		{
			printf("Hidden unit count greater than %u is not supported.\n", (SiCKL::OpenGLRuntime::GetMaxTextureSize() - 1));
			return false;
		}
		
		trainer.bp = new BackPropagation(loaded.mlp, schedule.bp->GetMinibatchSize());
	}
	else
	{
		trainer.bp = new BackPropagation(model_config, schedule.bp->GetMinibatchSize());
	}

	BP::TrainingConfig train_config;
	bool populated = schedule.bp->GetTrainingConfig(train_config);
	assert(populated == true);
	trainer.bp->SetTrainingConfig(train_config);

	return true;
}

template<>
void Train<BP>(const OpenGLBuffer2D& train_example)
{
	trainer.bp->Train(train_example, train_example);
}

template<>
float GetError<BP>()
{
	return trainer.bp->GetLastOutputError();
}

template <>
float Validate<BP>(const OpenGLBuffer2D& validation_example)
{
	return trainer.bp->GetOutputError(validation_example, validation_example);
}

template <>
std::string ToJSON<BP>()
{
	MLP* mlp = trainer.bp->GetMultilayerPerceptron();
	std::string json = mlp->ToJSON();
	delete mlp;

	return json;
}

#pragma endregion

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
			fflush(stdout);

			if(!SiCKL::OpenGLRuntime::Initialize())
			{
				printf("Could not initialize OpenGL runtime; support for OpenGL 3.3 required\n");
				goto ERROR;
			}

			bool success = false;
			switch(model_type)
			{
			case ModelType::RBM:
				success = Run<RBM, CD>(loaded.rbm, schedule.cd);
				break;
			case ModelType::MLP:
				success = Run<MLP, BP>(loaded.mlp, schedule.bp);
				break;
			}

			if(success == false )
			{
				goto ERROR;
			}

			goto FINISHED;
		}
	case Error:
		goto ERROR;
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
}