#include "RBMTrainer.h"
#include "Common.h"
#include "OpenGLCommon.h"

#include "IDX.h"
#include "RBM.h"

#include <malloc.h>
#include <limits>
#include <intrin.h>
#include <math.h>
#include <iostream>

#pragma region Shader Enums

#pragma warning ( push )
#pragma warning (disable : 4480)

#define ENUM(A) struct A { enum  : uint32_t {
#define ENDENUM };};

ENUM(Tex)
	Visible = 0,
	Weights0,
	Weights1,
	HiddenProbs,
	HiddenStates,
	VisiblePrime,
	HiddenPrime,
	DeltaWeights0,
	DeltaWeights1,
	Random0,
	Random1,
	Error,
	ValidationVisiblePrime,
	ValidationHiddenProbs,
	ValidationHidddenStates,
	EnabledHiddenUnits,
	Count
ENDENUM

ENUM(CalcRandomParams)
Seeds = 0,
Count
ENDENUM

ENUM(CalcHiddenProbParams)
	VisibleUnits = 0,
	VisibleStates,
	Weights,
	EnabledHidden,
	Count
ENDENUM

ENUM(CalcHiddenStateParams)
	Random = 0,
	Probs,
	Count
ENDENUM

ENUM(CalcVisibleParams)
	HiddenUnits = 0,
	HiddenStates,
	Weights,
	Sigmoid,
	Count
ENDENUM

ENUM(CalcWeightDeltaParams)
	Visible = 0,
	Hidden,
	VisiblePrime,
	HiddenPrime,
	PrevWeightDeltas,
	PrevWeights,
	MinibatchSize,
	Momentum,
	EnabledHiddenUnits,
	Count
ENDENUM

ENUM(CalcWeightParams)
	DeltaWeights = 0,
	PrevWeights,
	WeightFactor,
	VisibleFactor,
	HiddenFactor,
	LearningRate,
	L1Regularization,
	L2Regularization,
	EnabledHiddenUnits,
	Count
ENDENUM

ENUM(CalcErrorParams)
	Visible = 0,
	VisiblePrime,
	Minibatchsize,
	Count
ENDENUM

#pragma warning (pop)

#pragma endregion

/** Shaders **/
Shader<CalcRandomParams> program_calc_randoms;
Shader<CalcHiddenProbParams> program_calc_hidden_probs;
Shader<CalcHiddenStateParams> program_calc_hidden_states;
Shader<CalcVisibleParams> program_calc_visible;
Shader<CalcWeightDeltaParams> program_calc_weight_deltas;
Shader<CalcWeightParams> program_calc_weights;
Shader<CalcErrorParams> program_calc_error;


