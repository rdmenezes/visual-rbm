#include <stdint.h>
#include <stdio.h>
#include <vector>

// OMLT
#include <IDX.hpp>
#include <DataAtlas.h>
#include <RestrictedBoltzmannMachine.h>
#include <ContrastiveDivergence.h>
#include <AutoEncoder.h>
#include <AutoEncoderBackPropagation.h>
#include <MultilayerPerceptron.h>
#include <BackPropagation.h>
#include <TrainingSchedule.h>
#include <Enums.h>

using namespace OMLT;

// idx file to train on
IDX* training_data = nullptr;
// idx file containing labels
IDX* training_labels = nullptr;

// idx file containing validation data (optional)
IDX* validation_data = nullptr;
// ifx file containing validation labels (optional)
IDX* validation_labels = nullptr;

// training parameters

ModelType_t model_type = ModelType::Invalid;

// imported model to start with (optional)
static union
{
	RBM* rbm;
	AutoEncoder* ae;
	MLP* mlp;
} loaded =  {0};


static union
{
	TrainingSchedule<CD>* cd;
	TrainingSchedule<AutoEncoderBackPropagation>* aebp;
	TrainingSchedule<BackPropagation>* bp;
} schedule = {0};

static union
{
	ContrastiveDivergence* cd;
	AutoEncoderBackPropagation* aebp;
	BackPropagation* bp;
} trainer = {0};

// file to save rbm to
FILE* export_file = nullptr;
// in quiet mode, reconstruction error is not calculated
bool quiet = false;

// amount of gpu memory used to allocate our data atlas
size_t atlasSize = 0;

