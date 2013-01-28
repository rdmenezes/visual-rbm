#pragma once

#include <stdint.h>

#include "Common.h"

enum Model
{
	Model_RBM,
	Model_SRBM
};

enum UnitType 
{ 
	Binary, 
	Gaussian 
};
enum ErrorCode
{
	NoError,
	BinaryDataOutsideZeroOne,
	DataContainsNaN,
	DataContainsInfinite,
	DataHasIncorrectNumberOfVisibleInputs,
	ImportedRBMHasIncorrectNumberOfVisibleInputs,
	RequiredOpenGLVersionUnsupported
};

typedef uint32_t GLuint;

class IDX;
class RBM;
class RBMTrainer
{
public:

	RBMTrainer();
	~RBMTrainer();

	virtual void Reset();

	virtual void Train();

	// training setters
	void SetTrainingData(IDX* in_data);
	void SetValidationData(IDX* in_data);
	

	void SetModelParameters(UnitType in_VisibleType, uint32_t in_HiddenUnits, uint32_t in_MinibatchSize);

	void SetVisibleType(UnitType in_type) {ASSERT(IsInitialized == false); VisibleType = in_type;};
	void SetHiddenCount(uint32_t in_units) {ASSERT(IsInitialized == false); HiddenCount = in_units;};
	void SetMinibatchSize(uint32_t in_size) {ASSERT(IsInitialized == false); MinibatchSize = in_size;};

	void SetLearningRate(float in_learning_rate) { LearningRate = in_learning_rate;};
	void SetMomentum(float in_momentum) { Momentum = in_momentum;};
	void SetL1Regularization(float in_reg) {L1Regularization = in_reg;};
	void SetL2Regularization(float in_reg) {L2Regularization = in_reg;};
	void SetHiddenDropout(float in_dropout) {HiddenDropout = in_dropout;};
	void SetVisibleDropout(float in_droput) {VisibleDropout = in_droput;};
	// sets the maximum GPU allocation allowed before we swap in data from disk (in bytes)
	void SetMaxGPUAllocation(uint32_t in_max_data_memory) {MaxDataMemory = in_max_data_memory;}

	// getters
	uint32_t GetMinibatches() const {return TotalTrainingMinibatches;};
	uint32_t GetMinibatchSize() const {return MinibatchSize;};
	uint32_t GetVisibleCount() const {return VisibleCount;};
	uint32_t GetHiddenCount() const {return HiddenCount;};
	Model GetModelType() const {return Model_RBM;};
	UnitType GetVisibleType() const {return VisibleType;};
	float GetLearningRate() const {return LearningRate;};
	float GetMomentum() const {return Momentum;};
	float GetL1Regularization() const {return L1Regularization;};
	float GetL2Regularization() const {return L2Regularization;};
	float GetVisibleDropout() const {return VisibleDropout;};
	float GetHiddenDropout() const {return HiddenDropout;};

	float GetReconstructionError();
	float GetValidationReconstructionError();
	ErrorCode GetLastErrorCode() {return PreviousError;};
	
	// get a new RBM object dumped from GPU memory
	RBM* GetRBM();
	// copy in new RBM object into GPU memory
	void SetRBM(RBM* in_RBM);

	bool DumpVisible(float* image, float* recon);
	bool DumpHidden(float* activations);
	bool DumpWeights(float* weights);

	// allocate memory and setup initial values for weights
	// returns ture on success, false on error
	bool Initialize();
	bool GetIsInitialized() {return IsInitialized;}

protected:
	
	bool IsInitialized;
	RBM* LoadedRBM;


	float ReconstructionError;

	// training data
	IDX* TrainingData;
	// validation data
	IDX* ValidationData;
	// last encountered error
	ErrorCode PreviousError;

	/** Model Parameters **/
	UnitType VisibleType;
	int32_t VisibleCount;
	int32_t HiddenCount;
	int32_t MinibatchSize;
	int32_t LoadedTrainingMinibatches;	// number of minibatches we have in our training set (in GPU memory)
	int32_t LoadedValidationMinibatches;	// number of minibatches in the validation set (in GPU memory)
	int32_t TotalTrainingMinibatches;	// number of minibatches in the training IDX data file
	int32_t TotalValidationMinibatches;	// number of minibatches in the validation IDX data file

	/** Training Parameters **/
	float LearningRate;
	float Momentum;
	float L1Regularization;
	float L2Regularization;
	float HiddenDropout;
	float VisibleDropout;

	/** Weight Factors **/
	float HFactor;
	float VFactor;
	float WFactor;

	float DeltaHFactor;
	float DeltaVFactor;
	float DeltaWFactor;
	

	/** OpenGL Handles **/
	// array of textures used in training
	GLuint* Textures;
	// array of the raw data used in training
	GLuint* VisibleTrainingTextures;
	GLuint* VisibleValidationTextures;

	/** Training Methods **/

	// calculate which visible/hidden units are enabled
	void CalcEnabledUnits();	
	// update the random number buffers
	void CalcRandom();			
	// calculate depth mask for visible units (given the enabled visible texture)
	void CalcVisibleDepth(GLuint v);
	// calculate depth mask for hidden units (given the enabled hidden textures)
	void CalcHiddenDepth(GLuint h);
	// calculate the depth mask for the weights (given the enabled visible and hidden textures)
	void CalcWeightDepth(GLuint prev_w, GLuint w);
	// copy visible data into new texture
	void CalcVisibleCopy(GLuint in_v, GLuint out_v);
	// calculate hidden probabilities
	void CalcHiddenProbs(GLuint v, GLuint h);
	// calculate the hidden states
	void CalcHiddenStates(GLuint hprob, GLuint hstate);
	// calculate the visible activation/probabilities
	void CalcVisible(GLuint hstate, GLuint v);
	// calculate the weight deltas
	void CalcWeightDeltas();
	// calculate the new weights
	void CalcWeights();
	// calculates the differnece between the given visible/visible reconstruction
	float CalcError(GLuint v, GLuint vp);

	/** Some helper methods **/

	// calculates means and stddev of each piece of data
	void CalcMeans(AlignedMemoryBlock<float>& out_means);
	// allocates empty textures for training/validation data
	GLuint* AllocateEmptyMinibatchTextures(uint32_t Count);
	// copies IDX data (in random order) into textures for training/validation data
	GLuint* AllocatePopulatedMinibatchTextures(uint32_t Count, IDX* Data);
	// transfer a new subset of the given IDX data (in random order) into already allocated textures
	void SwapInNewMinibatchTextures(uint32_t StartIndex, uint32_t Count, IDX* Data, GLuint* Handles);

	// number bytes worth of training examples we can have on the GPU at any given time
	uint32_t MaxDataMemory;

	// which training minibatch are we on
	uint32_t CurrentTrainingMinibatchIndex;
	// which validation minibatch are we on
	uint32_t CurrentValidationMinibatchIndex;

	// which training data row did do we start reading from when swapping?
	uint32_t CurrentTrainingRowIndex;
	// which validation data row do we start reading from when swapping
	uint32_t CurrentValidationRowIndex;


	// do we need to swap training data from disk
	bool SwappingTrainingData;
	// do we need to swap validation data from disk
	bool SwappingValidationData;

	// allocate a block of data 16 byte aligned for the given number of floats
	AlignedMemoryBlock<float> LocalWeightBuffer;
	AlignedMemoryBlock<float> LocalHBiasBuffer;
	AlignedMemoryBlock<float> LocalVBiasBuffer;
	AlignedMemoryBlock<float> LocalErrorBuffer;

};