RBMTrainer::RBMTrainer() 
: Textures(NULL)
, EnabledHiddenUnitBuffer(NULL)
, IsInitialized(false)
, LoadedRBM(NULL)
, TrainingData(NULL)
, ValidationData(NULL)
, PreviousError(NoError)
, VisibleType(Binary)
, VisibleCount(0)
, HiddenCount(100)
, MinibatchSize(10)
, Minibatches(0)
, ValidationMinibatches(0)
, LearningRate(0.001f)
, Momentum(0.5f)
, L1Regularization(0.0f)
, L2Regularization(0.0001f)
, Dropout(0.5f)
, TrainingIndex(0)
{
	// startup opengl
	if(!StartupOpenGL())
	{
		PreviousError = RequiredOpenGLVersionUnsupported;
		return;
	}

	Textures = new GLuint[Tex::Count];

	/**  Build our Shaders **/
	program_calc_randoms.Build("NativeShaders/calc_random.frag", "next_int");
		program_calc_randoms.RegisterParameter(CalcRandomParams::Seeds, "seeds", Texture);

	program_calc_hidden_probs.Build("NativeShaders/calc_hidden_probabilities.frag", "probability");
		program_calc_hidden_probs.RegisterParameter(CalcHiddenProbParams::VisibleUnits, "visible_units", Int);	
		program_calc_hidden_probs.RegisterParameter(CalcHiddenProbParams::VisibleStates, "visible_states", Texture);
		program_calc_hidden_probs.RegisterParameter(CalcHiddenProbParams::Weights, "rbm_weights", Texture);
		program_calc_hidden_probs.RegisterParameter(CalcHiddenProbParams::EnabledHidden, "enabled_hidden", Texture);

	program_calc_hidden_states.Build("NativeShaders/calc_binary_states.frag", "state");
		program_calc_hidden_states.RegisterParameter(CalcHiddenStateParams::Random, "random", Texture);
		program_calc_hidden_states.RegisterParameter(CalcHiddenStateParams::Probs, "probabilities", Texture);

	program_calc_visible.Build("NativeShaders/calc_visible_reconstructions.frag", "reconstruction");
		program_calc_visible.RegisterParameter(CalcVisibleParams::HiddenUnits, "hidden_units", Int);
		program_calc_visible.RegisterParameter(CalcVisibleParams::HiddenStates, "hidden_states", Texture);
		program_calc_visible.RegisterParameter(CalcVisibleParams::Weights, "rbm_weights", Texture);
		program_calc_visible.RegisterParameter(CalcVisibleParams::Sigmoid, "sigmoid", Int);
		
	program_calc_weight_deltas.Build("NativeShaders/calc_weight_deltas.frag", "delta");
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::Visible, "visible", Texture);
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::Hidden, "hidden", Texture);
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::VisiblePrime, "visible_prime", Texture);
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::HiddenPrime, "hidden_prime", Texture);
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::PrevWeightDeltas, "prev_weight_deltas", Texture);
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::PrevWeights, "prev_weights", Texture);
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::MinibatchSize, "minibatch_size", Int);
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::Momentum, "momentum", Float);
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::EnabledHiddenUnits, "enabled_hidden", Texture);

	program_calc_weights.Build("NativeShaders/calc_new_weights.frag", "new_weight");
		program_calc_weights.RegisterParameter(CalcWeightParams::DeltaWeights, "delta_weights", Texture);
		program_calc_weights.RegisterParameter(CalcWeightParams::PrevWeights, "prev_weights", Texture);
		program_calc_weights.RegisterParameter(CalcWeightParams::WeightFactor, "weight_factor", Float);
		program_calc_weights.RegisterParameter(CalcWeightParams::HiddenFactor, "hidden_factor", Float);
		program_calc_weights.RegisterParameter(CalcWeightParams::VisibleFactor, "visible_factor", Float);
		program_calc_weights.RegisterParameter(CalcWeightParams::LearningRate, "learning_rate", Float);
		program_calc_weights.RegisterParameter(CalcWeightParams::L1Regularization, "l1_regularization", Float);
		program_calc_weights.RegisterParameter(CalcWeightParams::L2Regularization, "l2_regularization", Float);
		program_calc_weights.RegisterParameter(CalcWeightParams::EnabledHiddenUnits, "enabled_hidden", Texture);

	program_calc_error.Build("NativeShaders/calc_error_vector.frag", "mean_square_error");
		program_calc_error.RegisterParameter(CalcErrorParams::Visible, "visible", Texture);
		program_calc_error.RegisterParameter(CalcErrorParams::VisiblePrime, "visible_reconstruction", Texture);
		program_calc_error.RegisterParameter(CalcErrorParams::Minibatchsize, "minibatch_size", Int);
}

RBMTrainer::~RBMTrainer()
{
	delete[] Textures;
	delete[] EnabledHiddenUnitBuffer;

	ShutdownOpenGL();
}

void RBMTrainer::Reset()
{
	assert(IsInitialized == true);

	IsInitialized = false;
	assert(LoadedRBM == NULL);

	// clear allocated space
	LocalWeightBuffer.Release();
	LocalHBiasBuffer.Release();
	LocalVBiasBuffer.Release();
	LocalErrorBuffer.Release();

	delete[] EnabledHiddenUnitBuffer;
	EnabledHiddenUnitBuffer = 0;

	// deallocate training textures
	ReleaseTextures(Textures + 1, Tex::Count - 1);	// don't want to release Visible here because it is temporary, lives in VisibleTextures
	
	// deallocate training data textures
	ReleaseTextures(VisibleTextures, Minibatches);
	delete[] VisibleTextures;
	VisibleTextures = NULL;
	Minibatches = 0;

	// deallocate validation data textures
	if(ValidationData)
	{
		ReleaseTextures(VisibleValidationTextures, ValidationMinibatches);
		delete[] VisibleValidationTextures;
		VisibleValidationTextures = NULL;
		ValidationMinibatches = 0;
	}

	TrainingIndex = 0;
}

bool RBMTrainer::SetTrainingData(IDX* in_data)
{
	// makes sure the data doesn't contain nans or infinities
	if(!ValidateData(in_data))
	{
		return false;
	}

	// clean up
	if(TrainingData)
	{
		delete TrainingData;
		delete ValidationData;
		ValidationData = NULL;
	}

	// data is good, so keep it around
	TrainingData = in_data;

	VisibleCount = TrainingData->RowLength();
	// calculate data statistics (mean/stddev)
	CalcStatistics();

	return true;
}

bool RBMTrainer::SetValidationData(IDX* in_data)
{
	// make sure we have training data
	assert(TrainingData != NULL);

	if(in_data == NULL)
	{
		if(ValidationData)
		{
			delete ValidationData;
			ValidationData = NULL;
		}

		return true;
	}


	if(in_data->RowLength() != VisibleCount)
	{
		PreviousError = ValidationDataHasIncorrectNumberOfVisibleInputs;
		return false;
	}

	// make sure data is valid
	if(!ValidateData(in_data))
	{
		return false;
	}

	// clean up
	if(ValidationData)
	{
		delete ValidationData;
	}
	ValidationData = in_data;
	return true;
}

