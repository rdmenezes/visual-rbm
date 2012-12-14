#include "RBMTrainer.h"
#include "Common.h"
#include "OpenGLCommon.h"
#include "Shaders.h"

#include "IDX.hpp"
#include "RBM.hpp"

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
	ValidationVisible,
	ValidationVisiblePrime,
	ValidationHiddenProbs,
	ValidationHidddenStates,
	EnabledVisibleUnits,
	EnabledHiddenUnits,
	EnabledVisibleRandom0,
	EnabledVisibleRandom1,
	EnabledHiddenRandom0,
	EnabledHiddenRandom1,
	Count
ENDENUM

ENUM(CalcRandomParams)
	Seeds = 0,
	Count
ENDENUM

ENUM(CalcDepthMapParams)
	EnabledRows = 0,
	EnabledColumns,
	PreviousValues,
	CheckRows,
	CheckColumns,
	UsePreviousValues,
	Offset,
	Count
ENDENUM

ENUM(CalcEnabledUnitParams)
	Random = 0,
	Probability,
	Count
ENDENUM

ENUM(CalcClearDepth)
	Count = 0
ENDENUM

ENUM(CalcCopyTextureParams)
	Source = 0,
	Count
ENDENUM

ENUM(CalcHiddenProbParams)
	VisibleUnits = 0,
	VisibleStates,
	Weights,
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
Shader<CalcEnabledUnitParams> program_calc_enabled_units;
Shader<CalcClearDepth> program_calc_clear_depth;
Shader<CalcCopyTextureParams> program_calc_copy_texture;
Shader<CalcDepthMapParams> program_calc_depth_map;
Shader<CalcHiddenProbParams> program_calc_hidden_probs;
Shader<CalcHiddenStateParams> program_calc_hidden_states;
Shader<CalcVisibleParams> program_calc_visible;
Shader<CalcWeightDeltaParams> program_calc_weight_deltas;
Shader<CalcWeightParams> program_calc_weights;
Shader<CalcErrorParams> program_calc_error;



RBMTrainer::RBMTrainer() 
: Textures(NULL)
, VisibleTextures(NULL)
, VisibleValidationTextures(NULL)
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
, L2Regularization(0.0f)
, HiddenDropout(0.5f)
, VisibleDropout(0.0f)
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
	program_calc_randoms.Build(calc_random, "next_int");
		program_calc_randoms.RegisterParameter(CalcRandomParams::Seeds, "seeds", Texture);

	program_calc_enabled_units.Build(calc_enabled_unit, "state");
		program_calc_enabled_units.RegisterParameter(CalcEnabledUnitParams::Random, "random", Texture);
		program_calc_enabled_units.RegisterParameter(CalcEnabledUnitParams::Probability, "probability", Float);

	program_calc_clear_depth.Build(calc_clear_depth, "value");

	program_calc_copy_texture.Build(calc_copy_texture, "value");
		program_calc_copy_texture.RegisterParameter(CalcCopyTextureParams::Source, "source", Texture);

	program_calc_depth_map.Build(calc_depthmap, "value");
		program_calc_depth_map.RegisterParameter(CalcDepthMapParams::EnabledRows, "enabled_rows", Texture);
		program_calc_depth_map.RegisterParameter(CalcDepthMapParams::EnabledColumns, "enabled_columns", Texture);
		program_calc_depth_map.RegisterParameter(CalcDepthMapParams::PreviousValues, "prev_vals", Texture);
		program_calc_depth_map.RegisterParameter(CalcDepthMapParams::CheckRows, "check_rows", Int);
		program_calc_depth_map.RegisterParameter(CalcDepthMapParams::CheckColumns, "check_columns", Int);
		program_calc_depth_map.RegisterParameter(CalcDepthMapParams::UsePreviousValues, "use_prev_vals", Int);
		program_calc_depth_map.RegisterParameter(CalcDepthMapParams::Offset, "offset", Vec2);

	program_calc_hidden_probs.Build(calc_hidden_probabilities, "probability");
		program_calc_hidden_probs.RegisterParameter(CalcHiddenProbParams::VisibleUnits, "visible_units", Int);	
		program_calc_hidden_probs.RegisterParameter(CalcHiddenProbParams::VisibleStates, "visible_states", Texture);
		program_calc_hidden_probs.RegisterParameter(CalcHiddenProbParams::Weights, "rbm_weights", Texture);

	program_calc_hidden_states.Build(calc_binary_states, "state");
		program_calc_hidden_states.RegisterParameter(CalcHiddenStateParams::Random, "random", Texture);
		program_calc_hidden_states.RegisterParameter(CalcHiddenStateParams::Probs, "probabilities", Texture);

	program_calc_visible.Build(calc_visible_reconstructions, "reconstruction");
		program_calc_visible.RegisterParameter(CalcVisibleParams::HiddenUnits, "hidden_units", Int);
		program_calc_visible.RegisterParameter(CalcVisibleParams::HiddenStates, "hidden_states", Texture);
		program_calc_visible.RegisterParameter(CalcVisibleParams::Weights, "rbm_weights", Texture);
		program_calc_visible.RegisterParameter(CalcVisibleParams::Sigmoid, "sigmoid", Int);
		
	program_calc_weight_deltas.Build(calc_weight_deltas, "delta");
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::Visible, "visible", Texture);
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::Hidden, "hidden", Texture);
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::VisiblePrime, "visible_prime", Texture);
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::HiddenPrime, "hidden_prime", Texture);
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::PrevWeightDeltas, "prev_weight_deltas", Texture);
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::PrevWeights, "prev_weights", Texture);
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::MinibatchSize, "minibatch_size", Int);
		program_calc_weight_deltas.RegisterParameter(CalcWeightDeltaParams::Momentum, "momentum", Float);

	program_calc_weights.Build(calc_new_weights, "new_weight");
		program_calc_weights.RegisterParameter(CalcWeightParams::DeltaWeights, "delta_weights", Texture);
		program_calc_weights.RegisterParameter(CalcWeightParams::PrevWeights, "prev_weights", Texture);
		program_calc_weights.RegisterParameter(CalcWeightParams::WeightFactor, "weight_factor", Float);
		program_calc_weights.RegisterParameter(CalcWeightParams::HiddenFactor, "hidden_factor", Float);
		program_calc_weights.RegisterParameter(CalcWeightParams::VisibleFactor, "visible_factor", Float);
		program_calc_weights.RegisterParameter(CalcWeightParams::LearningRate, "learning_rate", Float);
		program_calc_weights.RegisterParameter(CalcWeightParams::L1Regularization, "l1_regularization", Float);
		program_calc_weights.RegisterParameter(CalcWeightParams::L2Regularization, "l2_regularization", Float);

	program_calc_error.Build(calc_error_vector, "mean_square_error");
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
	ReleaseTextures(Textures, Tex::Count);
	
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

