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

			uint32_t LayerCount;
			float* Dropout;

			TrainingConfig()
				: LearningRate(0.0f)
				, Momentum(0.0f)
				, L1Regularization(0.0f)
				, L2Regularization(0.0f)
				, Dropout(nullptr)
				, LayerCount(0)
			{ }

			TrainingConfig(const TrainingConfig& in_config)
			{
				*this = in_config;
			}

			TrainingConfig& operator=(const TrainingConfig& in_config)
			{
				LearningRate = in_config.LearningRate;
				Momentum = in_config.Momentum;
				L1Regularization = in_config.L1Regularization;
				L2Regularization = in_config.L2Regularization;
				LayerCount = in_config.LayerCount;
				Dropout =  new float[LayerCount];
				for(uint32_t k = 0; k < LayerCount; k++)
				{
					Dropout[k] = in_config.Dropout[k];
				}

				return *this;
			}

			void Initialize(uint32_t layer_count)
			{
				LayerCount = layer_count;
				Dropout =  new float[LayerCount];
				for(uint32_t k = 0; k < LayerCount; k++)
				{
					Dropout[k] = 0.0f;
				}

			}

			~TrainingConfig()
			{
				delete[] Dropout;
				Dropout = nullptr;
			}
		};

		
		BackPropagation(const ModelConfig, uint32_t in_minibatchsize);
		BackPropagation(MultilayerPerceptron* in_mlp, uint32_t in_minibatchsize);

		~BackPropagation();

		void SetTrainingConfig(const TrainingConfig&);
		void SetActivationFunction(uint32_t in_layer_index, ActivationFunction_t in_func);
		void SetNoisy(uint32_t in_layer_index, bool in_noisy);

		void Train(OpenGLBuffer2D& example_input, OpenGLBuffer2D& example_label);

		float GetLastOutputError();
		float GetOutputError(OpenGLBuffer2D& example_input, OpenGLBuffer2D& example_output);

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
			bool Noisy;

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
		OpenGLBuffer2D* _last_label;

		// buffer and program for dropping out input units
		OpenGLBuffer2D _input_buffer;
		OpenGLProgram* _copy_visible;

		// used for error calculations
		AlignedMemoryBlock<float> _output_buffer0;
		AlignedMemoryBlock<float> _output_buffer1;

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