RBM* RBMTrainer::GetRBM()
{
	assert(IsInitialized == true);

	RBM* result = new RBM(VisibleCount, HiddenCount, VisibleType == Gaussian);

	if(VisibleType == Gaussian)
	{
		memcpy( result->_visible_means, DataMeans, sizeof(float) * VisibleCount);
		memcpy( result->_visible_stddev, DataStdDev, sizeof(float) * VisibleCount);
	}

	float* weights = new float[(VisibleCount + 1) * (HiddenCount + 1)];

	DumpWeights(weights);


	// copy over the hidden biases
	memcpy(result->_hidden_biases, weights + 1, sizeof(float) * HiddenCount);

	float* buf = weights + (HiddenCount + 1);
	for(int k = 0; k < VisibleCount; k++)
	{
		result->_visible_biases[k] = *buf;
		buf += 1;
		memcpy(result->_weights + k * HiddenCount, buf, sizeof(float) * HiddenCount);
		buf += HiddenCount;
	}

	delete[] weights;

	return result;
}

void RBMTrainer::SetRBM(RBM* in_RBM)
{
	assert(IsInitialized == false);
	assert(TrainingData != NULL);

	if(in_RBM->_visible_count != VisibleCount)
	{
		PreviousError = ImportedRBMHasIncorrectNumberOfVisibleInputs;
		return;
	}
	
	// set the data
	LoadedRBM = in_RBM;
	HiddenCount = LoadedRBM->_hidden_count;
	VisibleType = LoadedRBM->_visible_units_linear == true ? Gaussian : Binary;
	
	// copy in loaded rbms stats
	if(LoadedRBM->_visible_units_linear)
	{
		memcpy(DataMeans, LoadedRBM->_visible_means, sizeof(float) * VisibleCount);
		memcpy(DataStdDev, LoadedRBM->_visible_stddev, sizeof(float) * VisibleCount);
	}
}

void RBMTrainer::SetModelParameters(UnitType in_VisibleType, uint32_t in_HiddenUnits, uint32_t in_MinibatchSize)
{
	VisibleType = in_VisibleType;
	HiddenCount = in_HiddenUnits;
	MinibatchSize = in_MinibatchSize;
}

bool RBMTrainer::ValidateData(IDX* in_data)
{
	float* temp_data_buffer = new float[in_data->RowLength()];
	assert(temp_data_buffer != 0);

	PreviousError = NoError;
	for(uint32_t k = 0; k < in_data->Rows(); k++)
	{
		in_data->ReadRow(k, temp_data_buffer);
		for(uint32_t i = 0; i < in_data->RowLength(); i++)
		{
			float& f = temp_data_buffer[i];
			if(f != f)
			{
				PreviousError = DataContainsNaN;
				goto Finished;
			}
			else if(f == std::numeric_limits <float>::infinity() || f == -std::numeric_limits <float>::infinity())
			{
				PreviousError = DataContainsInfinite;
				goto Finished;
			}
			else if(VisibleType == Binary &&
				(f < 0.0f || f > 1.0))
			{
				PreviousError = BinaryDataOutsideZeroOne;
				goto Finished;
			}
		}
	}

Finished:
	delete[] temp_data_buffer;

	return PreviousError == NoError;
}

void RBMTrainer::CalcStatistics()
{
	DataMeans.Acquire(VisibleCount);
	DataStdDev.Acquire(VisibleCount);

	float* mean_head = DataMeans;
	float* stddev_head = DataStdDev;

	memset(mean_head, 0, DataMeans.Size());
	memset(stddev_head, 0, DataMeans.Size());

	AlignedMemoryBlock<float> Row;
	Row.Acquire(VisibleCount);

	// sum up the data and the square data (temporarily stored in means and std dev respectively)
	for(uint32_t k = 0; k < TrainingData->Rows(); k++)
	{
		TrainingData->ReadRow(k, (float*)Row);
		float* data_head = Row;
		for(uint32_t i = 0; i < Row.BlockCount(); i++)
		{
			uint32_t offset = i * 4;

			__m128 vals = _mm_load_ps(data_head + offset);
			__m128 mean = _mm_load_ps(mean_head + offset);

			// update data sum
			mean = _mm_add_ps(mean, vals);
			_mm_store_ps(mean_head + offset, mean);

			// update square data sum
			vals = _mm_mul_ps(vals, vals);
			__m128 stddev = _mm_load_ps(stddev_head + offset);
			stddev = _mm_add_ps(stddev, vals);
			_mm_store_ps(stddev_head + offset, stddev);

		}
	}

	__m128 divisor = _mm_set1_ps((float)TrainingData->Rows());
	// now  using sum data and sum square data, calculate means and stddev
	for(uint32_t i = 0; i < Row.BlockCount(); i++)
	{
		uint32_t offset = i * 4;
		__m128 mean = _mm_load_ps(mean_head + offset);
		mean = _mm_div_ps(mean, divisor);
	
		__m128 stddev = _mm_load_ps(stddev_head + offset);
		stddev = _mm_div_ps(stddev, divisor);

		stddev = _mm_sub_ps(stddev, _mm_mul_ps(mean, mean));
		stddev = _mm_sqrt_ps(stddev);

		_mm_store_ps(mean_head + offset, mean);
		_mm_store_ps(stddev_head + offset, stddev);
	}
}



