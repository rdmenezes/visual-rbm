// SiCKL
#include <SiCKL.h>

// OMLT
#include <IDX.hpp>
#include <DataAtlas.h>
#include <BackPropagation.h>
#include <MultilayerPerceptron.h>
#include <TrainingSchedule.h>

using namespace OMLT;

// defined in TestCD.cpp
extern float calc_square_error(const float* buff1, const float* buff2, const uint32_t size);
extern void rescale(const float* in_buffer, float* out_buffer, const uint32_t count, const float mean, const float stddev);

bool TrainAutoEncoder(int argc, char** argv)
{
	if(argc != 4)
	{
		printf("Usage: TrainAutoEncoder [in_data.idx] [out_reconstruction.idx] [in_schedule.json] [out_model.json]\n");
		return false;
	}

	IDX* in_data = IDX::Load(argv[0]);

	if(in_data == nullptr)
	{
		printf("Could not load %s\n", argv[0]);
		return false;
	}
	const uint32_t input_count = in_data->GetRowLength();

	IDX* out_recon = IDX::Create(argv[1], LittleEndian, Single, in_data->GetRowLength());
	if(out_recon == nullptr)
	{
		printf("Could not create %s\n", argv[1]);
		return false;
	}

	std::string schedule_json;
	if(!OMLT::ReadTextFile(argv[2], schedule_json))
	{
		printf("Could not open %s\n", argv[2]);
		return false;
	}

	printf("Loading Training Schedule\n");
	// load up our training schedule
	TrainingSchedule<BackPropagation>* training_schedule = TrainingSchedule<BackPropagation>::FromJSON(schedule_json);
	if(training_schedule == nullptr)
	{
		printf("Could not parse training schedule\n");
		return false;
	}

	printf("Initing SiCKL\n");

	// startup SiCKL
	SiCKL::OpenGLRuntime::Initialize();

	printf("Building DataAtlas\n");
	// construct our data atlas
	DataAtlas atlas(256);
	atlas.Initialize(in_data, training_schedule->GetMinibatchSize());

	printf("Constructing Backpropagation Trainer\n");
	// construct backprop
	BackPropagation bp(training_schedule->GetModelConfig(), training_schedule->GetMinibatchSize(), 1);

	// get our train config
	BackPropagation::TrainingConfig train_config;
	training_schedule->StartTraining();
	training_schedule->GetTrainingConfig(train_config);
	bp.SetTrainingConfig(train_config);

	OpenGLBuffer2D train_example;
	float error = 0.0f;
	uint32_t epoch_count = 0;
	uint32_t iteration_count = 0;
	printf("Training!\n");

	while(training_schedule->TrainingComplete() == false)
	{
		atlas.Next(train_example);
		bp.Train(train_example, train_example);

		error += bp.GetLastOutputError();
		iteration_count++;

		if((iteration_count % atlas.GetTotalBatches()) == 0)
		{
			epoch_count++;
			error /= iteration_count;
			printf("Epoch %u: Error %f\n", epoch_count, error);
			iteration_count = 0;
			error = 0;

			if(training_schedule->NextEpoch() && training_schedule->TrainingComplete() == false)
			{
				training_schedule->GetTrainingConfig(train_config);
				bp.SetTrainingConfig(train_config);
			}
		}
	}

	printf("Writing out reconstructions\n");

	MLP* mlp = bp.GetMultilayerPerceptron();

	float* visible_buffer = (float*)AlignedMalloc(sizeof(float) * (mlp->InputLayer()->inputs + 4), 16);
	float* output_buffer = (float*)AlignedMalloc(sizeof(float) * (mlp->OutputLayer()->outputs + 4), 16);

	for(uint32_t k = 0; k < in_data->GetRowCount(); k++)
	{
		in_data->ReadRow(k, visible_buffer);

		mlp->FeedForward(visible_buffer, output_buffer);

		out_recon->AddRow(output_buffer);
	}

	out_recon->Close();
	
	SiCKL::OpenGLRuntime::Finalize();

	return true;
}