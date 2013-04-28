#include "RBMTrainer.h"
#include "Common.h"

#include "IDX.hpp"
#include "RBM.hpp"

#include <malloc.h>
#include <limits>
#include <intrin.h>
#include <math.h>
#include <iostream>
#include <stdint.h>

#define ENABLE_DEBUG_CODE 0
#if ENABLE_DEBUG_CODE
#define debug_log(...) printf(__VA_ARGS__)
#else
#define debug_log(...) (void)0
#endif


#define SAFE_FREE(X) if(X != nullptr) delete X; X = nullptr;

// largest 32 bit prime
const uint64_t Prime = 4294967291;

// From 'A Friendly Introduction to Number Theory'
// Claim 10.2:
// if gcd(a, m) = 1 then the  numbers:
// b_1 * a, b_2 * a, b_3 * a, ..., b_m-1 * a (mod m)
// are teh same as the numbers
// b1, b2, b3, b_m-1 (mod m)
// although they may be in a different order
// 
// I'll be taking advantage of this fact to do stochastic gradient descent without
// having to explicitly shuffle the minibatche buffers

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

RBMTrainer::RBMTrainer() 
: Buffers(NULL)
, VisibleTrainingBuffers(NULL)
, VisibleValidationBuffers(NULL)
, IsInitialized(false)
, LoadedRBM(NULL)
, TrainingData(NULL)
, ValidationData(NULL)
, PreviousError(NoError)
, VisibleType(Binary)
, VisibleCount(0)
, HiddenCount(100)
, MinibatchSize(10)
, LoadedTrainingMinibatches(0)
, LoadedValidationMinibatches(0)
, LearningRate(0.001f)
, Momentum(0.5f)
, L1Regularization(0.0f)
, L2Regularization(0.0f)
, HiddenDropout(0.5f)
, VisibleDropout(0.0f)
, UpdateDropout(false)
, UpdateTrainingParameters(false)
// 1 gigabyte
, MaxDataMemory(1 * 1024 * 1024 * 1024)
, SwappingTrainingData(false)
, SwappingValidationData(false)
, CurrentTrainingMinibatchIndex(0)
, CurrentValidationMinibatchIndex(0)
, CurrentTrainingRowIndex(0)
, CurrentValidationRowIndex(0)
, TotalTrainingMinibatches(0)
, TotalValidationMinibatches(0)
// kernel programs
, CalcEnabledVisible(nullptr)
, CalcEnabledHidden(nullptr)
, CopyVisible(nullptr)
, CalcHiddenProbability(nullptr)
, CalcStates(nullptr)
, CalcVisible(nullptr)
, CalcWeightUpdates(nullptr)
, CalcErrorVector(nullptr)
{
	// startup opengl
	if(OpenGLRuntime::Initialize() == false)
	{
		PreviousError = RequiredOpenGLVersionUnsupported;
		return;
	}
}

RBMTrainer::~RBMTrainer()
{
	delete[] Buffers;

	OpenGLRuntime::Finalize();
}

void RBMTrainer::SetTrainingData(IDX* in_data)
{
	if(TrainingData)
	{
		delete TrainingData;
	}

	TrainingData = in_data;
	VisibleCount = TrainingData->GetRowLength();
}

void RBMTrainer::SetValidationData(IDX* in_data)
{
	if(ValidationData)
	{
		delete ValidationData;
	}

	ValidationData = in_data;
}

RBM* RBMTrainer::GetRBM()
{
	ASSERT(IsInitialized == true);

	RBM* result = new RBM(VisibleCount, HiddenCount);

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
	ASSERT(IsInitialized == false);
	ASSERT(TrainingData != NULL);

	if(in_RBM->_visible_count != VisibleCount)
	{
		PreviousError = ImportedRBMHasIncorrectNumberOfVisibleInputs;
		return;
	}
	
	// set the data
	LoadedRBM = in_RBM;
	HiddenCount = LoadedRBM->_hidden_count;
}

void RBMTrainer::SetModelParameters(UnitType in_VisibleType, uint32_t in_HiddenUnits, uint32_t in_MinibatchSize)
{
	VisibleType = in_VisibleType;
	HiddenCount = in_HiddenUnits;
	MinibatchSize = in_MinibatchSize;
}

#pragma region Helper Methods