void RBMTrainer::Initialize()
{
	assert(IsInitialized == false);
	
	IsInitialized = true;

	// seed the random number generator
	srand(1);

	// allocate aligned space
	LocalWeightBuffer.Acquire(VisibleCount * HiddenCount);
	LocalHBiasBuffer.Acquire(HiddenCount);
	LocalVBiasBuffer.Acquire(VisibleCount);
	LocalErrorBuffer.Acquire(VisibleCount);

	// allocate space for enabled hidden units
	EnabledHiddenUnitBuffer = new uint8_t[HiddenCount];

	// setup initial random values
	uint32_t* initial_random_seeds = new uint32_t[MinibatchSize * HiddenCount];
	for(uint32_t k = 0; k < MinibatchSize * HiddenCount; k++)
		initial_random_seeds[k] = rand();

	float* initial_rbm_weights = new float[(VisibleCount + 1) * (HiddenCount + 1)];
	if(LoadedRBM == NULL)
	{
		// randomly initialize weights

		// setup initial weights
		for(uint32_t i = 0; i <= VisibleCount; i++)
		{
			for(uint32_t j = 0; j <= HiddenCount; j++)
			{
				float val = 0.0f;
				// hidden bias 
				if(i == 0)
					val = -4.0f;
				// visible bias
				else if(j == 0)
				{
					// basically we want the visible unit to give the mean
					// when input from the hidden states is 0

					switch(VisibleType)
					{
					case Binary:
						val = (float)log(DataMeans[i-1] / (1 - DataMeans[i-1]));
						if(val < -15.0f)
							val = -15.0f;
						else if(val > 15.0f)
							val = 15.0f;
						break;
					case Gaussian:
						val = DataMeans[i-1];
						break;
					default:
						assert(false);
					}
				}
				// a regular weight
				else
				{
					// finally, gaussian noise for the rest
					val = (float)(NextGaussian() * 0.01);
				}
				// set val
				initial_rbm_weights[i * (HiddenCount + 1) + j] = val;
			}
		}
	}
	else
	{
		assert(LoadedRBM->_hidden_count == HiddenCount && LoadedRBM->_visible_count == VisibleCount);

		// copy loaded rbm to initial_rbm_weights so it can get copied in and ahve factors calculated easily

		for(uint32_t i = 0; i < VisibleCount; i++)
		{
			initial_rbm_weights[(HiddenCount + 1) * (i + 1)] = LoadedRBM->_visible_biases[i];
		}

		for(uint32_t j = 0; j < HiddenCount; j++)
		{
			initial_rbm_weights[j + 1] = LoadedRBM->_hidden_biases[j];
		}

		for(uint32_t i = 0; i < VisibleCount; i++)
			for(uint32_t j = 0; j < HiddenCount; j++)
				initial_rbm_weights[(HiddenCount + 1) * (i + 1) + (j + 1)] = LoadedRBM->_weights[i * HiddenCount + j];


		// get rid of this cpu copy
		delete LoadedRBM;
		LoadedRBM = NULL;
	}


	// calculate initial weight factors
	
	// visible
	VFactor = 0.0f;
	for(uint32_t i = 0; i < VisibleCount; i++)
		VFactor += abs(initial_rbm_weights[(HiddenCount + 1) * (i + 1)]);
	VFactor /= VisibleCount;

	// hidden
	HFactor = 0.0f;
	for(uint32_t j = 0; j < HiddenCount; j++)
		HFactor += abs(initial_rbm_weights[j + 1]);
	HFactor /= HiddenCount;
	

	// weights
	WFactor = 0.0f;
	for(uint32_t i = 0; i < VisibleCount; i++)
		for(uint32_t j = 0; j < HiddenCount; j++)
			WFactor += abs(initial_rbm_weights[(HiddenCount + 1) * (i + 1) + (j + 1)]);
	WFactor /= VisibleCount * HiddenCount; 

	DeltaHFactor = DeltaVFactor = DeltaWFactor = 1.0f;

	/** Allocate Texture Memory for Trainer **/
	
	Textures[Tex::Visible] = 0;
	// 2 buffers for RBM weights
	Textures[Tex::Weights0] = AllocateFloatTexture(VisibleCount + 1, HiddenCount + 1, initial_rbm_weights);
	Textures[Tex::Weights1] = AllocateFloatTexture(VisibleCount + 1, HiddenCount + 1);
	// hidden buffers
	Textures[Tex::HiddenProbs] = AllocateFloatTexture(MinibatchSize, HiddenCount);
	Textures[Tex::HiddenStates] = AllocateFloatTexture(MinibatchSize, HiddenCount);
	// reconstruction buffers
	Textures[Tex::VisiblePrime] = AllocateFloatTexture(MinibatchSize, VisibleCount);
	Textures[Tex::HiddenPrime] = AllocateFloatTexture(MinibatchSize, HiddenCount);
	// weight delta buffers
	Textures[Tex::DeltaWeights0] = AllocateFloatTexture(VisibleCount + 1, HiddenCount + 1);
	Textures[Tex::DeltaWeights1] = AllocateFloatTexture(VisibleCount + 1, HiddenCount + 1);
	// random buffers
	Textures[Tex::Random0] = AllocateUInt32Texture(MinibatchSize, HiddenCount, initial_random_seeds);
	Textures[Tex::Random1] = AllocateUInt32Texture(MinibatchSize, HiddenCount);
	// error buffer
	Textures[Tex::Error] = AllocateFloatTexture(1, VisibleCount);

	// extra buffers for getting reconstruction error on validation set
	Textures[Tex::ValidationVisiblePrime] = AllocateFloatTexture(MinibatchSize, VisibleCount);
	Textures[Tex::ValidationHiddenProbs] = AllocateFloatTexture(MinibatchSize, HiddenCount);
	Textures[Tex::ValidationHidddenStates] = AllocateFloatTexture(MinibatchSize, HiddenCount);

	// enabled hidden units for dropout
	Textures[Tex::EnabledHiddenUnits] = AllocateUInt8Texture(1, HiddenCount);

	// delete these buffers
	delete[] initial_rbm_weights;
	delete[] initial_random_seeds;

	/** Allocate Data's Texture Memory **/

	auto IDXToTextureMemory = [&] (int32_t& texture_count, GLuint*& texture_array, IDX* idx_data)
	{
		// want to shuffle the order in which we add data
		uint32_t* index_buffer = new uint32_t[idx_data->Rows()];
		for(uint32_t k = 0; k < idx_data->Rows(); k++)
			index_buffer[k] = k;
		shuffle(index_buffer, idx_data->Rows());

		// number of minibatches for this data
		texture_count = idx_data->Rows() / MinibatchSize;
		// array off texture handles
		texture_array = new GLuint[texture_count];

		float* minibatch_buffer = new float[MinibatchSize * VisibleCount];
		uint32_t index = 0;
		for(uint32_t b = 0; b < texture_count; b++)
		{
			float* buff = minibatch_buffer;
			for(uint32_t k = 0; k < MinibatchSize; k++)
			{
				idx_data->ReadRow(index_buffer[index++], buff);

				// normalize the data if we're dealing with Gaussian units
				if(VisibleType == Gaussian)
				{
					for(uint32_t i = 0; i < VisibleCount; i++)
					{
						buff[i] -= DataMeans[i];
						if(DataStdDev[i] != 0.0f)
							buff[i] /= DataStdDev[i];
					}
				}

				buff += VisibleCount;
			}

			texture_array[b] = AllocateFloatTexture(MinibatchSize, VisibleCount, minibatch_buffer);
		}

		delete[] index_buffer;
		delete[] minibatch_buffer;
	};

	// copy training and validation data to GPU
	IDXToTextureMemory(Minibatches, VisibleTextures, TrainingData);
	if(ValidationData)
	{
		IDXToTextureMemory(ValidationMinibatches, VisibleValidationTextures, ValidationData);
	}
}

