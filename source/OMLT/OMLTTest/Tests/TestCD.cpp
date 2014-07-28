// std
#include <stdio.h>

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
	DataAtlas atlas(512);
	atlas.Initialize(in_data, minibatch_size);

	SiCKL::OpenGLBuffer2D training_example;

	printf("Constructing Contrastive Divergence algorithm\n");

	ContrastiveDivergence cd(model_config, minibatch_size, 1);

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

	delete[] visible_buffer;
	delete[] visible_recon_buffer;
	delete[] hidden_buffer;
	
	delete rbm;

	delete out_recon;
	delete out_features;
	delete in_data;

	SiCKL::OpenGLRuntime::Finalize();

	return true;
}

// train an RBM, dump it to JSON, convert JSON back to RBM object,
// create new CD with said RBM object, dump RBM to reserialized json
bool SerializeRBM(int argc, char** argv)
{
	if(argc != 3)
	{
		printf("Usage: SerializeRBM [data.idx] [trained_rbm.json] [reserialized_rbm.json]\n");
		return false;
	}



	printf("Loading Data\n");

	IDX* data = IDX::Load(argv[0]);
	if(data == nullptr)
	{
		printf("Could not load %s as IDX file\n", argv[0]);
		return false;
	}

	FILE* rbm_json = fopen(argv[1], "wb");
	if(rbm_json == nullptr)
	{
		printf("Could not create %s\n", argv[1]);
		return false;
	}

	FILE* rbm_reserial_json = fopen(argv[2], "wb");
	if(rbm_reserial_json == nullptr)
	{
		printf("Could not create %s\n", argv[2]);
		return false;
	}

	printf("Initing SiCKL\n");
	SiCKL::OpenGLRuntime::Initialize();

	printf("Construction training params");

	const uint32_t minibatch_size = 10;

	CD::ModelConfig model_config;
	{
		model_config.VisibleUnits = data->GetRowLength();
		model_config.VisibleType = ActivationFunction::Sigmoid;
		model_config.HiddenUnits = 100;
		model_config.HiddenType = ActivationFunction::Sigmoid;
	}

	CD::TrainingConfig train_config;
	{
		train_config.LearningRate = 0.1f;
		train_config.Momentum = 0.5f;
		train_config.L1Regularization = 0.0f;
		train_config.L2Regularization = 0.0f;
		train_config.VisibleDropout = 0.0f;
		train_config.HiddenDropout = 0.0f;
	}

	printf("Constructing CD trainer\n");

	ContrastiveDivergence cd1(model_config, minibatch_size, 1);
	cd1.SetTrainingConfig(train_config);

	printf("Building data atlas\n");

	DataAtlas atlas(256);
	atlas.Initialize(data, minibatch_size);

	SiCKL::OpenGLBuffer2D training_example;

	printf("Training...\n");

	const uint32_t epochs = 20;
	for(uint32_t e = 0; e < epochs; e++)
	{
		for(uint32_t k = 0; k < atlas.GetTotalBatches(); k++)
		{
			atlas.Next(training_example);
			cd1.Train(training_example);
		}
		printf("Epoch: %u\n", e);
	}

	printf("Dumping RBM from texture memory\n");

	RBM* rbm = cd1.GetRestrictedBoltzmannMachine();

	printf("Serializing to disk\n");

	fprintf(rbm_json, "%s\n", rbm->ToJSON().c_str());
	fclose(rbm_json);
	
	printf("Creating new CD\n");

	ContrastiveDivergence cd2(rbm, minibatch_size, 1);

	printf("Dumping re-serialized RBM from texture memory\n");

	RBM* rbm_reserial = cd2.GetRestrictedBoltzmannMachine();
	
	printf("And Serializing that to disk\n");
	
	fprintf(rbm_reserial_json, "%s\n", rbm_reserial->ToJSON().c_str());
	fclose(rbm_reserial_json);

	printf("Cleanup\n");

	delete rbm;
	delete rbm_reserial;

	SiCKL::OpenGLRuntime::Finalize();
}