void print_help()
{
	printf("\nUsage: cltrain [ARGS]\n");
	printf("Train a model using OpenGL\n\n");
	printf(" Required Arguments:\n");
	printf("  -trainingData=IDX       Specifies the training data file.\n");
	printf("  -trainingLabels=IDX     Specifies the training label file (for MLPs only).\n");
	printf("  -schedule=SCHEDULE      Load training schedule to use during training.\n");
	printf("  -export=OUT             Specifies filename to save trained model as.\n\n");
	printf(" Optional Arguments:\n");
	printf("  -validationData=IDX     Specifies an optional validation data file.\n");
	printf("  -validationLabels=IDX   Specifies an optional validation label file (for MLPs only)\n");
	printf("  -import=IN              Specifies filename of optional model to import and train.\n");
	printf("  -quiet                  Suppresses all stdout output.\n");
	printf("  -atlasSize=SIZE         Specifies the total memory allocated for our data atlas in\n");
	printf("                          megabytes.  Default value is 512.");
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
		TrainingLabels,
		ValidationData,
		ValidationLabels,
		Schedule,
		Import,
		Export,
		Quiet,
		AtlasSize,
		Count
	};

	const char* flags[Count] = {"-trainingData=", "-trainingLabels=", "-validationData=", "-validationLabels=", "-schedule=", "-import=", "-export=", "-quiet", "-atlasSize="};
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
		else if(arguments[index] == nullptr)
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

	// load our training schedule
	if(arguments[Schedule] == nullptr)
	{
		printf("Need training schedule.\n");
		return Error;
	}
	else
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
		else if(schedule.aebp = TrainingSchedule<AutoEncoderBackPropagation>::FromJSON(schedule_json))
		{
			if(schedule.aebp->GetMinibatchSize() > SiCKL::OpenGLRuntime::GetMaxTextureSize())
			{
				printf("Minibatch size greater than %u is not supported.\n", SiCKL::OpenGLRuntime::GetMaxTextureSize());
				return Error;
			}
			else
			{
				model_type = ModelType::AutoEncoder;
			}
		}
		else if(schedule.bp = TrainingSchedule<BackPropagation>::FromJSON(schedule_json))
		{
			if(schedule.bp->GetMinibatchSize() > SiCKL::OpenGLRuntime::GetMaxTextureSize())
			{
				printf("Minibatch size greater than %u is not supported.\n", SiCKL::OpenGLRuntime::GetMaxTextureSize());
				return Error;
			}
			else
			{
				model_type = ModelType::MultilayerPerceptron;
			}			
		}
		else
		{
			printf("Problem parsing training schedule: \"%s\"\n", arguments[Schedule]);
			return Error;
		}
	}

	// error handling
	if(arguments[TrainingData] == nullptr)
	{
		printf("Need training data.\n");
		return Error;
	}

	if(model_type == ModelType::MultilayerPerceptron && arguments[TrainingLabels] == nullptr)
	{
		printf("Need training labels.\n");
		return Error;
	}
	else if(model_type != ModelType::MultilayerPerceptron && arguments[TrainingLabels] != nullptr)
	{
		printf("Label data only needed for training Multilayer Perceptron.\n");
		return Error;
	}

	if(arguments[Export] == nullptr)
	{
		printf("Need export destination filename.\n");
		return Error;
	}

	// get training data file
	training_data = IDX::Load(arguments[TrainingData]);
	if(training_data == nullptr)
	{
		printf("Problem loading idx training data: \"%s\"\n", arguments[TrainingData]);
		return Error;
	}
	else if(training_data->GetRowLength() >= SiCKL::OpenGLRuntime::GetMaxTextureSize())
	{
		printf("Training data row length is too long; can only support up to length %u\n", (SiCKL::OpenGLRuntime::GetMaxTextureSize() - 1));
		return Error;
	}

	// if we're training an MLP load the labels
	if(model_type == ModelType::MultilayerPerceptron)
	{
		training_labels = IDX::Load(arguments[TrainingLabels]);
		if(training_labels == nullptr)
		{
			printf("Problem loading idx training labels: \"%s\"\n", arguments[TrainingLabels]);
			return Error;
		}
		else if(training_labels->GetRowLength() >= SiCKL::OpenGLRuntime::GetMaxTextureSize())
		{
			printf("Training label row length is too long; can only support up to length %u\n", (SiCKL::OpenGLRuntime::GetMaxTextureSize() - 1));
			return Error;
		}
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

		// load validation labels
		if(model_type == ModelType::MultilayerPerceptron)
		{
			if(arguments[ValidationLabels])
			{
				validation_labels = IDX::Load(arguments[ValidationLabels]);
				if(validation_labels == nullptr)
				{
					printf("Problem loading idx validation labels: \"%s\"\n", arguments[ValidationLabels]);
					return Error;
				}
				else if(validation_labels->GetRowLength() != training_labels->GetRowLength())
				{
					printf("Mismatch between training label row length and validation label row length\n");
					return Error;
				}
			}
		}
	}
	else if(arguments[ValidationLabels])
	{
		printf("Validation labels was specified but Validation data was not.\n");
		return Error;
	}
	

	// get model to start from
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
			if(!Model::FromJSON(model_json, model))
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
			case ModelType::AE:
				loaded.ae = model.ae;
				break;
			case ModelType::MLP:
				loaded.mlp = model.mlp;
				break;
			default:
				printf("Could not parse \"%s\"\n", arguments[Import]);
				return Error;
			}
		}
	}

	// filename to export to
	if(arguments[Export] == nullptr)
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
	quiet = arguments[Quiet] != nullptr;

	// figure out what our atlas size should be
	if(arguments[AtlasSize] == nullptr)
	{
		atlasSize = 512;
	}
	else if(sscanf(arguments[AtlasSize], "%u", &atlasSize) != 1 || atlasSize < 128)
	{
		printf("Could not parse \"%s\" as a valid size, must be at least 128 megabytes\n", arguments[AtlasSize]);
		return Error;
	}


	return Success;
}

