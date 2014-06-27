#pragma once

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
#include "SiCKLShared.h"
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
		};

		struct ModelConfig
		{
			uint32_t InputCount;
			std::vector<LayerConfig> LayerConfigs;
		};

		struct LayerParameters
		{
			union
			{
				struct
				{
					float LearningRate;
					float Momentum;
					float L1Regularization;
					float L2Regularization;
					float Dropout;
					float Noise;
					float AdadeltaDecay;
				};
				float Data[7];
			};
			LayerParameters() 
				: LearningRate(0.0f)
				, Momentum(0.0f)
				, L1Regularization(0.0f)
				, L2Regularization(0.0f)
				, Dropout(0.0f)
				, Noise(0.0f)
				, AdadeltaDecay(1.0f)
			{ }
				
		};

		// config for a nn trainer
		struct TrainingConfig
		{
			std::vector<LayerParameters> Parameters;
		};

		
		BackPropagation(const ModelConfig, uint32_t in_minibatchsize);
		BackPropagation(MultilayerPerceptron* in_mlp, uint32_t in_minibatchsize);

		~BackPropagation();

		void SetTrainingConfig(const TrainingConfig&);

		void Train(const OpenGLBuffer2D& example_input, const OpenGLBuffer2D& example_label);

		float GetLastOutputError();
		float GetOutputError(const OpenGLBuffer2D& example_input, const OpenGLBuffer2D& example_output);

		uint32_t LayerCount() const {return _layers.size();}

		MultilayerPerceptron* GetMultilayerPerceptron() const;
		MultilayerPerceptron* GetMultilayerPerceptron(uint32_t begin_layer, uint32_t end_layer) const;

		bool DumpLastLabel(float** label);
		bool DumpInput(uint32_t layer, float** input);
		bool DumpActivation(uint32_t layer, float** output);
		bool DumpWeightMatrix(uint32_t layer, float** weights);

	private:

		struct Layer
		{
			Layer* NextLayer;

			uint32_t InputUnits;
			uint32_t OutputUnits;

			ActivationFunction_t Function;

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
			OpenGLBuffer2D NesterovWeight;
			OpenGLBuffer2D DeltaWeights0;
			OpenGLBuffer2D DeltaWeights1;
			OpenGLBuffer2D MeanSquareDelta0;
			OpenGLBuffer2D MeanSquareDelta1;
			OpenGLBuffer2D* OutputEnabled;

			/*
			 * Output associated data
			 */
			OpenGLBuffer2D Activation0;
			OpenGLBuffer2D Activation1;
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
			OpenGLProgram* CalcSoftmax;
			OpenGLProgram* CalcSensitivity;
			OpenGLProgram* UpdateWeights;
		};
		const OpenGLBuffer2D* _last_label;

		// buffer and program for dropping out input units
		OpenGLBuffer2D _input_buffer;
		OpenGLProgram* _copy_visible;

		ErrorCalculator* _error_calculator;

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