bool RBMTrainer::SetTrainingData(IDX* in_data, bool calculate_stats)
{
	// additional replacement data is being loaded
	if(IsInitialized)
	{
		if(VisibleCount != TrainingData->GetRowLength())
		{
			PreviousError = DataHasIncorrectNumberOfVisibleInputs;
			return false;
		}

		// makes sure the data doesn't contain nans or infinities
		if(!ValidateData(in_data))
		{
			return false;
		}

		// clean up
		if(TrainingData)
		{
			delete TrainingData;
		}

		// data is good, so keep it around
		TrainingData = in_data;

		// calculate data statistics (mean/stddev)
		if(calculate_stats)
		{
			CalcStatistics();
		}

		// transfer to GPU
		TransferDataToGPU(TrainingData, VisibleTextures, Minibatches);
	}
	// loading initial dataset
	else
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

		// set visible count
		VisibleCount = TrainingData->GetRowLength();

		// calculate data statistics (mean/stddev)
		if(calculate_stats)
		{
			CalcStatistics();
		}
		else
		{
			// assume 0 mean 1 stddev
			DataMeans.Acquire(VisibleCount);
			DataStdDev.Acquire(VisibleCount);

			float* m_ptr = DataMeans;
			float* s_ptr = DataStdDev;

			for(int i = 0; i < VisibleCount; i++)
			{
				m_ptr[i] = 0.0f;
				s_ptr[i] = 1.0f;
			}
		}
	}

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


	if(in_data->GetRowLength() != VisibleCount)
	{
		PreviousError = DataHasIncorrectNumberOfVisibleInputs;
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
	float* temp_data_buffer = new float[in_data->GetRowLength()];
	assert(temp_data_buffer != 0);

	PreviousError = NoError;
	for(uint32_t k = 0; k < in_data->GetRowCount(); k++)
	{
		in_data->ReadRow(k, temp_data_buffer);
		for(uint32_t i = 0; i < in_data->GetRowLength(); i++)
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
	for(uint32_t k = 0; k < TrainingData->GetRowCount(); k++)
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

	__m128 divisor = _mm_set1_ps((float)TrainingData->GetRowCount());
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

void RBMTrainer::TransferDataToGPU(IDX* Data, GLuint*& TextureHandles, int& Count)
{
	if(TextureHandles != NULL)
	{
		ReleaseTextures(TextureHandles, Count);
		delete[] TextureHandles;
		TextureHandles = NULL;
	}
	
	// want to shuffle the order in which we add data
	uint32_t* index_buffer = new uint32_t[Data->GetRowCount()];
	for(uint32_t k = 0; k < Data->GetRowCount(); k++)
		index_buffer[k] = k;
	shuffle(index_buffer, Data->GetRowCount());

	// number of minibatches for this data
	Count = Data->GetRowCount() / MinibatchSize;
	// array off texture handles
	TextureHandles = new GLuint[Count];

	float* minibatch_buffer = new float[MinibatchSize * VisibleCount];
	uint32_t index = 0;
	for(uint32_t b = 0; b < Count; b++)
	{
		float* buff = minibatch_buffer;
		for(uint32_t k = 0; k < MinibatchSize; k++)
		{
			Data->ReadRow(index_buffer[index++], buff);

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

		TextureHandles[b] = AllocateFloatTexture(MinibatchSize, VisibleCount, minibatch_buffer);
	}

	delete[] index_buffer;
	delete[] minibatch_buffer;
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

	uint32_t* initial_visible_seeds = new uint32_t[VisibleCount];
	for(uint32_t k = 0; k < VisibleCount; k++)
		initial_visible_seeds[k] = rand();

	uint32_t* initial_hidden_seeds = new uint32_t[HiddenCount];
	for(uint32_t k = 0; k < HiddenCount; k++)
		initial_hidden_seeds[k] = rand();

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
						// logit
						val = (float)log(DataMeans[i-1] / (1 - DataMeans[i-1]));
						if(val < -15.0f)
							val = -15.0f;
						else if(val > 15.0f)
							val = 15.0f;
						break;
					case Gaussian:
						val = 0.0f;	// data is normalized, so the mean visible image is all 0
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
	
	Textures[Tex::Visible] = AllocateFloatTexture(MinibatchSize, VisibleCount);
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
	Textures[Tex::ValidationVisible] = AllocateFloatTexture(MinibatchSize, VisibleCount);
	Textures[Tex::ValidationVisiblePrime] = AllocateFloatTexture(MinibatchSize, VisibleCount);
	Textures[Tex::ValidationHiddenProbs] = AllocateFloatTexture(MinibatchSize, HiddenCount);
	Textures[Tex::ValidationHidddenStates] = AllocateFloatTexture(MinibatchSize, HiddenCount);

	// enabled hidden units for dropout
	Textures[Tex::EnabledVisibleUnits] = AllocateFloatTexture(1, VisibleCount);
	Textures[Tex::EnabledHiddenUnits] = AllocateFloatTexture(1, HiddenCount);
	
	// random seeds for determining which hidden/visible units are enabled
	Textures[Tex::EnabledVisibleRandom0] = AllocateUInt32Texture(1, VisibleCount, initial_visible_seeds);
	Textures[Tex::EnabledVisibleRandom1] = AllocateUInt32Texture(1, VisibleCount);

	Textures[Tex::EnabledHiddenRandom0] = AllocateUInt32Texture(1, HiddenCount, initial_hidden_seeds);
	Textures[Tex::EnabledHiddenRandom1] = AllocateUInt32Texture(1, HiddenCount);
	// delete these buffers
	delete[] initial_rbm_weights;
	delete[] initial_random_seeds;
	delete[] initial_visible_seeds;
	delete[] initial_hidden_seeds;

	/** Allocate Data's Texture Memory **/

	// copy training and validation data to GPU
	TransferDataToGPU(TrainingData, VisibleTextures, Minibatches);
	if(ValidationData)
	{
		TransferDataToGPU(ValidationData, VisibleValidationTextures, ValidationMinibatches);
	}

	BindDepthBuffer(std::max(VisibleCount + 1, MinibatchSize), std::max(HiddenCount + 1, VisibleCount));
}

void RBMTrainer::Train()
{
	assert(IsInitialized == true);

	if(TrainingIndex % Minibatches == 0)
	{
		// shuffle the minibatch order now
		shuffle(VisibleTextures, Minibatches);
	}

	// get next training minibatch
	GLuint visible_data = VisibleTextures[TrainingIndex % Minibatches];

	CalcRandom();
	CalcEnabledUnits();

	CalcVisibleDepth(Textures[Tex::Visible]);
	CalcVisibleCopy(visible_data, Textures[Tex::Visible]);

	CalcHiddenDepth(Textures[Tex::HiddenProbs]);
	CalcHiddenProbs(Textures[Tex::Visible], Textures[Tex::HiddenProbs]);
	CalcHiddenStates(Textures[Tex::HiddenProbs], Textures[Tex::HiddenStates]);
	
	CalcVisibleDepth(Textures[Tex::VisiblePrime]);	
	CalcVisible(Textures[Tex::HiddenStates], Textures[Tex::VisiblePrime]);
	
	CalcHiddenDepth(Textures[Tex::HiddenPrime]);
	CalcHiddenProbs(Textures[Tex::VisiblePrime], Textures[Tex::HiddenPrime]);

	CalcWeightDepth(Textures[Tex::DeltaWeights0], Textures[Tex::DeltaWeights1]);
	CalcWeightDeltas();
	
	CalcWeightDepth(Textures[Tex::Weights0], Textures[Tex::Weights1]);
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
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);
	// set params
	program_calc_randoms.SetParam(CalcRandomParams::Seeds, Textures[Tex::Random0]);
	// run
	program_calc_randoms.SetRenderTargetSize(HiddenCount, MinibatchSize);
	program_calc_randoms.Run(HiddenCount, MinibatchSize, Textures[Tex::Random1]);
	
	//{
	//	unsigned int* buffer0 = new unsigned int[HiddenCount * MinibatchSize];

	//	glBindTexture(GL_TEXTURE_RECTANGLE, Textures[Tex::Random0]);
	//	glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, buffer0);

	//	unsigned int* buffer1 = new unsigned int[HiddenCount * MinibatchSize];

	//	const uint32_t a = 1664525;  
	//	const uint32_t c = 1013904223;  

	//	for(int i = 0; i < HiddenCount * MinibatchSize; i++)
	//	{
	//		buffer1[i] = buffer0[i] * a + c;
	//	}

	//	glBindTexture(GL_TEXTURE_RECTANGLE, Textures[Tex::Random1]);
	//	glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, buffer0);

	//	delete[] buffer0;
	//	delete[] buffer1;
	//}

	// swap those textures
	swap(Textures[Tex::Random0], Textures[Tex::Random1]);
}

void RBMTrainer::CalcEnabledUnits()
{
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);
	
	/// Visible
	// update randoms
	// set params

	program_calc_randoms.SetParam(CalcRandomParams::Seeds, Textures[Tex::EnabledVisibleRandom0]);
	// run
	program_calc_randoms.SetRenderTargetSize(VisibleCount, 1);
	program_calc_randoms.Run(VisibleCount, 1, Textures[Tex::EnabledVisibleRandom1]);

	// calc enabled units
	// set params
	program_calc_enabled_units.SetParam(CalcEnabledUnitParams::Random, Textures[Tex::EnabledVisibleRandom1]);
	program_calc_enabled_units.SetParam(CalcEnabledUnitParams::Probability, 1.0f - VisibleDropout);
	// run
	program_calc_enabled_units.SetRenderTargetSize(VisibleCount, 1);
	program_calc_enabled_units.Run(VisibleCount, 1, Textures[Tex::EnabledVisibleUnits]);
	swap(Textures[Tex::EnabledVisibleRandom0], Textures[Tex::EnabledVisibleRandom1]);
	
	/// Hidden
	// update randoms
	// set params
	
	program_calc_randoms.SetParam(CalcRandomParams::Seeds, Textures[Tex::EnabledHiddenRandom0]);
	// run
	program_calc_randoms.SetRenderTargetSize(HiddenCount, 1);
	program_calc_randoms.Run(HiddenCount, 1, Textures[Tex::EnabledHiddenRandom1]);

	// calc enabled units
	// set params
	program_calc_enabled_units.SetParam(CalcEnabledUnitParams::Random, Textures[Tex::EnabledHiddenRandom1]);
	program_calc_enabled_units.SetParam(CalcEnabledUnitParams::Probability, 1.0f - HiddenDropout);
	// run
	program_calc_enabled_units.SetRenderTargetSize(HiddenCount, 1);
	program_calc_enabled_units.Run(HiddenCount, 1, Textures[Tex::EnabledHiddenUnits]);
	swap(Textures[Tex::EnabledHiddenRandom0], Textures[Tex::EnabledHiddenRandom1]);

	//{
	//	float* enabled_hidden_gpu = GetTexture(HiddenCount, 1, Textures[Tex::EnabledHiddenUnits]);
	//	unsigned int* hidden_rands = new unsigned int[std::max(HiddenCount, VisibleCount)];
	//	glBindTexture(GL_TEXTURE_RECTANGLE, Textures[Tex::EnabledUnitRandom0]);
	//	glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, hidden_rands);
	//
	//	float* enabled_hidden_cpu = new float[HiddenCount];

	//	for(int j = 0; j < HiddenCount; j++)
	//	{
	//		if( (unsigned int)((1.0f - HiddenDropout) * 4294967296.0) > hidden_rands[j])
	//		{
	//			enabled_hidden_cpu[j] = 1.0;
	//		}
	//		else
	//		{
	//			enabled_hidden_cpu[j] = 0.0;
	//		}
	//	}

	//	delete[] enabled_hidden_gpu;
	//	delete[] enabled_hidden_cpu;
	//	delete[] hidden_rands;
	//}
}

void RBMTrainer::CalcVisibleDepth(GLuint visible_tex)
{
	// first calculate the depth buffer for early-z culling
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_ALWAYS);

	// first clear the depth buffer manually
	program_calc_clear_depth.SetDepth(1.0f);
	program_calc_clear_depth.SetRenderTargetSize(VisibleCount, MinibatchSize);
	// run
	program_calc_clear_depth.Run(VisibleCount, MinibatchSize, visible_tex);

	program_calc_depth_map.SetParam(CalcDepthMapParams::EnabledColumns, Textures[Tex::EnabledVisibleUnits]);
	program_calc_depth_map.SetParam(CalcDepthMapParams::EnabledRows, (GLuint)0);
	program_calc_depth_map.SetParam(CalcDepthMapParams::PreviousValues, (GLuint)0);
	program_calc_depth_map.SetParam(CalcDepthMapParams::CheckColumns, 1);
	program_calc_depth_map.SetParam(CalcDepthMapParams::CheckRows, 0);
	program_calc_depth_map.SetParam(CalcDepthMapParams::UsePreviousValues, 0);
	program_calc_depth_map.SetParam(CalcDepthMapParams::Offset, 0.0f, 0.0f);

	program_calc_depth_map.SetDepth(0.0f);

	// run
	program_calc_depth_map.SetRenderTargetSize(VisibleCount, MinibatchSize);
	program_calc_depth_map.Run(VisibleCount, MinibatchSize, visible_tex);
}
	