// all sizes are in megabytes
void GetOptimalParitioning(uint32_t total_atlas_size, uint32_t a_size, uint32_t b_size, uint32_t& out_a_atlas_size, uint32_t& out_b_atlas_size)
{
	float f_a_size = a_size / float(total_atlas_size);
	float f_b_size = b_size / float(total_atlas_size);

	// trival case, both fit in memory just fine
	if((a_size + b_size) < total_atlas_size)
	{
		out_a_atlas_size = a_size;
		out_b_atlas_size = b_size;
	}
	// if we're both more than 3/4 the size of our total size, split the memory 50:50
	else if(f_a_size > 0.75f && f_b_size > 0.75f)
	{
		out_a_atlas_size = out_b_atlas_size = 256;
	}
	// with one large and one small, stick small one entirely into memory
	else if(f_a_size > 0.75f && f_b_size <= 0.75f)
	{
		out_b_atlas_size = b_size;
		out_a_atlas_size = total_atlas_size - out_b_atlas_size;
	}
	// same as before but swapped
	else if(f_a_size <= 0.75f && f_b_size > 0.75f)
	{
		out_a_atlas_size = a_size;
		out_b_atlas_size = total_atlas_size - out_a_atlas_size;
	}
	// finally, if both are smaller than the 3/4 total memory, put the smaller
	// one entirely in memory and give the other one the rest
	else
	{
		if(f_a_size > f_b_size)
		{
			out_b_atlas_size = b_size;
			out_a_atlas_size = total_atlas_size - out_b_atlas_size;
		}
		else
		{
			out_a_atlas_size = a_size;
			out_b_atlas_size = total_atlas_size - out_a_atlas_size;
		}
	}
}

DataAtlas* training_data_atlas = nullptr;
DataAtlas* training_label_atlas = nullptr;
DataAtlas* validation_data_atlas = nullptr;
DataAtlas* validation_label_atlas = nullptr;

SiCKL::OpenGLBuffer2D train_example;
SiCKL::OpenGLBuffer2D train_label;
SiCKL::OpenGLBuffer2D validation_example;
SiCKL::OpenGLBuffer2D validation_label;

template<typename TRAINER>
TRAINER* GetTrainer() { return nullptr;}

template<typename TRAINER>
void InitDataAtlas(uint32_t minibatch_size)
{
	if(validation_data)
	{
		uint32_t training_atlas_size, validation_atlas_size;
		GetOptimalParitioning(atlasSize, training_data->GetDatasetSize(), validation_data->GetDatasetSize(), training_atlas_size, validation_atlas_size);

		// load and initialize data
		training_data_atlas = new DataAtlas(training_atlas_size);
		training_data_atlas->Initialize(training_data, minibatch_size);

		validation_data_atlas = new DataAtlas(validation_atlas_size);
		validation_data_atlas->Initialize(validation_data, minibatch_size);
	}
	else
	{
		uint32_t training_atlas_size = training_data->GetDatasetSize() > atlasSize ? atlasSize : training_data->GetDatasetSize();
		training_data_atlas = new DataAtlas(training_atlas_size);
		training_data_atlas->Initialize(training_data, minibatch_size);
	}
}

template<typename TRAINER>
void NextExample()
{
	training_data_atlas->Next(train_example);
	if(!quiet && validation_data_atlas)
	{
		validation_data_atlas->Next(validation_example);
	}
}