void RBMTrainer::Train()
{
	assert(IsInitialized == true);

	if(TrainingIndex % MinibatchSize == 0)
	{
		// shuffle the minibatch order now
		shuffle(VisibleTextures, Minibatches);
	}

	Textures[Tex::Visible] = VisibleTextures[TrainingIndex % MinibatchSize];

	CalcRandom();
	CalcEnabledHiddenUnits();
	CalcHiddenProbs(Textures[Tex::Visible], Textures[Tex::HiddenProbs]);
	CalcHiddenStates(Textures[Tex::HiddenProbs], Textures[Tex::HiddenStates]);
	CalcVisible(Textures[Tex::HiddenStates], Textures[Tex::VisiblePrime]);
	CalcHiddenProbs(Textures[Tex::VisiblePrime], Textures[Tex::HiddenPrime]);

	CalcWeightDeltas();
	CalcWeights();

	TrainingIndex++;
}

float* GetTexture(int width, int height, GLuint texture)
{
	float* result = new float[width * height];

	glBindTexture(GL_TEXTURE_RECTANGLE, texture);
	glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED, GL_FLOAT, result);

	return result;
}

void RBMTrainer::CalcRandom()
{
	// set params
	program_calc_randoms.SetParam(CalcRandomParams::Seeds, Textures[Tex::Random0]);
	// run
	program_calc_randoms.Run(HiddenCount, MinibatchSize, Textures[Tex::Random1]);

/*
	unsigned int* buffer0 = new unsigned int[HiddenCount * MinibatchSize];

	glBindTexture(GL_TEXTURE_RECTANGLE, Textures[Tex::Random0]);
	glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, buffer0);

	unsigned int* buffer1 = new unsigned int[HiddenCount * MinibatchSize];

	const uint32_t a = 1664525;  
	const uint32_t c = 1013904223;  

	for(int i = 0; i < HiddenCount * MinibatchSize; i++)
	{
		buffer1[i] = buffer0[i] * a + c;
	}

	glBindTexture(GL_TEXTURE_RECTANGLE, Textures[Tex::Random1]);
	glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, buffer0);

	delete[] buffer0;
	delete[] buffer1;
*/

	// swap those textures
	swap(Textures[Tex::Random0], Textures[Tex::Random1]);
}