void RBMTrainer::CalcVisibleCopy(GLuint in_v, GLuint out_v)
{
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LESS);

	program_calc_copy_texture.SetParam(CalcCopyTextureParams::Source, in_v);

	program_calc_copy_texture.SetRenderTargetSize(VisibleCount, MinibatchSize);
	program_calc_copy_texture.Run(VisibleCount, MinibatchSize, out_v);

}

void RBMTrainer::CalcHiddenDepth(GLuint hidden_tex)
{
	// first calculate the depth buffer for early-z culling
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_ALWAYS);

	// first clear the depth buffer manually
	program_calc_clear_depth.SetDepth(1.0f);
	program_calc_clear_depth.SetRenderTargetSize(HiddenCount, MinibatchSize);
	// run
	program_calc_clear_depth.Run(HiddenCount, MinibatchSize, hidden_tex);

	program_calc_depth_map.SetParam(CalcDepthMapParams::EnabledColumns, Textures[Tex::EnabledHiddenUnits]);
	program_calc_depth_map.SetParam(CalcDepthMapParams::EnabledRows, (GLuint)0);
	program_calc_depth_map.SetParam(CalcDepthMapParams::PreviousValues, (GLuint)0);
	program_calc_depth_map.SetParam(CalcDepthMapParams::CheckColumns, 1);
	program_calc_depth_map.SetParam(CalcDepthMapParams::CheckRows, 0);
	program_calc_depth_map.SetParam(CalcDepthMapParams::UsePreviousValues, 0);
	program_calc_depth_map.SetParam(CalcDepthMapParams::Offset, 0.0f, 0.0f);

	program_calc_depth_map.SetDepth(0.0f);

	// run
	program_calc_depth_map.SetRenderTargetSize(HiddenCount, MinibatchSize);
	program_calc_depth_map.Run(HiddenCount, MinibatchSize, hidden_tex);

	//{
	//	float* gpu_hidden = GetTexture(HiddenCount, MinibatchSize, hidden_tex);
	//	float* cpu_hidden = new float[HiddenCount * MinibatchSize];
	//	float* gpu_enabled_hidden = GetTexture(HiddenCount, 1, Textures[Tex::EnabledHiddenUnits]);

	//	for(int k = 0; k < MinibatchSize; k++)
	//	{
	//		for(int j = 0; j < HiddenCount; j++)
	//		{
	//			cpu_hidden[k * HiddenCount + j] = gpu_enabled_hidden[j] == 0.0 ? 0.0 : -1.0;
	//		}
	//	}

	//	glReadBuffer(GL_DEPTH_ATTACHMENT);
	//	float* depths = new float[HiddenCount * MinibatchSize];
	//	memset(depths, 0xCD, sizeof(float) * (HiddenCount * MinibatchSize));
	//	glReadPixels(0, 0, HiddenCount, MinibatchSize, GL_DEPTH_COMPONENT, GL_FLOAT, depths);

	//	delete[] depths;
	//	delete[] gpu_hidden;
	//	delete[] cpu_hidden;
	//	delete[] gpu_enabled_hidden;
	//}
}


