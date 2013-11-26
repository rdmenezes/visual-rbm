// SiCKL
#include <SiCKL.h>

// OMLT
#include <IDX.hpp>
#include <DataAtlas.h>
#include <BackPropagation.h>
#include <MultilayerPerceptron.h>

using namespace OMLT;

// defined in TestCD.cpp
extern float calc_square_error(const float* buff1, const float* buff2, const uint32_t size);
extern void rescale(const float* in_buffer, float* out_buffer, const uint32_t count, const float mean, const float stddev);

bool TrainAutoEncoder(int argc, char** argv)
{
	if(argc != 4)
	{
		printf("Usage: VerifyCD [in_data.idx] [out_features.idx] [out_reconstruction.idx] [out_encode_features.idx]\n");
		return false;
	}

	IDX* in_data = IDX::Load(argv[0]);

	if(in_data == nullptr)
	{
		printf("Could not load %s\n", argv[0]);
		return false;
	}

	const uint32_t input_count = in_data->GetRowLength();
	const uint32_t output_count = 1024;
	IDX* out_features = IDX::Create(argv[1], LittleEndian, Single, output_count);
	if(out_features == nullptr)
	{
		printf("Could not create %s\n", argv[1]);
		return false;
	}
	IDX* out_recon = IDX::Create(argv[2], LittleEndian, Single, in_data->GetRowLength());
	if(out_recon == nullptr)
	{
		printf("Could not create %s\n", argv[2]);
		return false;
	}

	IDX* out_learned_features = IDX::Create(argv[3], LittleEndian, Single, in_data->GetRowLength());
	if(out_learned_features == nullptr)
	{
		printf("Could not create %s\n", argv[3]);
		return false;
	}


	printf("Initing SiCKL\n");

	// startup SiCKL
	SiCKL::OpenGLRuntime::Initialize();

	printf("Setting up model and training parameters\n");

	BP::LayerConfig hidden_config;
	{
		hidden_config.OutputUnits = output_count;
		hidden_config.Function = ActivationFunction::Sigmoid;
		hidden_config.Noisy = true;
		hidden_config.InputDropoutProbability = 0.2f;
	}

	BP::LayerConfig output_config;
	{
		output_config.OutputUnits = input_count;
		output_config.Function = ActivationFunction::Sigmoid;
		output_config.Noisy = false;
		output_config.InputDropoutProbability = 0.5f;
	}

	BP::ModelConfig model_config;
	{
		model_config.InputCount = in_data->GetRowLength();
		model_config.LayerConfigs.push_back(hidden_config);
		model_config.LayerConfigs.push_back(output_config);
	}

	const float base_rate = 0.5f;
	const uint32_t minibatch_size = 10;

	BP::TrainingConfig train_config;
	{
		train_config.LearningRate = base_rate;
		train_config.Momentum = 0.5f;
		train_config.L1Regularization = 0.00001f;
		train_config.L2Regularization = 0.0f;
	}

	printf("Constructing data atlas\n");

	// construct our data atlas
	DataAtlas atlas(in_data);
	atlas.Initialize(minibatch_size, 512);

	SiCKL::OpenGLBuffer2D training_example;

	printf("Constructing BackPropagation algorithm\n");

	BackPropagation bp(model_config, minibatch_size);

	printf("Training\n");

	// do actual training
	const uint32_t epochs = 50;
	for(uint32_t e = 0; e < epochs; e++)
	{
		train_config.LearningRate = 1.0f / ( 1.0f + 0.005f * e) * base_rate;
		bp.SetTrainingConfig(train_config);

		float error = 0.0f;
		for(uint32_t k = 0; k < atlas.GetTotalBatches(); k++)
		{
			atlas.Next(training_example);
			bp.Train(training_example, training_example);
			error += bp.GetLastOutputError();
		}
		error /= atlas.GetTotalBatches();
		printf("Epoch : %u, learning rate : %f, error : %f\n", e, train_config.LearningRate, error);
		//printf("Epoch : %u, learning rate : %f\n", e, train_config.LearningRate);
	}

	printf("Dumping MLP from GPU\n");

	// get our autoencoder from GPU
	MLP* encoder = bp.GetMultilayerPerceptron();

	// pop top layer off, stick it in a new MLP
	MLP* decoder = new MLP();
	decoder->AddLayer(encoder->PopOutputLayer());

	assert(encoder->LayerCount() == 1);
	assert(decoder->LayerCount() == 1);
	assert(encoder->GetLayer(0)->inputs == decoder->GetLayer(0)->outputs);
	assert(encoder->GetLayer(0)->outputs == decoder->GetLayer(0)->inputs);

	float* input_buffer = new float[input_count];
	float* output_buffer = new float[input_count];
	float* hidden_buffer = new float[output_count];

	float err = 0.0f;

	printf("Calculating Mean Square Error");

	for(uint32_t k = 0; k < in_data->GetRowCount(); k++)
	{
		in_data->ReadRow(k, input_buffer);

		encoder->FeedForward(input_buffer, hidden_buffer);
		decoder->FeedForward(hidden_buffer, output_buffer);

		out_features->AddRow(hidden_buffer);
		out_recon->AddRow(output_buffer);

		err += calc_square_error(input_buffer, output_buffer, input_count);
	}

	err /= in_data->GetRowCount();
	printf("Average reconstruction error: %f\n", err);

	out_features->Close();
	out_recon->Close();

	in_data->Close();

	printf("Dumping MLP encoding features to disk\n");

	float sum = 0.0f;
	float sq_sum = 0.0f;

	MLP::Layer* layer = encoder->InputLayer();

	for(uint32_t j = 0; j < output_count; j++)
	{
		for(uint32_t i = 0 ; i < input_count; i++)
		{
			const float& val = layer->weights[j][i];
			sum += val;
			sq_sum += val * val;
		}
	}

	const uint32_t n = (output_count * input_count);
	float mean = sum / n;
	float variance = sq_sum / n - mean * mean;
	float stdev = std::sqrtf(variance);

	float* buffer = new float[input_count];
	for(uint32_t j = 0; j < output_count; j++)
	{
		rescale(layer->weights[j], buffer, input_count, mean, stdev);
		out_learned_features->AddRow(buffer);
	}
	out_learned_features->Close();

	delete[] input_buffer;
	delete[] output_buffer;
	delete[] hidden_buffer;

	delete encoder;
	delete decoder;

	delete out_recon;
	delete out_features;
	delete out_learned_features;
	delete in_data;

	SiCKL::OpenGLRuntime::Finalize();

	return true;
}