void RBMTrainer::CalcEnabledHiddenUnits()
{
	for(int j = 0; j < HiddenCount; j++)
	{
		EnabledHiddenUnitBuffer[j] = rand() < Dropout * RAND_MAX ? 255 : 0;
	}

	// copy this buffer to GPU
	glBindTexture(GL_TEXTURE_RECTANGLE, Textures[Tex::EnabledHiddenUnits]);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, HiddenCount, 1, GL_RED_INTEGER, GL_UNSIGNED_BYTE, EnabledHiddenUnitBuffer);


	//uint8_t* enabled_hidden_gpu = new uint8_t[HiddenCount];
	//glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, enabled_hidden_gpu);
	//glBindTexture(GL_TEXTURE_RECTANGLE, 0);
	//delete[] enabled_hidden_gpu;
}

void RBMTrainer::CalcHiddenProbs(GLuint visible_tex, GLuint hidden_tex)
{
	// set params
	program_calc_hidden_probs.SetParam(CalcHiddenProbParams::VisibleStates, visible_tex);
	program_calc_hidden_probs.SetParam(CalcHiddenProbParams::VisibleUnits, VisibleCount);
	program_calc_hidden_probs.SetParam(CalcHiddenProbParams::Weights, Textures[Tex::Weights0]);
	program_calc_hidden_probs.SetParam(CalcHiddenProbParams::EnabledHidden, Textures[Tex::EnabledHiddenUnits]);

	// run
	program_calc_hidden_probs.Run(HiddenCount, MinibatchSize, hidden_tex);

	// CPU comparison
	//{
	//	float* v = GetTexture(VisibleCount, MinibatchSize, visible_tex);
	//	float* rbm = GetTexture(HiddenCount+1, VisibleCount + 1, Textures[Tex::Weights0]);

	//	float* h = new float[HiddenCount * MinibatchSize];

	//	for(int j = 0; j < HiddenCount; j++)
	//	{
	//		if(EnabledHiddenUnitBuffer[j] != 0)
	//		{
	//			for(int k = 0; k < MinibatchSize; k++)
	//			{
	//				int index = k * HiddenCount + j;
	//				h[index] = rbm[j + 1];
	//				for(int i = 0; i < VisibleCount; i++)
	//				{
	//					h[index] += v[k * VisibleCount + i] * rbm[j + 1 + (HiddenCount + 1) * ( i + 1)];
	//				}
	//				h[index] = 1.0f / (1.0f + exp(-h[index]));
	//			}
	//		}
	//		else
	//		{
	//			for(int k = 0; k < MinibatchSize; k++)
	//			{
	//				int index = k * HiddenCount + j;
	//				h[index] = 0.0f;
	//			}
	//		}
	//	}

	//	float* h_prime5 = GetTexture(HiddenCount, MinibatchSize, hidden_tex);
	//	__debugbreak();
	//}
}

void RBMTrainer::CalcHiddenStates(GLuint hprob, GLuint hstate)
{
	// set params
	program_calc_hidden_states.SetParam(CalcHiddenStateParams::Probs, hprob);
	program_calc_hidden_states.SetParam(CalcHiddenStateParams::Random, Textures[Tex::Random0]);

	// run
	program_calc_hidden_states.Run(HiddenCount, MinibatchSize, hstate);

	//{
	//	float* probabilities = GetTexture(HiddenCount, MinibatchSize, Textures[Tex::HiddenProbs]);
	//	unsigned int* random = new unsigned int[HiddenCount * MinibatchSize];

	//	glBindTexture(GL_TEXTURE_RECTANGLE, Textures[Tex::Random0]);
	//	glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, random);

	//	float* h_states_gpu = GetTexture(HiddenCount, MinibatchSize, Textures[Tex::HiddenStates]);
	//	float* h_states = new float[HiddenCount * MinibatchSize];
	//	for(int k = 0;k < HiddenCount * MinibatchSize; k++)
	//	{
	//		if(probabilities[k] > random[k] / 4294967296.0)
	//			h_states[k] = 1.0f;
	//		else
	//			h_states[k] = 0.0f;

	//	}

	//	__debugbreak();
	//}
}