void RBMTrainer::CalcMeans(AlignedMemoryBlock<float>& out_means)
{
	out_means.Acquire(VisibleCount);

	float* mean_head = (float*)out_means;

	memset(mean_head, 0, out_means.Size());
	
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
		}
	}

	__m128 divisor = _mm_set1_ps((float)TrainingData->GetRowCount());
	// now  using sum data and sum square data, calculate means and stddev
	for(uint32_t i = 0; i < Row.BlockCount(); i++)
	{
		uint32_t offset = i * 4;
		__m128 mean = _mm_load_ps(mean_head + offset);
		mean = _mm_div_ps(mean, divisor);

		_mm_store_ps(mean_head + offset, mean);
	}
}

OpenGLBuffer2D* RBMTrainer::AllocateEmptyMinibatchTextures(uint32_t in_Count)
{
	float* zero_bufffer = new float[(VisibleCount + 1) * MinibatchSize];
	memset(zero_bufffer, 0x00, sizeof(float) * (VisibleCount + 1) * MinibatchSize);
	OpenGLBuffer2D* textures = new OpenGLBuffer2D[in_Count];

	for(uint32_t i = 0; i < in_Count; i++)
	{
		textures[i] = OpenGLBuffer2D(VisibleCount + 1, MinibatchSize, ReturnType::Float, zero_bufffer);
	}

	// don't forget to cleanup!
	delete[] zero_bufffer;

	return textures;
}

OpenGLBuffer2D* RBMTrainer::AllocatePopulatedMinibatchTextures(uint32_t in_Count, IDX* in_Data)
{
	// just make sure we're not getting a weird Count value
	ASSERT(in_Data->GetRowCount() / MinibatchSize >= in_Count);

	float* minibatch_buffer = new float[(VisibleCount + 1) * MinibatchSize];

	uint32_t row = 0;
	OpenGLBuffer2D* Textures = new OpenGLBuffer2D[in_Count];
	for(uint32_t i = 0; i < in_Count; i++)
	{
		float* row_buffer = minibatch_buffer;
		// fill up this minibatch
		for(uint32_t j = 0; j < MinibatchSize; j++)
		{
			// bias
			row_buffer[0] = 1.0f;
			in_Data->ReadRow(row, row_buffer + 1);

			row += 1;
			row_buffer += (VisibleCount + 1);
		}

		// transfer to GPU
		Textures[i] = OpenGLBuffer2D(VisibleCount + 1, MinibatchSize, ReturnType::Float, minibatch_buffer);
	}

	// don't forget to cleanup
	delete[] minibatch_buffer;

	return Textures;
}

void RBMTrainer::SwapInNewMinibatchData(uint32_t in_StartIndex, uint32_t in_Count, IDX* in_Data, OpenGLBuffer2D* in_Buffers)
{
	float* minibatch_buffer = new float[(VisibleCount + 1) * MinibatchSize];

	uint32_t row = in_StartIndex;
	for(uint32_t i = 0; i < in_Count; i++)
	{
		float* row_buffer = minibatch_buffer;
		for(uint32_t j = 0; j < MinibatchSize; j++)
		{
			// bias
			row_buffer[0] = 1.0f;
			in_Data->ReadRow(row, row_buffer + 1);

			// row index can overflow in here, in which case go back to beginning
			row = (row + 1) % in_Data->GetRowCount();
			row_buffer += (VisibleCount + 1);
		}

		// copy this new data into texture memory
		in_Buffers[i].SetData(minibatch_buffer);
	}

	delete[] minibatch_buffer;
}

#pragma endregion

