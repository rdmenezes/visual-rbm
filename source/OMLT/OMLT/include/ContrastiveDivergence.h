#pragma once

// std
#include <stdint.h>

// sickl header
#include <SiCKL.h>

// OMLT
#include "Enums.h"
#include "Common.h"
#include "SiCKLShared.h"


using namespace SiCKL;

namespace OMLT
{
	class RestrictedBoltzmannMachine;
	class ContrastiveDivergence
	{
	public:
		struct ModelConfig
		{
			uint32_t VisibleUnits;
			ActivationFunction_t VisibleType;
			uint32_t HiddenUnits;
			ActivationFunction_t HiddenType;

			ModelConfig()
				: VisibleUnits(0)
				, VisibleType(ActivationFunction::Invalid)
				, HiddenUnits(0)
				, HiddenType(ActivationFunction::Invalid)
			{ }

			bool operator==(const ModelConfig& that)
			{
				return this->VisibleUnits == that.VisibleUnits &&
					   this->VisibleType == that.VisibleType &&
					   this->HiddenUnits == that.HiddenUnits &&
					   this->HiddenType == that.HiddenType;
			}

		};

		struct TrainingConfig
		{
			float LearningRate;
			float Momentum;
			float L1Regularization;
			float L2Regularization;
			float VisibleDropout;
			float HiddenDropout;
			float AdadeltaDecay;

			TrainingConfig() 
				: LearningRate(0.0f)
				, Momentum(0.0f)
				, L1Regularization(0.0f)
				, L2Regularization(0.0f)
				, VisibleDropout(0.0f)
				, HiddenDropout(0.0f)
				, AdadeltaDecay(1.0f)
			{ }
		};

		ContrastiveDivergence(const ModelConfig, uint32_t in_minibatch_size, int32_t in_seed);
		ContrastiveDivergence(RestrictedBoltzmannMachine* in_rbm, uint32_t in_minibatch_size, int32_t in_seed);
		~ContrastiveDivergence();

		void SetTrainingConfig(const TrainingConfig&);
		ModelConfig GetModelConfig() const {return _model_config;};

		void Train(const OpenGLBuffer2D&);

		float GetLastReconstructionError();
		float GetReconstructionError(const OpenGLBuffer2D&);

		// get a new RBM object dumped from GPU memory
		RestrictedBoltzmannMachine* GetRestrictedBoltzmannMachine() const;

		/// these methods are used by VisualRBM for real-time visualization
		bool DumpLastVisible(float** image, float** recon);
		bool DumpLastHidden(float** activations);
		bool DumpLastWeights(float** weights);

	private:
		uint32_t _minibatch_size;
		ModelConfig _model_config;
		TrainingConfig _training_config;
		// we need to recompile the shaders if the training config changes
		bool _recompile_required;

		// texture buffers
		OpenGLBuffer2D _visible_dropout_random0;
		OpenGLBuffer2D _visible_dropout_random1;
		OpenGLBuffer2D _hidden_dropout_random0;
		OpenGLBuffer2D _hidden_dropout_random1;
		OpenGLBuffer2D _enabled_visible;
		OpenGLBuffer2D _enabled_hidden;

		OpenGLBuffer2D _visible0;
		OpenGLBuffer2D _visible1;
		OpenGLBuffer2D _hidden0;
		OpenGLBuffer2D _hidden1;
		OpenGLBuffer2D _hidden_states;
		OpenGLBuffer2D _hidden_random0;
		OpenGLBuffer2D _hidden_random1;
		OpenGLBuffer2D _visible_prime0;
		OpenGLBuffer2D _visible_prime1;
		OpenGLBuffer2D _hidden_prime0;
		OpenGLBuffer2D _hidden_prime1;

		OpenGLBuffer2D _weights0;
		OpenGLBuffer2D _weights1;
		OpenGLBuffer2D _delta_weights0;
		OpenGLBuffer2D _delta_weights1;
		OpenGLBuffer2D _nesterov_weight;
		OpenGLBuffer2D _mean_square_delta0;
		OpenGLBuffer2D _mean_square_delta1;

		OpenGLProgram* _calc_enabled_visible;
		OpenGLProgram* _calc_enabled_hidden;
		OpenGLProgram* _calc_hidden_states;
		OpenGLProgram* _calc_hidden_softmax_states;
		OpenGLProgram* _calc_visible;
		OpenGLProgram* _calc_visible_softmax;
		OpenGLProgram* _calc_hidden;
		OpenGLProgram* _calc_hidden_softmax;
		OpenGLProgram* _update_weights;

		ErrorCalculator* _error_calculator;

		// kernel sources definitions
#		include "ContrastiveDivergenceKernels.h"
		// recompile kernel programs as necessary
		void free_kernels();
		void build_kernels();
		void allocate_textures(float* weight_buffer, int32_t seed);
	};

	typedef ContrastiveDivergence CD;
}