void RBMTrainer::CalcVisible(GLuint hidden_states, GLuint visible_tex)
{
	// set paramsg
	program_calc_visible.SetParam(CalcVisibleParams::HiddenUnits, HiddenCount);
	program_calc_visible.SetParam(CalcVisibleParams::HiddenStates, hidden_states);
	program_calc_visible.SetParam(CalcVisibleParams::Weights, Textures[Tex::Weights0]);
	program_calc_visible.SetParam(CalcVisibleParams::Sigmoid, (int)(VisibleType == Binary));

	// run
	program_calc_visible.Run(VisibleCount, MinibatchSize, visible_tex);

	//{
	//	float* h = GetTexture(HiddenCount, MinibatchSize, Textures[Tex::HiddenStates]);
	//	float* rbm = GetTexture(HiddenCount + 1, VisibleCount + 1, Textures[Tex::Weights0]);

	//	float* v_prime = new float[VisibleCount * MinibatchSize];
	//	for(int i = 0; i < VisibleCount; i++)
	//	{
	//		for(int k = 0; k < MinibatchSize; k++)
	//		{

	//			int index = k * VisibleCount + i;
	//			v_prime[index] = rbm[(HiddenCount + 1) * (i + 1)];
	//			for(int j = 0; j < HiddenCount; j++)
	//			{
	//				v_prime[index] += h[k * HiddenCount + j] * rbm[j + 1 + (HiddenCount + 1) * ( i + 1)];
	//			}
	//			v_prime[index] = 1.0f / (1.0f + exp(-v_prime[index]));
	//		}
	//	}

	//	float* v_prime_gpu = GetTexture(VisibleCount, MinibatchSize, Textures[Tex::VisiblePrime]);

	//	__debugbreak();
	//}
}

void RBMTrainer::CalcWeightDeltas()
{
	// set params
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::Visible, Textures[Tex::Visible]);
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::Hidden, Textures[Tex::HiddenProbs]);
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::VisiblePrime, Textures[Tex::VisiblePrime]);
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::HiddenPrime, Textures[Tex::HiddenPrime]);
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::PrevWeightDeltas, Textures[Tex::DeltaWeights0]);
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::PrevWeights, Textures[Tex::Weights0]);
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::MinibatchSize, MinibatchSize);
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::Momentum, Momentum);
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::EnabledHiddenUnits, Textures[Tex::EnabledHiddenUnits]);

	// run
	program_calc_weight_deltas.Run(HiddenCount + 1, VisibleCount + 1, Textures[Tex::DeltaWeights1]);

	// only update these factors every 250 training iterations
	if(TrainingIndex % 250 == 0)
	{
		// read back the deltas so we can get statistics about them
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(1, 0, HiddenCount, 1, GL_RED, GL_FLOAT, LocalHBiasBuffer);
		glReadPixels(0, 1, 1, VisibleCount, GL_RED, GL_FLOAT, LocalVBiasBuffer);
		glReadPixels(1, 1, HiddenCount, VisibleCount, GL_RED, GL_FLOAT, LocalWeightBuffer);

		DeltaWFactor = CalcMeanAbsValueParallel(LocalWeightBuffer, HiddenCount * VisibleCount);
		DeltaVFactor = CalcMeanAbsValue(LocalVBiasBuffer, VisibleCount);
		DeltaHFactor = CalcMeanAbsValue(LocalHBiasBuffer, HiddenCount);
	}
	// swap texture handles 
	swap(Textures[Tex::DeltaWeights0], Textures[Tex::DeltaWeights1]);
}

void RBMTrainer::CalcWeights()
{
	// set params
	program_calc_weights.SetParam(CalcWeightParams::DeltaWeights, Textures[Tex::DeltaWeights0]);
	program_calc_weights.SetParam(CalcWeightParams::PrevWeights, Textures[Tex::Weights0]);

	program_calc_weights.SetParam(CalcWeightParams::WeightFactor, WFactor / DeltaWFactor);
	program_calc_weights.SetParam(CalcWeightParams::VisibleFactor, VFactor / DeltaVFactor);
	program_calc_weights.SetParam(CalcWeightParams::HiddenFactor, HFactor / DeltaHFactor);
	
	program_calc_weights.SetParam(CalcWeightParams::LearningRate, LearningRate);
	program_calc_weights.SetParam(CalcWeightParams::L1Regularization, L1Regularization);
	program_calc_weights.SetParam(CalcWeightParams::L2Regularization, L2Regularization);

	program_calc_weights.SetParam(CalcWeightParams::EnabledHiddenUnits, Textures[Tex::EnabledHiddenUnits]);

	// run
	program_calc_weights.Run(HiddenCount + 1, VisibleCount + 1, Textures[Tex::Weights1]);

	// only update these factors every 250 training iterations
	if(TrainingIndex % 250 == 0)
	{
		//printf("WFactor: %f\n", WFactor);
		//printf("DeltaWFactor: %f\n", DeltaWFactor);
		// read  bacak weights so we can get statistics
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(1, 1, HiddenCount, VisibleCount, GL_RED, GL_FLOAT, LocalWeightBuffer);
		glReadPixels(0, 1, 1, VisibleCount, GL_RED, GL_FLOAT, LocalVBiasBuffer);
		glReadPixels(1, 0, HiddenCount, 1, GL_RED, GL_FLOAT, LocalHBiasBuffer);

		WFactor = CalcMeanAbsValueParallel(LocalWeightBuffer, HiddenCount * VisibleCount);
		VFactor = CalcMeanAbsValue(LocalVBiasBuffer, VisibleCount);
		HFactor = CalcMeanAbsValue(LocalHBiasBuffer, HiddenCount);
	}

	swap(Textures[Tex::Weights0], Textures[Tex::Weights1]);
}

