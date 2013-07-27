// stdlib
#include <string.h>
#include <algorithm>
#include <random>


// OMLT
#include "ContrastiveDivergence.h"
#include "Common.h"

namespace OMLT
{
	ContrastiveDivergence::ContrastiveDivergence( const ModelConfig in_config)
		: _model_config(in_config)
		, _calc_enabled_visible(nullptr)
		, _calc_enabled_hidden(nullptr)
		, _copy_visible(nullptr)
		, _calc_hidden(nullptr)
		, _calc_hidden_states(nullptr)
		, _calc_visible(nullptr)
		, _update_weights(nullptr)
		, _calc_error(nullptr)
		, _recompile_required(true)
	{
		AllocateTextures(nullptr);
	}

	ContrastiveDivergence::~ContrastiveDivergence()
	{
		DeleteKernels();
	}

	void ContrastiveDivergence::SetTrainingConfig( const TrainingConfig& in_config)
	{
		// only set recompile flag if the new config is different
		if(memcmp(&in_config, &_training_config, sizeof(TrainingConfig)) != 0)
		{
			_training_config = in_config;
			_recompile_required = true;
		}
	}

	void ContrastiveDivergence::Train( const OpenGLBuffer2D& in_example)
	{
		
	}

	float ContrastiveDivergence::GetLastReconstructionError()
	{

	}

	float ContrastiveDivergence::GetReconstructionError( const OpenGLBuffer2D& )
	{

	}

	RestrictedBoltzmannMachine* ContrastiveDivergence::GetRestrictedBoltzmannMachine() const
	{

	}

	bool ContrastiveDivergence::DumpLastVisible( float* image, float* recon )
	{

	}

	bool ContrastiveDivergence::DumpLastHidden( float* activations )
	{

	}

	bool ContrastiveDivergence::DumpLastWeights( float* weights )
	{

	}

	void ContrastiveDivergence::DeleteKernels()
	{
		// delete our kernels
		SafeDelete(_calc_enabled_visible);
		SafeDelete(_calc_enabled_hidden);
		SafeDelete(_copy_visible);
		SafeDelete(_calc_hidden);
		SafeDelete(_calc_hidden_states);
		SafeDelete(_calc_visible);
		SafeDelete(_update_weights);
		SafeDelete(_calc_error);
	}

	void ContrastiveDivergence::BuildKernels()
	{
		OpenGLCompiler compiler;

		/// Calc Enabled Visible
		SourceCalcEnabled src_calc_enabled_visible;
		src_calc_enabled_visible.DROPOUT_PROB = _training_config.VisibleDropout;
		src_calc_enabled_visible.Parse();
		
		_calc_enabled_visible = compiler.Build(src_calc_enabled_visible);
		_calc_enabled_visible->Initialize(_model_config.VisibleUnits, 1);

		/// Calc Enabled Hidden
		SourceCalcEnabled src_calc_enabled_hidden;
		src_calc_enabled_hidden.DROPOUT_PROB = _training_config.HiddenDropout;
		src_calc_enabled_hidden.Parse();

		_calc_enabled_visible = compiler.Build(src_calc_enabled_hidden);
		_calc_enabled_visible->Initialize(_model_config.HiddenUnits, 1);

		/// Copy Visible
		SourceCopyVisible src_copy_visible;
		src_copy_visible.Parse();

		_copy_visible = compiler.Build(src_copy_visible);
		_copy_visible->Initialize(_model_config.VisibleUnits, _model_config.MinibatchSize);

		/// Calc Hidden
		SourceCalcHidden src_calc_hidden;
		src_calc_hidden.VISIBLE_UNITS = _model_config.VisibleUnits;
		src_calc_hidden.Parse();

		_calc_hidden = compiler.Build(src_calc_hidden);	

		/// Calc Hidden States
		SourceCalcStates src_calc_states;
		src_calc_states.Parse();

		_calc_hidden_states = compiler.Build(src_calc_states);

		/// Calc Visible
		SourceCalcVisible src_calc_visible;
		src_calc_visible.USE_SIGMOID = true;
		src_calc_visible.HIDDEN_UNITS = _model_config.HiddenUnits;
		src_calc_visible.Parse();

		_calc_visible = compiler.Build(src_calc_visible);

		/// Update Weights
		SourceCalcWeightUpdates src_update_weights;
		src_update_weights.MINIBATCH_SIZE = _model_config.MinibatchSize;
		src_update_weights.LEARNING_RATE = _training_config.LearningRate;
		src_update_weights.MOMENTUM = _training_config.Momentum;
		src_update_weights.L1_REGULARIZATION = _training_config.L1Regularization;
		src_update_weights.L2_REGULARIZATION = _training_config.L2Regularization;
		src_update_weights.Parse();

		_update_weights = compiler.Build(src_update_weights);
		
		/// Calc Error
		SourceCalcErrorVector src_calc_error;
		src_calc_error.MINIBATCH_SIZE = _model_config.MinibatchSize;
		src_calc_error.Parse();

		_calc_error = compiler.Build(src_calc_error);
	}