template<typename MODEL, typename TRAINER>
bool Run(MODEL* in_model, TrainingSchedule<TRAINER>* in_schedule)
{
	InitDataAtlas<TRAINER>(in_schedule->GetMinibatchSize());
	in_schedule->StartTraining();
	if(Initialize<TRAINER>() == false)
	{
		return false;
	}
	
	uint32_t iterations = 0;
	uint32_t epoch = 0;
	float train_error = 0.0f;
	float validation_error = 0.0f;

	const uint32_t total_batches = training_data_atlas->GetTotalBatches();

	
	if(!quiet)
	{
		if(validation_data_atlas)
		{
			printf("epoch;training error;validation error\n");
		}
		else
		{
			printf("epoch;training error\n");
		}
		fflush(stdout);
	}

	uint32_t epoch_count = 0;
	while(in_schedule->TrainingComplete() == false)
	{
		NextExample<TRAINER>();

		Train<TRAINER>();

		if(!quiet)
		{
			train_error += GetError<TRAINER>();
			if(validation_data_atlas)
			{
				validation_error += Validate<TRAINER>();
			}
		}

		iterations = (iterations + 1) % total_batches;
		if(iterations == 0)
		{
			epoch++;
			epoch_count++;
			train_error /= total_batches;
			validation_error /= total_batches;

			if(!quiet)
			{
				if(validation_data_atlas)
				{
					printf("%u;%.8f;%.8f\n", epoch_count, train_error, validation_error);
				}
				else
				{
					printf("%u;%.8f\n", epoch_count, train_error);
				}
				fflush(stdout);
			}

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
void Train() { }

template <typename TRAINER>
float GetError() {return 0.0f;}

template <typename TRAINER>
float Validate() { return 0.0f; }

template <typename TRAINER>
std::string ToJSON() {return "";}

#pragma region Contrastive Divergencce

template<>
CD* GetTrainer() {return trainer.cd;}

template<>
bool Initialize<CD>()
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

		trainer.cd = new ContrastiveDivergence(loaded.rbm, schedule.cd->GetMinibatchSize(), schedule.cd->GetSeed());
	}
	else
	{
		trainer.cd = new ContrastiveDivergence(model_config, schedule.cd->GetMinibatchSize(), schedule.cd->GetSeed());
	}
	
	CD::TrainingConfig train_config;
	bool populated = schedule.cd->GetTrainingConfig(train_config);
	assert(populated == true);
	trainer.cd->SetTrainingConfig(train_config);

	return true;
}

template<>
void Train<CD>()
{
	trainer.cd->Train(train_example);
}

template<>
float GetError<CD>()
{
	return trainer.cd->GetLastReconstructionError();
}

template <>
float Validate<CD>()
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

#pragma region AutoEncoder Back Propagation

template<>
AutoEncoderBackPropagation* GetTrainer() {return trainer.aebp;}


template<>
bool Initialize<AutoEncoderBackPropagation>()
{
	const auto model_config = schedule.aebp->GetModelConfig();
	if(loaded.ae)
	{
		// verify model config matches
		if(loaded.ae->visible_count != model_config.VisibleCount ||
		   loaded.ae->hidden_count != model_config.HiddenCount ||
		   loaded.ae->hidden_type != model_config.HiddenType ||
		   loaded.ae->output_type != model_config.OutputType)
		{
			printf("Model parameters in schedule do not match those found in loaded AutoEncoder\n");
			return false;
		}
		else if(loaded.ae->hidden_count >= SiCKL::OpenGLRuntime::GetMaxTextureSize())
		{
			printf("Hidden unit count greater than %u is not supported.\n", (SiCKL::OpenGLRuntime::GetMaxTextureSize() - 1));
			return false;
		}
		
		trainer.aebp = new AutoEncoderBackPropagation(loaded.ae, schedule.aebp->GetMinibatchSize(), schedule.aebp->GetSeed());
	}
	else
	{
		trainer.aebp = new AutoEncoderBackPropagation(model_config, schedule.aebp->GetMinibatchSize(), schedule.aebp->GetSeed());
	}

	AutoEncoderBackPropagation::TrainingConfig train_config;
	bool populated = schedule.aebp->GetTrainingConfig(train_config);
	assert(populated == true);
	trainer.aebp->SetTrainingConfig(train_config);

	return true;
}

template<>
void Train<AutoEncoderBackPropagation>()
{
	trainer.aebp->Train(train_example);
}

template<>
float GetError<AutoEncoderBackPropagation>()
{
	return trainer.aebp->GetLastError();
}

template <>
float Validate<AutoEncoderBackPropagation>()
{
	return trainer.aebp->GetError(validation_example);
}

template <>
std::string ToJSON<AutoEncoderBackPropagation>()
{
	AutoEncoder* ae = trainer.aebp->GetAutoEncoder();
	std::string json = ae->ToJSON();
	delete ae;

	return json;
}

#pragma endregion

#pragma region Back Propagation

template<>
BP* GetTrainer() {return trainer.bp;}

template<>
void InitDataAtlas<BP>(uint32_t minibatch_size)
{
	if(validation_data)
	{
		// load and initialize data
		uint32_t training_data_atlas_size, training_label_atlas_size;
		GetOptimalParitioning(atlasSize / 2, training_data->GetDatasetSize(), training_labels->GetDatasetSize(), training_data_atlas_size, training_label_atlas_size);

		training_data_atlas = new DataAtlas(training_data_atlas_size);
		training_data_atlas->Initialize(training_data, minibatch_size);
		training_label_atlas = new DataAtlas(training_label_atlas_size);
		training_label_atlas->Initialize(training_labels, minibatch_size);

		uint32_t validation_data_atlas_size, validation_label_atlas_size;
		GetOptimalParitioning(atlasSize / 2, validation_data->GetDatasetSize(), validation_labels->GetDatasetSize(), validation_data_atlas_size, validation_label_atlas_size);

		validation_data_atlas = new DataAtlas(validation_data_atlas_size);
		validation_data_atlas->Initialize(validation_data, minibatch_size);
		validation_label_atlas = new DataAtlas(validation_label_atlas_size);
		validation_label_atlas->Initialize(validation_labels, minibatch_size);
	}
	else
	{
		// load and initialize data
		uint32_t training_data_atlas_size, training_label_atlas_size;
		GetOptimalParitioning(atlasSize, training_data->GetDatasetSize(), training_labels->GetDatasetSize(), training_data_atlas_size, training_label_atlas_size);

		training_data_atlas = new DataAtlas(training_data_atlas_size);
		training_data_atlas->Initialize(training_data, minibatch_size);
		training_label_atlas = new DataAtlas(training_label_atlas_size);
		training_label_atlas->Initialize(training_labels, minibatch_size);
	}
}

template<>
void NextExample<BP>()
{
	training_data_atlas->Next(train_example);
	training_label_atlas->Next(train_label);
	if(!quiet && validation_data_atlas)
	{
		validation_data_atlas->Next(validation_example);
		validation_label_atlas->Next(validation_label);
	}
}

template<>
bool Initialize<BP>()
{
	const auto model_config = schedule.bp->GetModelConfig();
	if(loaded.mlp)
	{
		bool matches = true;
		// verify same number of layers
		if(model_config.LayerConfigs.size() != loaded.mlp->LayerCount() || model_config.InputCount != loaded.mlp->InputLayer()->inputs)
		{
			matches = false;
		}
		else
		{
			// make sure each layer's dimensions and function matches
			for(uint32_t k = 0; k < loaded.mlp->LayerCount(); k++)
			{
				auto sconfig = model_config.LayerConfigs[k];
				auto lconfig = loaded.mlp->GetLayer(k);

				if(sconfig.OutputUnits != lconfig->outputs || sconfig.Function != lconfig->function)
				{
					matches = false;
					break;
				}
			}
		}

		if(matches)
		{
			trainer.bp = new BackPropagation(loaded.mlp, schedule.bp->GetMinibatchSize(), schedule.bp->GetSeed());
		}
		else
		{
			printf("Loaded MLP and training schedule specify different model configurations.\n");
			return false;
		}

	}
	else
	{
		trainer.bp = new BackPropagation(model_config, schedule.bp->GetMinibatchSize(), schedule.bp->GetSeed());
	}

	BP::TrainingConfig train_config;
	bool populated = schedule.bp->GetTrainingConfig(train_config);
	assert(populated == true);
	trainer.bp->SetTrainingConfig(train_config);

	return true;
}

template<>
void Train<BP>()
{
	trainer.bp->Train(train_example, train_label);
}

template<>
float GetError<BP>()
{
	return trainer.bp->GetLastOutputError();
}

template <>
float Validate<BP>()
{
	return trainer.bp->GetOutputError(validation_example, validation_label);
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
			case ModelType::AutoEncoder:
				success = Run<AutoEncoder, AutoEncoderBackPropagation>(loaded.ae, schedule.aebp);
				break;
			case ModelType::MultilayerPerceptron:
				success = Run<MLP, BackPropagation>(loaded.mlp, schedule.bp);
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
	return -1;
FINISHED:
	return 0;
}