void RBMTrainer::CalcHiddenProbs(GLuint visible_tex, GLuint hidden_tex)
{
	// only write fragments with depth less than the current depth (hidden depth shader will have written 0 there otherwise)
	glDepthFunc(GL_LESS);
	glDepthMask(GL_FALSE);

	// set params
	program_calc_hidden_probs.SetParam(CalcHiddenProbParams::VisibleStates, visible_tex);
	program_calc_hidden_probs.SetParam(CalcHiddenProbParams::VisibleUnits, VisibleCount);
	program_calc_hidden_probs.SetParam(CalcHiddenProbParams::Weights, Textures[Tex::Weights0]);
	
	// run
	program_calc_hidden_probs.SetRenderTargetSize(HiddenCount, MinibatchSize);
	program_calc_hidden_probs.Run(HiddenCount, MinibatchSize, hidden_tex);

	//CPU comparison
	//{
	//	float* v = GetTexture(VisibleCount, MinibatchSize, visible_tex);
	//	float* rbm = GetTexture(HiddenCount+1, VisibleCount + 1, Textures[Tex::Weights0]);
	//	float* gpu_enabled_hidden = GetTexture(HiddenCount, 1, Textures[Tex::EnabledHiddenUnits]);

	//	float* cpu_hidden = new float[HiddenCount * MinibatchSize];

	//	for(int j = 0; j < HiddenCount; j++)
	//	{
	//		if(gpu_enabled_hidden[j] == 1.0)
	//		{
	//			for(int k = 0; k < MinibatchSize; k++)
	//			{
	//				int index = k * HiddenCount + j;
	//				cpu_hidden[index] = rbm[j + 1];
	//				for(int i = 0; i < VisibleCount; i++)
	//				{
	//					cpu_hidden[index] += v[k * VisibleCount + i] * rbm[j + 1 + (HiddenCount + 1) * ( i + 1)];
	//				}
	//				cpu_hidden[index] = 1.0f / (1.0f + exp(-cpu_hidden[index]));
	//			}
	//		}
	//		else
	//		{
	//			for(int k = 0; k < MinibatchSize; k++)
	//			{
	//				int index = k * HiddenCount + j;
	//				cpu_hidden[index] = 0.0f;
	//			}
	//		}
	//	}
	//	float* gpu_hidden = GetTexture(HiddenCount, MinibatchSize, hidden_tex);

	//	delete[] v;
	//	delete[] rbm;
	//	delete[] gpu_enabled_hidden;
	//	delete[] cpu_hidden;
	//	delete[] gpu_hidden;
	//}


}

