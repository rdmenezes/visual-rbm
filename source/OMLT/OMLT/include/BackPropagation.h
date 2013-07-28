// std
#include <stdint.h>
#include <vector>
using std::vector;

// sickl header
#include <SiCKL.h>
using namespace SiCKL;
// omlt
#include "Enums.h"
#include "MultilayerPerceptron.h"
#include "Common.h"




namespace OMLT
{
	// config for a nn trainer
	struct TrainerConfig
	{
		uint32_t MinibatchSize;
		float LearningRate;
		float Momentum;
		float L1Regularization;
		float L2Regularization;
	};

	// training configuration for a given unit
	struct LayerConfig
	{
		// the number of neurons on this layer
		uint32_t OutputUnits;
		// activation function used
		ActivationFunction_t Function;
		// will we add noise to accumulation
		bool Noisy;
		// probability an input unit will be dropped out
		float InputDropoutProbability;
	};

	class BackPropagation
	{
	public:
		BackPropagation(uint32_t in_InputUnits, TrainerConfig in_Config);

		void AddLayer(LayerConfig config);
		float Train(OpenGLBuffer2D& example_input, OpenGLBuffer2D& example_label);
		void Initialize();

		uint32_t LayerCount() const {return Layers.size();}

		MultilayerPerceptron* GetMultilayerPerceptron() const;
		MultilayerPerceptron* GetMultilayerPerceptron(uint32_t being_layer, uint32_t end_layer) const;

	private:

		struct Layer
		{
			Layer();
			~Layer();

			Layer* NextLayer;

			uint32_t InputUnits;
			uint32_t OutputUnits;

			ActivationFunction_t Function;
			bool Noisy;
			float InputDropoutProbability;

			/* 
			 * Inputs from previous layer
			 */
			OpenGLBuffer2D* Input;
			OpenGLBuffer2D InputEnabled;
			// random seeds for input dropout
			OpenGLBuffer2D InputRandom0;
			OpenGLBuffer2D InputRandom1;

			/*
			 * Weights and weight deltas
			 */
			OpenGLBuffer2D Weights0;
			OpenGLBuffer2D Weights1;
			OpenGLBuffer2D DeltaWeights0;
			OpenGLBuffer2D DeltaWeights1;
			OpenGLBuffer2D* OutputEnabled;

			/*
			 * Output associated data
			 */
			OpenGLBuffer2D Activation;
			// random seeds used for Gaussian noise (if warranted)
			OpenGLBuffer2D OutputRandom0;
			OpenGLBuffer2D OutputRandom1;

			/* 
			 * Sensitivity related data
			 */ 
			OpenGLBuffer2D Sensitivities;

			// methods
			OpenGLProgram* CalcEnabledInputs;
			OpenGLProgram* FeedForward;
			OpenGLProgram* CalcSensitivity;
			OpenGLProgram* UpdateWeights;
		};

		Layer* BuildLayer(LayerConfig in_Config);

		vector<Layer*> Layers;
		const uint32_t InputUnits;
		const uint32_t MinibatchSize;		
		const float LearningRate;
		const float Momentum;
		const float L1Regularization;
		const float L2Regularization;

#		include "BackPropagationKernels.h"
	};
}