bool RBMTrainer::Initialize()
{
	ASSERT(IsInitialized == false);
	ASSERT(TrainingData != NULL);

	// make sure the validation data and training data have the same number of values per row
	if(ValidationData && ValidationData->GetRowLength() != VisibleCount)
	{
		PreviousError = DataHasIncorrectNumberOfVisibleInputs;
		goto ErrorOcurred;
	}

	// seed the random number generator
	srand(1);

	// allocate aligned space
	LocalWeightBuffer.Acquire(VisibleCount * HiddenCount);
	LocalHBiasBuffer.Acquire(HiddenCount);
	LocalVBiasBuffer.Acquire(VisibleCount);
	LocalErrorBuffer.Acquire(VisibleCount);

	// setup initial random values
	uint32_t* initial_random_seeds = new uint32_t[MinibatchSize * HiddenCount];
	for(uint32_t k = 0; k < MinibatchSize * HiddenCount; k++)
	{
		initial_random_seeds[k] = rand();
	}

	uint32_t* initial_visible_seeds = new uint32_t[VisibleCount + 1];
	for(uint32_t k = 0; k <= VisibleCount; k++)
	{
		initial_visible_seeds[k] = rand();
	}

	uint32_t* initial_hidden_seeds = new uint32_t[HiddenCount + 1];
	for(uint32_t k = 0; k <= HiddenCount; k++)
	{
		initial_hidden_seeds[k] = rand();
	}

	float* initial_rbm_weights = new float[(VisibleCount + 1) * (HiddenCount + 1)];
	if(LoadedRBM == NULL)
	{
		AlignedMemoryBlock<float> data_means;
		if(VisibleType == Binary)
		{
			// calculate the means so we can init visible biases properly
			CalcMeans(data_means);
			for(int i = 0; i < VisibleCount; i++)
			{
				if(data_means[i] < 0.0f || data_means[i] > 1.0f)
				{
					PreviousError = BinaryDataOutsideZeroOne;
					goto ErrorOcurred;
				}
			}
		}
		// randomly initialize weights
		for(uint32_t i = 0; i <= VisibleCount; i++)
		{
			for(uint32_t j = 0; j <= HiddenCount; j++)
			{
				float val = 0.0f;
				// hidden bias 
				if(i == 0)
				{
					val = -4.0f;
				}
				// visible bias
				else if(j == 0)
				{
					// basically we want the visible unit to give the mean
					// when input from the hidden states is 0
					switch(VisibleType)
					{
					case Binary:
						// logit function
						{
							float& m = data_means[i - 1];
							m = clamp(m, 0.0001f, 0.999f);
							val = (float)log(m / (1.0f - m)); // range is about -7 to 7 given the above clamp
						}
						break;
					case Gaussian:
						val = 0.0f;	// assume data is normalized, so the mean visible image is all 0
						break;
					default:
						ASSERT(false);
					}
				}
				// a regular weight
				else
				{
					// finally, gaussian noise for the rest
					val = (float)(NextGaussian() / sqrt((double)HiddenCount)) * 0.1;
				}
				// set val
				initial_rbm_weights[i * (HiddenCount + 1) + j] = val;
			}
		}
	}
	else
	{
		ASSERT(LoadedRBM->_hidden_count == HiddenCount && LoadedRBM->_visible_count == VisibleCount);

		// copy loaded rbm to initial_rbm_weights so it can get copied in and have factors calculated easily

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

	/** Allocate Texture Memory for Trainer **/
	
	Buffers = new OpenGLBuffer2D[Tex::Count];

	Buffers[Tex::Visible] = OpenGLBuffer2D(VisibleCount + 1, MinibatchSize, ReturnType::Float, NULL);
		
	// 2 buffers for RBM weights
	Buffers[Tex::Weights0] = OpenGLBuffer2D(HiddenCount + 1, VisibleCount + 1, ReturnType::Float, initial_rbm_weights);
	Buffers[Tex::Weights1] = OpenGLBuffer2D(HiddenCount + 1, VisibleCount + 1, ReturnType::Float, NULL);
	// hidden buffers
	Buffers[Tex::HiddenProbs] = OpenGLBuffer2D(HiddenCount + 1, MinibatchSize, ReturnType::Float, NULL);
	Buffers[Tex::HiddenStates] = OpenGLBuffer2D(HiddenCount + 1, MinibatchSize, ReturnType::Float, NULL);
	// reconstruction buffers
	Buffers[Tex::VisiblePrime] = OpenGLBuffer2D(VisibleCount + 1, MinibatchSize, ReturnType::Float, NULL);
	Buffers[Tex::HiddenPrime] = OpenGLBuffer2D(HiddenCount + 1, MinibatchSize, ReturnType::Float, NULL);
	// weight delta buffers
	Buffers[Tex::DeltaWeights0] = OpenGLBuffer2D(HiddenCount + 1, VisibleCount + 1, ReturnType::Float, NULL);
	Buffers[Tex::DeltaWeights1] = OpenGLBuffer2D(HiddenCount + 1, VisibleCount + 1, ReturnType::Float, NULL);
	// random buffers
	Buffers[Tex::Random0] = OpenGLBuffer2D(HiddenCount + 1, MinibatchSize, ReturnType::UInt, initial_random_seeds);
	Buffers[Tex::Random1] = OpenGLBuffer2D(HiddenCount + 1, MinibatchSize, ReturnType::UInt, NULL);
	// error buffer
	Buffers[Tex::Error] = OpenGLBuffer2D(VisibleCount + 1, 1, ReturnType::Float, NULL);

	// extra buffers for getting reconstruction error on validation set
	Buffers[Tex::ValidationVisible] = OpenGLBuffer2D(VisibleCount + 1, MinibatchSize, ReturnType::Float, NULL);
	Buffers[Tex::ValidationVisiblePrime] = OpenGLBuffer2D(VisibleCount + 1, MinibatchSize, ReturnType::Float, NULL);
	Buffers[Tex::ValidationHiddenProbs] = OpenGLBuffer2D(HiddenCount + 1, MinibatchSize, ReturnType::Float, NULL);
	Buffers[Tex::ValidationHidddenStates] = OpenGLBuffer2D(HiddenCount + 1, MinibatchSize, ReturnType::Float, NULL);

	// enabled hidden units for dropout
	Buffers[Tex::EnabledVisibleUnits] = OpenGLBuffer2D(VisibleCount + 1, 1, ReturnType::UInt, NULL);
	Buffers[Tex::EnabledHiddenUnits] = OpenGLBuffer2D(HiddenCount + 1, 1, ReturnType::UInt, NULL);
	
	// random seeds for determining which hidden/visible units are enabled
	Buffers[Tex::EnabledVisibleRandom0] = OpenGLBuffer2D(VisibleCount + 1, 1, ReturnType::UInt, initial_visible_seeds);
	Buffers[Tex::EnabledVisibleRandom1] = OpenGLBuffer2D(VisibleCount + 1, 1, ReturnType::UInt, NULL);

	Buffers[Tex::EnabledHiddenRandom0] = OpenGLBuffer2D(HiddenCount + 1, 1, ReturnType::UInt, initial_hidden_seeds);
	Buffers[Tex::EnabledHiddenRandom1] = OpenGLBuffer2D(HiddenCount + 1, 1, ReturnType::UInt, NULL);
	// delete these buffers
	delete[] initial_rbm_weights;
	delete[] initial_random_seeds;
	delete[] initial_visible_seeds;
	delete[] initial_hidden_seeds;

	/** Calculate how much texture space we have (given the MaxDataMemory parameter) **/

#pragma region Load Training and Validation Data

	uint32_t TotalAllocated = 0;
	for(int32_t i = 0;  i < Tex::Count; i++)
	{
		TotalAllocated += Buffers[i].GetBufferSize();
	}

	int64_t RemainingMemory = int64_t(MaxDataMemory) - int64_t(TotalAllocated);
	int64_t MaxDataMinibatches = RemainingMemory / (TrainingData->GetRowLengthBytes() * MinibatchSize);
	ASSERT(MaxDataMinibatches > 0);

	// save off the number of minibatches of data we have in our file

	TotalTrainingMinibatches = TrainingData->GetRowCount() / MinibatchSize;
	// no validation data, only need to worry about fitting training data
	if(ValidationData == NULL)
	{
		/** Allocate Data's Texture Memory **/

		// Figure out if we need to swap data from disk
		SwappingTrainingData = TotalTrainingMinibatches > MaxDataMinibatches ? true : false;

		if(SwappingTrainingData == true)
		{
			LoadedTrainingMinibatches = MaxDataMinibatches;
			VisibleTrainingBuffers = AllocateEmptyMinibatchTextures(LoadedTrainingMinibatches);
		}
		else
		{
			LoadedTrainingMinibatches = TotalTrainingMinibatches;
			VisibleTrainingBuffers = AllocatePopulatedMinibatchTextures(LoadedTrainingMinibatches, TrainingData);
		}
	}
	// we also have validation data to worry about
	else
	{
		TotalValidationMinibatches = ValidationData->GetRowCount() / MinibatchSize;
		// if we can fit both into memory, then do so!
		if(TotalTrainingMinibatches + TotalValidationMinibatches < MaxDataMinibatches)
		{
			SwappingTrainingData = false;
			SwappingValidationData = false;

			LoadedTrainingMinibatches = TotalTrainingMinibatches;
			LoadedValidationMinibatches = TotalValidationMinibatches;

			VisibleTrainingBuffers = AllocatePopulatedMinibatchTextures(LoadedTrainingMinibatches, TrainingData);
			VisibleValidationBuffers = AllocatePopulatedMinibatchTextures(LoadedValidationMinibatches, ValidationData);
		}
		// if validation set fits in less than half of the total minibatch budget, just load it all up and only swap training
		else if(TotalValidationMinibatches < MaxDataMinibatches / 2)
		{
			SwappingTrainingData = true;
			SwappingValidationData = false;

			LoadedValidationMinibatches = TotalValidationMinibatches;	// all validation batches
			LoadedTrainingMinibatches = MaxDataMinibatches - LoadedValidationMinibatches;	// remaining batches go to trianing

			VisibleTrainingBuffers = AllocateEmptyMinibatchTextures(LoadedTrainingMinibatches);
			VisibleValidationBuffers = AllocatePopulatedMinibatchTextures(LoadedValidationMinibatches, ValidationData);
		}
		// we have to swap in both training and validation data
		else
		{
			SwappingTrainingData = true;
			SwappingValidationData = true;

			// allocate equal space for both
			LoadedValidationMinibatches = LoadedTrainingMinibatches = MaxDataMinibatches / 2;

			// empty textures
			VisibleTrainingBuffers = AllocateEmptyMinibatchTextures(LoadedTrainingMinibatches);
			VisibleValidationBuffers = AllocateEmptyMinibatchTextures(LoadedValidationMinibatches);
		}
	}
	CurrentTrainingRowIndex = 0;
	CurrentValidationRowIndex = 0;

#pragma endregion

	// parse and build our training programs !
	BuildKernels();

	IsInitialized = true;

	return true;
ErrorOcurred:
	// cleanup
	Reset();
	return false;
}

void RBMTrainer::BuildKernels()
{
	BuildEnabledKernels();
	BuildVisibleHiddenKernels();
	BuildWeightKernels();
}

void RBMTrainer::BuildEnabledKernels()
{
	CalcEnabled_Source.enabled_probability = 1.0f - VisibleDropout;
	CalcEnabled_Source.Parse();
	SAFE_FREE(CalcEnabledVisible);
	CalcEnabledVisible = Compiler.Build(CalcEnabled_Source);
	CalcEnabledVisible->Initialize(VisibleCount + 1, 1);
	debug_log("CalcEnabledVisible:\n%s\n", CalcEnabledVisible->GetSource().c_str());

	CalcEnabled_Source.enabled_probability = 1.0f - HiddenDropout;
	CalcEnabled_Source.Parse();
	SAFE_FREE(CalcEnabledHidden);
	CalcEnabledHidden = Compiler.Build(CalcEnabled_Source);
	CalcEnabledHidden->Initialize(HiddenCount + 1, 1);
	debug_log("CalcEnabledHidden:\n%s\n", CalcEnabledHidden->GetSource().c_str());
}

void RBMTrainer::BuildVisibleHiddenKernels()
{
	CopyVisible_Source.Parse();
	SAFE_FREE(CopyVisible);
	CopyVisible = Compiler.Build(CopyVisible_Source);
	CopyVisible->Initialize(VisibleCount + 1, MinibatchSize);
	debug_log("CopyVisible:\n%s\n", CopyVisible->GetSource().c_str());

	CalcHiddenProbability_Source.visible_units = VisibleCount;
	CalcHiddenProbability_Source.Parse();
	SAFE_FREE(CalcHiddenProbability);
	CalcHiddenProbability = Compiler.Build(CalcHiddenProbability_Source);
	CalcHiddenProbability->Initialize(HiddenCount + 1, MinibatchSize);
	debug_log("CalcHiddenProbability:\n%s\n", CalcHiddenProbability->GetSource().c_str());

	CalcStates_Source.Parse();
	SAFE_FREE(CalcStates);
	CalcStates = Compiler.Build(CalcStates_Source);
	CalcStates->Initialize(HiddenCount + 1, MinibatchSize);
	debug_log("CalcStates:\n%s\n", CalcStates->GetSource().c_str());

	CalcVisible_Source.use_sigmoid = (VisibleType == Binary);
	CalcVisible_Source.hidden_units = HiddenCount;
	CalcVisible_Source.Parse();
	SAFE_FREE(CalcVisible);
	CalcVisible = Compiler.Build(CalcVisible_Source);
	CalcVisible->Initialize(VisibleCount + 1, MinibatchSize);
	debug_log("CalcVisible:\n%s\n", CalcVisible->GetSource().c_str());

	CalcErrorVector_Source.minibatch_size = MinibatchSize;
	CalcErrorVector_Source.Parse();
	SAFE_FREE(CalcErrorVector);
	CalcErrorVector = Compiler.Build(CalcErrorVector_Source);
	CalcErrorVector->Initialize(VisibleCount + 1, 1);
	debug_log("CalcErrorVector:\n%s\n", CalcErrorVector->GetSource().c_str());
}

void RBMTrainer::BuildWeightKernels()
{
	CalcWeightUpdates_Source.minibatch_size = MinibatchSize;
	CalcWeightUpdates_Source.learning_rate = LearningRate;
	CalcWeightUpdates_Source.momentum = Momentum;
	CalcWeightUpdates_Source.l1_regularization = L1Regularization;
	CalcWeightUpdates_Source.l2_regularization = L2Regularization;
	CalcWeightUpdates_Source.Parse();
	SAFE_FREE(CalcWeightUpdates);
	CalcWeightUpdates = Compiler.Build(CalcWeightUpdates_Source);
	CalcWeightUpdates->Initialize(HiddenCount + 1, VisibleCount + 1);
	debug_log("CalcWeightUpdates:\n%s\n", CalcWeightUpdates->GetSource().c_str());
}

void RBMTrainer::FreeKernels()
{
	SAFE_FREE(CalcEnabledVisible);
	SAFE_FREE(CalcEnabledHidden);
	SAFE_FREE(CopyVisible);
	SAFE_FREE(CalcHiddenProbability);
	SAFE_FREE(CalcStates);
	SAFE_FREE(CalcVisible);
	SAFE_FREE(CalcWeightUpdates);
	SAFE_FREE(CalcErrorVector);
}

void RBMTrainer::ReleaseBuffers(OpenGLBuffer2D*& buffers, uint32_t count)
{
	if(buffers)
	{
		for(uint32_t i = 0; i < count; i++)
		{
			buffers[i] = OpenGLBuffer2D();
		}
		delete[] buffers;
		buffers = nullptr;
	}
}

void RBMTrainer::Reset()
{
	IsInitialized = false;
	ASSERT(LoadedRBM == NULL);

	// clear allocated space
	LocalWeightBuffer.Release();
	LocalHBiasBuffer.Release();
	LocalVBiasBuffer.Release();
	LocalErrorBuffer.Release();

	// deallocate training textures
	ReleaseBuffers(Buffers, Tex::Count);

	// deallocate training data textures
	ReleaseBuffers(VisibleTrainingBuffers, LoadedTrainingMinibatches);
	LoadedTrainingMinibatches = 0;
	SwappingTrainingData = false;
	CurrentTrainingRowIndex = 0;
	CurrentTrainingMinibatchIndex = 0;


	if(ValidationData)
	{
		// deallocate validation data textures
		ReleaseBuffers(VisibleValidationBuffers, LoadedValidationMinibatches);
		LoadedValidationMinibatches = 0;
		SwappingValidationData = false;
		CurrentValidationRowIndex = 0;
		CurrentValidationMinibatchIndex = 0;
	}

	FreeKernels();
}

void RBMTrainer::Train()
{
	ASSERT(IsInitialized == true);

	if(CurrentTrainingMinibatchIndex == 0)
	{
		if(SwappingTrainingData == true)
		{
			SwapInNewMinibatchData(CurrentTrainingRowIndex, LoadedTrainingMinibatches, TrainingData, VisibleTrainingBuffers);
		}
	}

	if(UpdateDropout)
	{
		UpdateDropout = false;
		BuildEnabledKernels();
	}
	if(UpdateTrainingParameters)
	{
		UpdateTrainingParameters = false;
		BuildWeightKernels();
	}

	OpenGLBuffer2D& visible_data = VisibleTrainingBuffers[(CurrentTrainingMinibatchIndex * Prime) % LoadedTrainingMinibatches];

	// calc which hidden and visible units will be enabled this round
	CalcEnabledUnits();
	// copy data to new buffer and dropout visible data
	CalcVisibleCopy(visible_data, Buffers[Tex::Visible]);

	// 1 gibs sampling
	CalcHiddenProbs(Buffers[Tex::Visible], Buffers[Tex::HiddenProbs]);
	CalcHiddenStates(Buffers[Tex::HiddenProbs], Buffers[Tex::HiddenStates]);
	CalcVisiblePrime(Buffers[Tex::HiddenStates], Buffers[Tex::VisiblePrime]);
	CalcHiddenProbs(Buffers[Tex::VisiblePrime], Buffers[Tex::HiddenPrime]);

	// now update the weights
	CalcWeights();
	
	// update the minibatch index
	CurrentTrainingMinibatchIndex = (CurrentTrainingMinibatchIndex + 1) % LoadedTrainingMinibatches;
	// and update our row index
	CurrentTrainingRowIndex = (CurrentTrainingRowIndex + MinibatchSize) % TrainingData->GetRowCount();
}

void RBMTrainer::CalcEnabledUnits()
{
	// update the enabled visible units
	CalcEnabledVisible->SetInput(0, Buffers[Tex::EnabledVisibleRandom0]);

	CalcEnabledVisible->BindOutput(0, Buffers[Tex::EnabledVisibleUnits]);
	CalcEnabledVisible->BindOutput(1, Buffers[Tex::EnabledVisibleRandom1]);

	CalcEnabledVisible->Run();

	// update the enabled hidden units
	CalcEnabledHidden->SetInput(0, Buffers[Tex::EnabledHiddenRandom0]);

	CalcEnabledHidden->BindOutput(0, Buffers[Tex::EnabledHiddenUnits]);
	CalcEnabledHidden->BindOutput(1, Buffers[Tex::EnabledHiddenRandom1]);

	CalcEnabledHidden->Run();

	// swap the random seed buffers
	swap(Buffers[Tex::EnabledVisibleRandom0],  Buffers[Tex::EnabledVisibleRandom1]);
	swap(Buffers[Tex::EnabledHiddenRandom0],  Buffers[Tex::EnabledHiddenRandom1]);
}

void RBMTrainer::CalcVisibleCopy(OpenGLBuffer2D& source, OpenGLBuffer2D& destination)
{
	CopyVisible->SetInput(0, source);
	CopyVisible->SetInput(1, Buffers[Tex::EnabledVisibleUnits]);

	CopyVisible->BindOutput(0, destination);

	CopyVisible->Run();
}

void RBMTrainer::CalcHiddenProbs(OpenGLBuffer2D& visible, OpenGLBuffer2D& hidden_probs)
{
	CalcHiddenProbability->SetInput(0, visible);
	CalcHiddenProbability->SetInput(1, Buffers[Tex::Weights0]);
	CalcHiddenProbability->SetInput(2, Buffers[Tex::EnabledHiddenUnits]);

	CalcHiddenProbability->BindOutput(0, hidden_probs);

	CalcHiddenProbability->Run();
}


void RBMTrainer::CalcHiddenStates(OpenGLBuffer2D& probs, OpenGLBuffer2D& states)
{
	CalcStates->SetInput(0, Buffers[Tex::Random0]);
	CalcStates->SetInput(1, probs);

	CalcStates->BindOutput(0, Buffers[Tex::Random1]);
	CalcStates->BindOutput(1, states);

	CalcStates->Run();

	swap(Buffers[Tex::Random0], Buffers[Tex::Random1]);
}

void RBMTrainer::CalcVisiblePrime(OpenGLBuffer2D& hidden, OpenGLBuffer2D& visible)
{
	CalcVisible->SetInput(0, hidden);
	CalcVisible->SetInput(1, Buffers[Tex::Weights0]);
	CalcVisible->SetInput(2, Buffers[Tex::EnabledVisibleUnits]);

	CalcVisible->BindOutput(0, visible);

	CalcVisible->Run();
}

void RBMTrainer::CalcWeights()
{
	CalcWeightUpdates->SetInput(0, Buffers[Tex::Visible]);
	CalcWeightUpdates->SetInput(1, Buffers[Tex::HiddenProbs]);
	CalcWeightUpdates->SetInput(2, Buffers[Tex::VisiblePrime]);
	CalcWeightUpdates->SetInput(3, Buffers[Tex::HiddenPrime]);
	CalcWeightUpdates->SetInput(4, Buffers[Tex::DeltaWeights0]);
	CalcWeightUpdates->SetInput(5, Buffers[Tex::Weights0]);
	CalcWeightUpdates->SetInput(6, Buffers[Tex::EnabledVisibleUnits]);
	CalcWeightUpdates->SetInput(7, Buffers[Tex::EnabledHiddenUnits]);

	CalcWeightUpdates->BindOutput(0, Buffers[Tex::DeltaWeights1]);
	CalcWeightUpdates->BindOutput(1, Buffers[Tex::Weights1]);

	CalcWeightUpdates->Run();

	swap(Buffers[Tex::Weights0], Buffers[Tex::Weights1]);
	swap(Buffers[Tex::DeltaWeights0], Buffers[Tex::DeltaWeights1]);
}

float RBMTrainer::CalcError(OpenGLBuffer2D& v, OpenGLBuffer2D& vp)
{
	CalcErrorVector->SetInput(0, v);
	CalcErrorVector->SetInput(1, vp);

	CalcErrorVector->BindOutput(0, Buffers[Tex::Error]);

	CalcErrorVector->Run();

	float* head = LocalErrorBuffer;
	CalcErrorVector->GetOutput(0, head); 

	__m128 result = _mm_set_ps1(0.0f);	// zero out initially
	
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

	return (error / float(VisibleCount * MinibatchSize) );
}

bool RBMTrainer::DumpVisible(float* image, float* recon)
{
	if(IsInitialized == false)
	{
		return false;
	}

	Buffers[Tex::Visible].GetData(image);
	Buffers[Tex::VisiblePrime].GetData(recon);

	return true; 
}

bool RBMTrainer::DumpHidden(float* activations)
{
	if(IsInitialized == false)
	{
		return false;
	}

	Buffers[Tex::HiddenProbs].GetData(activations);

	return true;
}

bool RBMTrainer::DumpWeights(float * weights)
{
	if(IsInitialized == false)
	{
		return false;
	}

	Buffers[Tex::Weights0].GetData(weights);

	return true;
}

float RBMTrainer::GetReconstructionError()
{
	return CalcError(Buffers[Tex::Visible], Buffers[Tex::VisiblePrime]);
}

float RBMTrainer::GetValidationReconstructionError()
{
	float result = -1.0f;

	if(LoadedValidationMinibatches > 0)
	{
		if(CurrentValidationMinibatchIndex == 0)
		{
			if(SwappingValidationData == true)
			{
				SwapInNewMinibatchData(CurrentValidationRowIndex, LoadedValidationMinibatches, ValidationData, VisibleValidationBuffers);
			}
		}

		OpenGLBuffer2D& visible_data = VisibleValidationBuffers[CurrentValidationMinibatchIndex % LoadedValidationMinibatches];

		// calc which hidden and visible units will be enabled this round
		CalcEnabledUnits();
		// copy data to new buffer and dropout visible data
		CalcVisibleCopy(visible_data, Buffers[Tex::ValidationVisible]);
		CalcHiddenProbs(visible_data, Buffers[Tex::ValidationHiddenProbs]);
		CalcHiddenStates(Buffers[Tex::ValidationHiddenProbs], Buffers[Tex::ValidationHidddenStates]);

		CalcVisiblePrime(Buffers[Tex::ValidationHidddenStates], Buffers[Tex::ValidationVisiblePrime]);
		result = CalcError(visible_data, Buffers[Tex::ValidationVisiblePrime]);

		// update the minibatch index
		CurrentValidationMinibatchIndex = (CurrentValidationMinibatchIndex + 1) % LoadedValidationMinibatches;
		// and update our row index
		CurrentValidationRowIndex = (CurrentValidationRowIndex + MinibatchSize) % ValidationData->GetRowCount();
	}

	return result;
}