void RBMTrainer::CalcHiddenStates(GLuint hprob, GLuint hstate)
{
	// always write states regardless of depth
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);

	// set params
	program_calc_hidden_states.SetParam(CalcHiddenStateParams::Probs, hprob);
	program_calc_hidden_states.SetParam(CalcHiddenStateParams::Random, Textures[Tex::Random0]);

	// run
	program_calc_hidden_states.SetRenderTargetSize(HiddenCount, MinibatchSize);
	program_calc_hidden_states.Run(HiddenCount, MinibatchSize, hstate);

	//{
	//	float* h = GetTexture(HiddenCount, MinibatchSize, hprob);
	//	unsigned int* random = new unsigned int[HiddenCount * MinibatchSize];

	//	glBindTexture(GL_TEXTURE_RECTANGLE, Textures[Tex::Random0]);
	//	glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, random);

	//	float* gpu_enabled_hidden = GetTexture(HiddenCount, 1, Textures[Tex::EnabledHiddenUnits]);

	//	float* h_states_gpu = GetTexture(HiddenCount, MinibatchSize, hstate);
	//	float* h_states_cpu = new float[HiddenCount * MinibatchSize];

	//	for(int j = 0; j < HiddenCount; j++)
	//	{
	//		if(gpu_enabled_hidden[j] == 1.0)
	//		{
	//			for(int k = 0; k < MinibatchSize; k++)
	//			{
	//				int index = k * HiddenCount + j;
	//				h_states_cpu[index] = (unsigned int)(h[index] * 4294967296.0) > random[index] ? 1.0f : 0.0f;
	//			}
	//		}
	//		else
	//		{
	//			for(int k = 0; k < MinibatchSize; k++)
	//			{
	//				int index = k * HiddenCount + j;
	//				h_states_cpu[index] = 0.0f;
	//			}
	//		}

	//	}

	//	delete[] h;
	//	delete[] random;
	//	delete[] gpu_enabled_hidden;
	//	delete[] h_states_gpu;
	//	delete[] h_states_cpu;
	//}
}

