// SiCKL
#include <SiCKL.h>

// OMLT
#include <IDX.hpp>
#include <DataAtlas.h>
#include <ContrastiveDivergence.h>
#include <RestrictedBoltzmannMachine.h>

using namespace OMLT;

float calc_square_error(const float* buff1, const float* buff2, const uint32_t size)
{
	float result = 0.0f;
	for(uint32_t i = 0; i < size; i++)
	{
		float diff = buff1[i] - buff2[i];
		result += diff * diff;
	}
	return result / size;
}

void rescale(const float* in_buffer, float* out_buffer, const uint32_t count, const float mean, const float stddev)
{
	for(uint32_t k = 0; k < count; k++)
	{
		out_buffer[k] = 1.0f / (1.0f + std::expf(- (in_buffer[k] - mean) / stddev));
	}
}

bool TrainRBM(int argc, char** argv)
{
	if(argc != 4)
	{
		printf("Usage: TrainRBM [in_data.idx] [out_features.idx] [out_reconstruction.idx] [out_rbm.idx]\n");
		return false;
	}

	IDX* in_data = IDX::Load(argv[0]);
	
	if(in_data == nullptr)
	{
		printf("Could not load %s\n", argv[0]);
		return false;
	}

	const uint32_t hidden_units = 1024;
	IDX* out_features = IDX::Create(argv[1], LittleEndian, Single, hidden_units);
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

	IDX* out_rbm = IDX::Create(argv[3], LittleEndian, Single, in_data->GetRowLength());
	if(out_rbm == nullptr)
	{
		printf("Could not create %s\n", argv[3]);
		return false;
	}


	printf("Initing SiCKL\n");

	// startup SiCKL
	SiCKL::OpenGLRuntime::Initialize();

	printf("Setting up model and training parameters\n");

	const uint32_t minibatch_size = 10;
	// configure our model paramters
	CD::ModelConfig model_config;
	{
		model_config.VisibleUnits = in_data->GetRowLength();
		model_config.HiddenUnits = hidden_units;
		model_config.VisibleType = ActivationFunction::Sigmoid;
		model_config.HiddenType = ActivationFunction::Sigmoid;
	}

	const float base_rate = 0.1f;

	// setup training parameters
	CD::TrainingConfig train_config;
	{
		train_config.VisibleDropout = 0.2f;
		train_config.HiddenDropout = 0.5f;
		train_config.LearningRate = base_rate;
		train_config.Momentum = 0.5f;
		train_config.L1Regularization = 0.0f;
		train_config.L2Regularization = 0.0f;
	}

	printf("Constructing data atlas\n");

	// construct our data atlas
	DataAtlas atlas(in_data);
	atlas.Build(minibatch_size, 512);

	SiCKL::OpenGLBuffer2D training_example;

	printf("Constructing Contrastive Divergence algorithm\n");

	ContrastiveDivergence cd(model_config, minibatch_size);

	printf("Training\n");

	// do actual training
	const uint32_t epochs = 50;
	for(uint32_t e = 0; e < epochs; e++)
	{
		train_config.LearningRate = 1.0f / ( 1.0f + 0.005f * e) * base_rate;
		cd.SetTrainingConfig(train_config);

		float error = 0.0f;
		for(uint32_t k = 0; k < atlas.GetTotalBatches(); k++)
		{
			atlas.Next(training_example);
			cd.Train(training_example);
			error += cd.GetLastReconstructionError();
		}
		error /= atlas.GetTotalBatches();
		printf("Epoch : %u, learning rate : %f, error : %f\n", e, train_config.LearningRate, error);
		//printf("Epoch : %u, learning rate : %f\n", e, train_config.LearningRate);
	}

	printf("Dumping RBM from GPU\n");

	RBM* rbm = cd.GetRestrictedBoltzmannMachine();

	float* visible_buffer = new float[rbm->visible_count];
	float* visible_recon_buffer = new float[rbm->visible_count];
	float* hidden_buffer = new float[rbm->hidden_count];

	float err = 0.0f;

	printf("Calculating Mean Square Error");

	for(uint32_t k = 0; k < in_data->GetRowCount(); k++)
	{
		in_data->ReadRow(k, visible_buffer);

		rbm->CalcHidden(visible_buffer, hidden_buffer);
		rbm->CalcVisible(hidden_buffer, visible_recon_buffer);

		out_features->AddRow(hidden_buffer);
		out_recon->AddRow(visible_recon_buffer);

		err += calc_square_error(visible_buffer, visible_recon_buffer, rbm->visible_count);
	}

	err /= in_data->GetRowCount();
	printf("Average reconstruction error: %f\n", err);

	out_features->Close();
	out_recon->Close();
	in_data->Close();

	printf("Dumping RBM to disk\n");

	float sum = 0.0f;
	float sq_sum = 0.0f;

	for(uint32_t j = 0; j < rbm->hidden_count; j++)
	{
		for(uint32_t i = 0 ; i < rbm->visible_count; i++)
		{
			const float& val = rbm->hidden_features[j][i];
			sum += val;
			sq_sum += val * val;
		}
	}

	const uint32_t n = (rbm->hidden_count * rbm->visible_count);
	float mean = sum / n;
	float variance = sq_sum / n - mean * mean;
	float stdev = std::sqrtf(variance);

	float* buffer = new float[rbm->visible_count];
	rescale(rbm->visible_biases, buffer, rbm->visible_count, mean, stdev);
	out_rbm->AddRow(buffer);

	for(uint32_t j = 0; j < rbm->hidden_count; j++)
	{
		rescale(rbm->hidden_features[j], buffer, rbm->visible_count, mean, stdev);
		out_rbm->AddRow(buffer);
	}
	out_rbm->Close();

	delete[] visible_buffer;
	delete[] visible_recon_buffer;
	delete[] hidden_buffer;
	
	delete rbm;

	delete out_recon;
	delete out_features;
	delete out_rbm;
	delete in_data;

	SiCKL::OpenGLRuntime::Finalize();

	return true;
}