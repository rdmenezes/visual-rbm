#pragma once

// std
#include <stdint.h>
#include <assert.h>

// SiCKL
#include <SiCKL.h>
using namespace SiCKL;

// OMLT
#include "Enums.h"
#include "AutoEncoder.h"
#include "SiCKLShared.h"
#include "Common.h"

namespace OMLT
{
	class AutoEncoderBackPropagation
	{
	public:
		struct ModelConfig
		{
			uint32_t VisibleCount;
			uint32_t HiddenCount;
			ActivationFunction_t HiddenType;
			ActivationFunction_t OutputType;
		};

		struct TrainingConfig
		{
			float LearningRate;
			float Momentum;
			float L1Regularization;
			float L2Regularization;
			float VisibleDropout;
			float HiddenDropout;

			TrainingConfig() 
				: LearningRate(0.0f)
				, Momentum(0.0f)
				, L1Regularization(0.0f)
				, L2Regularization(0.0f)
				, VisibleDropout(0.0f)
				, HiddenDropout(0.0f)
			{ }
		};
		AutoEncoderBackPropagation(const ModelConfig&, uint32_t in_minibatch_size);
		AutoEncoderBackPropagation(const AutoEncoder* in_autoencoder, uint32_t in_minibatch_size);
		~AutoEncoderBackPropagation();

		void SetTrainingConfig(const TrainingConfig&);

		void Train(const OpenGLBuffer2D&);

		/// get our trained model
		AutoEncoder* GetAutoEncoder() const;

		/// these methods are used by VisualRBM for real-time visualization
		bool DumpLastVisible(float** image, float** recon);
		bool DumpLastHidden(float** activations);
		bool DumpLastWeights(float** weights);

		float GetLastError();
		float GetError(const OpenGLBuffer2D&);
	private:
		uint32_t _minibatch_size;
		ModelConfig _model_config;
		TrainingConfig _training_config;
		// we need to recompile the shaders if the training config changes
		bool _recompile_required;

		// kernel source defs
#		include "AutoEncoderBackPropagationKernels.h"

		/** Texture Buffers **/
		 
		// dropout related buffers
		OpenGLBuffer2D VisibleRandom0;
		OpenGLBuffer2D VisibleRandom1;
		OpenGLBuffer2D VisibleEnabled;
		OpenGLBuffer2D HiddenRandom0;
		OpenGLBuffer2D HiddenRandom1;
		OpenGLBuffer2D HiddenEnabled;

		// weight buffers
		OpenGLBuffer2D Weights0;
		OpenGLBuffer2D Weights1;
		OpenGLBuffer2D DeltaWeights0;
		OpenGLBuffer2D DeltaWeights1;

		// layer buffers
		OpenGLBuffer2D Target;
		OpenGLBuffer2D Visible;
		OpenGLBuffer2D Hidden0;
		OpenGLBuffer2D Hidden1;
		OpenGLBuffer2D Output0;
		OpenGLBuffer2D Output1;

		// sensitivities
		OpenGLBuffer2D HiddenSensitivities;
		OpenGLBuffer2D OutputSensitivities;

		OpenGLProgram* CalcEnabledVisible;
		OpenGLProgram* CalcEnabledHidden;
		OpenGLProgram* CopyVisible;
		OpenGLProgram* CalcHidden;
		OpenGLProgram* CalcHiddenSoftmax;
		OpenGLProgram* CalcOutput;
		OpenGLProgram* CalcOutputSoftmax;
		OpenGLProgram* CalcOutputSensitivities;
		OpenGLProgram* CalcHiddenSensitivities;
		OpenGLProgram* UpdateWeights;

		ErrorCalculator* _error_calculator;
		
		void free_kernels();
		void build_kernels();
		void allocate_textures(float* weight_buffer);
	};
}