	// free result when done
	uint32_t* GetSeedBuffer(uint32_t in_width, uint32_t in_height, std::mt19937_64& random)
	{
		// full range uniform dist
		std::uniform_int_distribution<uint32_t> uniform(0, 0xFFFFFFFF);

		uint32_t seed_count = in_width * in_height;
		uint32_t* result = (uint32_t*)malloc(seed_count * sizeof(uint32_t));

		for(uint32_t k = 0; k < seed_count; k++)
		{
			result[k] = uniform(random);
		}


		return result;
	}

	void ContrastiveDivergence::AllocateTextures(float* weight_buffer)
	{	
		std::mt19937_64 random;
		random.seed(1);

		uint32_t* visible_dropout_seed_buffer = GetSeedBuffer(_model_config.VisibleUnits, 1, random);
		uint32_t* hidden_dropout_seed_buffer = GetSeedBuffer(_model_config.HiddenUnits, 1, random);
		uint32_t* hidden_seed_buffer = GetSeedBuffer(_model_config.HiddenUnits, _model_config.MinibatchSize, random);

		_visible_dropout_random0 = OpenGLBuffer2D(_model_config.VisibleUnits, 1, ReturnType::UInt, nullptr);
		_visible_dropout_random1 = OpenGLBuffer2D(_model_config.VisibleUnits, 1, ReturnType::UInt, nullptr);
		_hidden_dropout_random0 = OpenGLBuffer2D(_model_config.HiddenUnits, 1, ReturnType::UInt, nullptr);
		_hidden_dropout_random1 = OpenGLBuffer2D(_model_config.HiddenUnits, 1, ReturnType::UInt, nullptr);

		_enabled_visible = OpenGLBuffer2D(_model_config.VisibleUnits, 1, ReturnType::UInt, nullptr);
		_enabled_hidden = OpenGLBuffer2D(_model_config.HiddenUnits, 1, ReturnType::UInt, nullptr);

		_visible = OpenGLBuffer2D(_model_config.VisibleUnits, _model_config.MinibatchSize, ReturnType::Float, nullptr);
		_hidden = OpenGLBuffer2D(_model_config.HiddenUnits, _model_config.MinibatchSize, ReturnType::Float, nullptr);
		_hidden_random0 = OpenGLBuffer2D(_model_config.HiddenUnits, _model_config.MinibatchSize, ReturnType::UInt, nullptr);
		_hidden_random1 = OpenGLBuffer2D(_model_config.HiddenUnits, _model_config.MinibatchSize, ReturnType::UInt, nullptr);
		_hidden_states = OpenGLBuffer2D(_model_config.HiddenUnits, _model_config.MinibatchSize, ReturnType::Float, nullptr);
		_visible_prime = OpenGLBuffer2D(_model_config.VisibleUnits, _model_config.MinibatchSize, ReturnType::Float, nullptr);
		_hidden_prime = OpenGLBuffer2D(_model_config.HiddenUnits, _model_config.MinibatchSize, ReturnType::Float, nullptr);

		if(weight_buffer == nullptr)
		{
			// initialize weights to random values 
			uint32_t weight_count = (_model_config.VisibleUnits + 1) * (_model_config.HiddenUnits + 1);
			float* weight_buffer = (float*)malloc(sizeof(float) * weight_count);

			float extent = 1.0f / std::sqrtf(_model_config.VisibleUnits);
			std::uniform_real_distribution<float> uniform(-extent, extent);

			for(uint32_t i = 0; i <= _model_config.VisibleUnits; i++)
			{
				for(uint32_t j = 0; j <= _model_config.HiddenUnits; j++)
				{
					uint32_t index = i * (_model_config.HiddenUnits + 1) + j;
					if(i == 0 || j == 0)
					{
						weight_buffer[ index ] = 0.0f;
					}
					else
					{
						weight_buffer[ index ] = uniform(random);
					}
				}
			}
			_weights0 = OpenGLBuffer2D(_model_config.HiddenUnits + 1, _model_config.VisibleUnits + 1, ReturnType::Float, weight_buffer);
			free(weight_buffer);
		}
		else
		{
			// just use weights received from model
			_weights0 = OpenGLBuffer2D(_model_config.HiddenUnits + 1, _model_config.VisibleUnits + 1, ReturnType::Float, weight_buffer);
		}
		_weights1 = OpenGLBuffer2D(_model_config.HiddenUnits + 1, _model_config.VisibleUnits + 1, ReturnType::Float, nullptr);
		_delta_weights0 = OpenGLBuffer2D(_model_config.HiddenUnits + 1, _model_config.VisibleUnits + 1, ReturnType::Float, nullptr);
		_delta_weights1 = OpenGLBuffer2D(_model_config.HiddenUnits + 1, _model_config.VisibleUnits + 1, ReturnType::Float, nullptr);

		free(visible_dropout_seed_buffer);
		free(hidden_dropout_seed_buffer);
		free(hidden_seed_buffer);
	}
}