void RBMTrainer::CalcVisible(GLuint hidden_states, GLuint visible_tex)
{
	glDepthFunc(GL_LESS);
	glDepthMask(GL_FALSE);
	// set params
	program_calc_visible.SetParam(CalcVisibleParams::HiddenUnits, HiddenCount);
	program_calc_visible.SetParam(CalcVisibleParams::HiddenStates, hidden_states);
	program_calc_visible.SetParam(CalcVisibleParams::Weights, Textures[Tex::Weights0]);
	program_calc_visible.SetParam(CalcVisibleParams::Sigmoid, (int)(VisibleType == Binary));

	// run
	program_calc_visible.SetRenderTargetSize(VisibleCount, MinibatchSize);
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

	//	delete[] h;
	//	delete[] rbm;
	//	delete[] v_prime;
	//	delete[] v_prime_gpu;
	//}
}

void RBMTrainer::CalcWeightDepth(GLuint prev_w, GLuint w)
{
	// first calculate the depth buffer for early-z culling
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_ALWAYS);

	// first clear the depth buffer manually
	program_calc_clear_depth.SetDepth(1.0f);
	program_calc_clear_depth.SetRenderTargetSize(HiddenCount + 1, VisibleCount + 1);
	// run
	program_calc_clear_depth.Run(HiddenCount + 1, VisibleCount + 1, w);

	// set dimensions
	program_calc_depth_map.SetDepth(0.0f);
	program_calc_depth_map.SetRenderTargetSize(HiddenCount + 1, VisibleCount + 1);

	// first, always enable biases
	program_calc_depth_map.SetParam(CalcDepthMapParams::EnabledColumns, (GLuint)0);
	program_calc_depth_map.SetParam(CalcDepthMapParams::EnabledRows, (GLuint)0);
	program_calc_depth_map.SetParam(CalcDepthMapParams::PreviousValues, (GLuint)0);
	program_calc_depth_map.SetParam(CalcDepthMapParams::CheckColumns, 0);
	program_calc_depth_map.SetParam(CalcDepthMapParams::CheckRows, 0);
	program_calc_depth_map.SetParam(CalcDepthMapParams::UsePreviousValues, 0);
	program_calc_depth_map.SetParam(CalcDepthMapParams::Offset, 0.0f, 0.0f);
	// run
	program_calc_depth_map.Run(0, 0, HiddenCount + 1, 1, w);
	program_calc_depth_map.Run(0, 1, 1, VisibleCount, w);

	// now use the enabled hidden buffer
	program_calc_depth_map.SetParam(CalcDepthMapParams::EnabledColumns, Textures[Tex::EnabledHiddenUnits]);
	program_calc_depth_map.SetParam(CalcDepthMapParams::EnabledRows, Textures[Tex::EnabledVisibleUnits]);
	program_calc_depth_map.SetParam(CalcDepthMapParams::PreviousValues, prev_w);
	program_calc_depth_map.SetParam(CalcDepthMapParams::CheckColumns, 1);
	program_calc_depth_map.SetParam(CalcDepthMapParams::CheckRows, 1);
	program_calc_depth_map.SetParam(CalcDepthMapParams::UsePreviousValues, 1);
	program_calc_depth_map.SetParam(CalcDepthMapParams::Offset, 1.0f, 1.0f);

	// run
	program_calc_depth_map.Run(1, 1, HiddenCount, VisibleCount, w);

	//{
	//	float* gpu_weights = GetTexture(HiddenCount + 1, VisibleCount + 1, w);
	//	float* cpu_weights = new float[(HiddenCount + 1) * (VisibleCount + 1)];
	//	float* gpu_enabled_hidden = GetTexture(HiddenCount, 1, Textures[Tex::EnabledHiddenUnits]);

	//	for(int i = 0; i <= VisibleCount; i++)
	//	{
	//		for(int j = 0; j <= HiddenCount; j++)
	//		{
	//			int index = i * (HiddenCount + 1) + j;
	//			if(i == 0 || j == 0)
	//			{
	//				cpu_weights[index] = -1.0f;
	//			}
	//			else
	//			{
	//				cpu_weights[index] = gpu_enabled_hidden[j-1] == 0.0 ? 0.0 : -1.0;
	//			}
	//		}
	//	}

	//	glReadBuffer(GL_DEPTH_ATTACHMENT);
	//	float* depths = new float[(HiddenCount + 1) * (VisibleCount + 1)];
	//	glReadPixels(0, 0, HiddenCount + 1, VisibleCount + 1, GL_DEPTH_COMPONENT, GL_FLOAT, depths);

	//	delete[] depths;

	//	delete[] gpu_weights;
	//	delete[] cpu_weights;
	//	delete[] gpu_enabled_hidden;
	//}
}