float RBMTrainer::CalcError(GLuint v, GLuint vp)
{
	// set params
	program_calc_error.SetParam(CalcErrorParams::Visible, v);
	program_calc_error.SetParam(CalcErrorParams::VisiblePrime, vp);
	program_calc_error.SetParam(CalcErrorParams::Minibatchsize, MinibatchSize);

	// run
	program_calc_error.Run(VisibleCount, 1, Textures[Tex::Error]);

	// now bring it back
	glBindTexture(GL_TEXTURE_RECTANGLE, Textures[Tex::Error]);
	glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED, GL_FLOAT, LocalErrorBuffer);

	
	__m128 result = _mm_set_ps1(0.0f);	// zero out initially

	if(VisibleType == UnitType::Binary)
	{
		float* head = LocalErrorBuffer;
		for(uint32_t k = 0; k < LocalErrorBuffer.BlockCount(); k++)
		{
			__m128 temp = _mm_load_ps(head);
			result = _mm_add_ps(result, temp);
			head += 4;
		}
	}
	else if(VisibleType == UnitType::Gaussian)
	{
		uint32_t offset = 0;
		for(uint32_t k = 0; k < LocalErrorBuffer.BlockCount(); k++)
		{
			// multiple data by visible unit variance so that error is back in terms of the data's statistics
			__m128 data = _mm_load_ps((float*)LocalErrorBuffer + offset);	// get error
			__m128 stddev = _mm_load_ps((float*)DataStdDev + offset);	// get stddev
			stddev = _mm_mul_ps(stddev, stddev);	// make it variance

			data = _mm_mul_ps(data, stddev);

			result = _mm_add_ps(result, data);
			offset += 4;
		}
	}

	result = _mm_hadd_ps(result, result);
	result = _mm_hadd_ps(result, result);

	float error;
	_mm_store_ss(&error, result);
	
	return error / float(VisibleCount);
}

bool RBMTrainer::DumpVisible(float* image, float* recon)
{
	if(IsInitialized == false)
	{
		return false;
	}

	glBindTexture(GL_TEXTURE_RECTANGLE, Textures[Tex::Visible]);
	glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED, GL_FLOAT, image);

	glBindTexture(GL_TEXTURE_RECTANGLE, Textures[Tex::VisiblePrime]);
	glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED, GL_FLOAT, recon);

	return true; 
}

bool RBMTrainer::DumpHidden(float* activations)
{
	if(IsInitialized == false)
	{
		return false;
	}

	glBindTexture(GL_TEXTURE_RECTANGLE, Textures[Tex::HiddenProbs]);
	glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED, GL_FLOAT, activations);

	return true;
}

bool RBMTrainer::DumpWeights(float * weights)
{
	if(IsInitialized == false)
	{
		return false;
	}

	glBindTexture(GL_TEXTURE_RECTANGLE, Textures[Tex::Weights0]);
	glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED, GL_FLOAT, weights);

	return true;
}

float RBMTrainer::GetReconstructionError()
{
	return CalcError(Textures[Tex::Visible], Textures[Tex::VisiblePrime]);
}

float RBMTrainer::GetValidationReconstructionError()
{
	if(ValidationMinibatches > 0)
	{

		// get our validation batch
		GLuint vis = VisibleValidationTextures[TrainingIndex % ValidationMinibatches];
	
		// calculate hidden states and reconstructions
		CalcRandom();
		CalcHiddenProbs(vis, Textures[Tex::ValidationHiddenProbs]);
		CalcHiddenStates(Textures[Tex::ValidationHiddenProbs], Textures[Tex::ValidationHidddenStates]);
		CalcVisible(Textures[Tex::ValidationHidddenStates], Textures[Tex::ValidationVisiblePrime]);

		float result = CalcError(vis, Textures[Tex::ValidationVisiblePrime]);

		return result;
	}
	return -1.0f;
}