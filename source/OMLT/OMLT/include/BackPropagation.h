// std
#include <stdint.h>
#include <vector>
#include <assert.h>
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
	class BackPropagation
	{
	public:

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

		struct ModelConfig
		{
			uint32_t InputCount;
			std::vector<LayerConfig> LayerConfigs;
		};

		// config for a nn trainer
		struct TrainingConfig
		{
			float LearningRate;
			float Momentum;
			float L1Regularization;
			float L2Regularization;

			TrainingConfig()
				: LearningRate(0.0f)
				, Momentum(0.0f)
				, L1Regularization(0.0f)
				, L2Regularization(0.0f)
			{ }
		};

		
		BackPropagation(const ModelConfig, uint32_t in_minibatchsize);
		BackPropagation(MultilayerPerceptron* in_mlp, uint32_t in_minibatchsize);

		~BackPropagation();


		void SetTrainingConfig(const TrainingConfig&);
		void SetActivationFunction(uint32_t in_layer_index, ActivationFunction_t in_func);
		void SetNoisy(uint32_t in_layer_index, bool in_noisy);
		void SetInputDropoutProbability(uint32_t in_layer_index, float in_prob);

		float Train(OpenGLBuffer2D& example_input, OpenGLBuffer2D& example_label);

		uint32_t LayerCount() const {return _layers.size();}



		MultilayerPerceptron* GetMultilayerPerceptron() const;
		MultilayerPerceptron* GetMultilayerPerceptron(uint32_t begin_layer, uint32_t end_layer) const;

	private:

		struct Layer
		{
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

		

		const uint32_t _input_units;
		const uint32_t _minibatch_size;
		TrainingConfig _training_config;
		
		// flag gets set when training config gets updated
		bool _recompile_required;

		vector<Layer*> _layers;

		// kernel source definitions
#		include "BackPropagationKernels.h"

		// recomplie kernel programs as necessary when parameters change
		void free_kernels();
		void build_kernels();
		void build_layer(LayerConfig in_Config, float* in_weights);
	};

	typedef BackPropagation BP;
}