void RBMTrainer::CalcWeightDeltas()
{
	glDepthFunc(GL_LESS);
	glDepthMask(GL_FALSE);

	// set params
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::Visible, Textures[Tex::Visible]);
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::Hidden, Textures[Tex::HiddenProbs]);
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::VisiblePrime, Textures[Tex::VisiblePrime]);
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::HiddenPrime, Textures[Tex::HiddenPrime]);
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::PrevWeightDeltas, Textures[Tex::DeltaWeights0]);
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::PrevWeights, Textures[Tex::Weights0]);
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::MinibatchSize, MinibatchSize);
	program_calc_weight_deltas.SetParam(CalcWeightDeltaParams::Momentum, Momentum);
	
	// run
	program_calc_weight_deltas.SetRenderTargetSize(HiddenCount + 1, VisibleCount + 1);
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
	glDepthFunc(GL_LESS);
	glDepthMask(GL_FALSE);

	// set params
	program_calc_weights.SetParam(CalcWeightParams::DeltaWeights, Textures[Tex::DeltaWeights0]);
	program_calc_weights.SetParam(CalcWeightParams::PrevWeights, Textures[Tex::Weights0]);

	program_calc_weights.SetParam(CalcWeightParams::WeightFactor, WFactor / DeltaWFactor);
	program_calc_weights.SetParam(CalcWeightParams::VisibleFactor, VFactor / DeltaVFactor);
	program_calc_weights.SetParam(CalcWeightParams::HiddenFactor, HFactor / DeltaHFactor);
	
	program_calc_weights.SetParam(CalcWeightParams::LearningRate, LearningRate);
	program_calc_weights.SetParam(CalcWeightParams::L1Regularization, L1Regularization);
	program_calc_weights.SetParam(CalcWeightParams::L2Regularization, L2Regularization);
	
	// run
	program_calc_weights.SetRenderTargetSize(HiddenCount + 1, VisibleCount + 1);
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

		//printf("%f, %f, %f, %f, %f, %f\n", DeltaWFactor, DeltaVFactor, DeltaHFactor, WFactor, VFactor, HFactor);

	}

	swap(Textures[Tex::Weights0], Textures[Tex::Weights1]);
}

float RBMTrainer::CalcError(GLuint v, GLuint vp)
{
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);

	// set params
	program_calc_error.SetParam(CalcErrorParams::Visible, v);
	program_calc_error.SetParam(CalcErrorParams::VisiblePrime, vp);
	program_calc_error.SetParam(CalcErrorParams::Minibatchsize, MinibatchSize);

	// run
	program_calc_error.SetRenderTargetSize(VisibleCount, 1);
	program_calc_error.Run(VisibleCount, 1, Textures[Tex::Error]);

	// now bring it back
	glBindTexture(GL_TEXTURE_RECTANGLE, Textures[Tex::Error]);
	glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED, GL_FLOAT, LocalErrorBuffer);

	
	__m128 result = _mm_set_ps1(0.0f);	// zero out initially

	float* head = LocalErrorBuffer;
	for(uint32_t k = 0; k < LocalErrorBuffer.BlockCount(); k++)
	{
		__m128 temp = _mm_load_ps(head);
		result = _mm_add_ps(result, temp);
		head += 4;
	}


	result = _mm_hadd_ps(result, result);
	result = _mm_hadd_ps(result, result);

	float error;
	_mm_store_ss(&error, result);
	
	return (error / float(VisibleCount)) / (1.0f - VisibleDropout);
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
		CalcEnabledUnits();


		CalcVisibleDepth(Textures[Tex::ValidationVisible]);
		CalcVisibleCopy(vis, Textures[Tex::ValidationVisible]);

		CalcHiddenDepth(Textures[Tex::ValidationHiddenProbs]);
		CalcHiddenProbs(Textures[Tex::ValidationVisible], Textures[Tex::ValidationHiddenProbs]);
		CalcHiddenStates(Textures[Tex::ValidationHiddenProbs], Textures[Tex::ValidationHidddenStates]);
		
		CalcVisibleDepth(Textures[Tex::ValidationVisiblePrime]);
		CalcVisible(Textures[Tex::ValidationHidddenStates], Textures[Tex::ValidationVisiblePrime]);

		float result = CalcError(Textures[Tex::ValidationVisible], Textures[Tex::ValidationVisiblePrime]);

		return result;
	}
	return -1.0f;
}
