// stdlib
#include <string.h>
#include <algorithm>
using std::swap;
#include <random>


// OMLT
#include "Common.h"
#include "ContrastiveDivergence.h"
#include "RestrictedBoltzmannMachine.h"

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
		FreeKernels();
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
		if(_recompile_required)
		{
			FreeKernels();
			BuildKernels();

			_recompile_required = false;
		}

		/// Calculate Enabled Units

		_calc_enabled_visible->SetInput(0, _visible_dropout_random0);
		_calc_enabled_visible->BindOutput(0, _enabled_visible);
		_calc_enabled_visible->BindOutput(1, _visible_dropout_random1);
		_calc_enabled_visible->Run();

		_calc_enabled_hidden->SetInput(0, _hidden_dropout_random0);
		_calc_enabled_hidden->BindOutput(0, _enabled_hidden);
		_calc_enabled_hidden->BindOutput(1, _hidden_dropout_random1);
		_calc_enabled_hidden->Run();

		// ping pong seed buffers
		swap(_visible_dropout_random0, _visible_dropout_random1);
		swap(_hidden_dropout_random0, _hidden_dropout_random1);

		/// Copy in Visible Data

		_copy_visible->SetInput(0, in_example);
		_copy_visible->SetInput(1, _enabled_visible);
		_copy_visible->BindOutput(0, _visible);
		_copy_visible->Run();
		
		/// Calc Hidden and States from Visible

		_calc_hidden_states->SetInput(0, _visible);
		_calc_hidden_states->SetInput(1, _weights0);
		_calc_hidden_states->SetInput(2, _enabled_hidden);
		_calc_hidden_states->SetInput(3, _hidden_random0);
		_calc_hidden_states->BindOutput(0, _hidden_random1);
		_calc_hidden_states->BindOutput(1, _hidden);
		_calc_hidden_states->BindOutput(2, _hidden_states);
		_calc_hidden_states->Run();

		swap(_hidden_random0, _hidden_random1);

		/// Calc Visible Prime

		_calc_visible->SetInput(0, _hidden_states);
		_calc_visible->SetInput(1, _weights0);
		_calc_visible->SetInput(2, _enabled_visible);
		_calc_visible->BindOutput(0, _visible_prime);
		_calc_visible->Run();

		/// Calc Hidden Prime

		_calc_hidden->SetInput(0, _visible_prime);
		_calc_hidden->SetInput(1, _weights0);
		_calc_hidden->SetInput(2, _enabled_hidden);
		_calc_hidden->BindOutput(0, _hidden_prime);
		_calc_hidden->Run();

		/// Update Weights

		_update_weights->SetInput(0, _visible);
		_update_weights->SetInput(1, _hidden);
		_update_weights->SetInput(2, _visible_prime);
		_update_weights->SetInput(3, _hidden_prime);
		_update_weights->SetInput(4, _delta_weights0);
		_update_weights->SetInput(5, _weights0);
		_update_weights->SetInput(6, _enabled_visible);
		_update_weights->SetInput(7, _enabled_hidden);
		_update_weights->BindOutput(0, _delta_weights1);
		_update_weights->BindOutput(1, _weights1);
		_update_weights->Run();

		swap(_delta_weights0, _delta_weights1);
		swap(_weights0, _weights1);

		/// Done!

	}

	float TotalError(float* buffer, uint32_t count, uint32_t height)
	{
		float result = 0.0f;
		for(uint32_t k = 0; k < count; k++)
		{
			result += buffer[k];
		}

		return result / float(count * height);
	}

	float ContrastiveDivergence::GetLastReconstructionError()
	{
		/// Calc error

		_calc_error->SetInput(0, _visible);
		_calc_error->SetInput(1, _visible_prime);
		_calc_error->BindOutput(0, _error);
		_calc_error->Run();

		float* ptr = _error_buffer;
		_calc_error->GetOutput(0, ptr);

		return TotalError(ptr, _model_config.VisibleUnits, _model_config.MinibatchSize);
	}

	float ContrastiveDivergence::GetReconstructionError( const OpenGLBuffer2D& in_example)
	{
		/// Copy example in and dropout data (using precalculated enabled visibles)

		_copy_visible->SetInput(0, in_example);
		_copy_visible->SetInput(1, _enabled_visible);
		_copy_visible->BindOutput(0, _visible);
		_copy_visible->Run();

		/// Calc Hidden

		_calc_hidden->SetInput(0, _visible);
		_calc_hidden->SetInput(1, _weights0);
		_calc_hidden->SetInput(2, _enabled_hidden);
		_calc_hidden->BindOutput(0, _hidden);
		_calc_hidden->Run();

		/// Calc Visible Prime

		_calc_visible->SetInput(0, _hidden);
		_calc_visible->SetInput(1, _weights0);
		_calc_visible->SetInput(2, _enabled_visible);
		_calc_visible->BindOutput(0, _visible_prime);
		_calc_visible->Run();

		/// Finally Calc error

		_calc_error->SetInput(0, _visible);
		_calc_error->SetInput(1, _visible_prime);
		_calc_error->BindOutput(0, _error);
		_calc_error->Run();

		float* ptr = _error_buffer;
		_calc_error->GetOutput(0, ptr);

		return TotalError(ptr, _model_config.VisibleUnits, _model_config.MinibatchSize);
	}

	RestrictedBoltzmannMachine* ContrastiveDivergence::GetRestrictedBoltzmannMachine() const
	{
		RestrictedBoltzmannMachine* rbm = new RestrictedBoltzmannMachine(_model_config.VisibleUnits, _model_config.HiddenUnits, _model_config.VisibleType, _model_config.HiddenType);	
		
		uint32_t weight_count = (_model_config.VisibleUnits + 1) * (_model_config.HiddenUnits + 1);
		float* raw_weights = new float[weight_count];

		// pull weights from GPU
		_weights0.GetData(raw_weights);

		// fill in our RBM object
		for(uint32_t i = 0; i <= _model_config.VisibleUnits; i++)
		{
			for(uint32_t j = 0; j <= _model_config.HiddenUnits; j++)
			{
				uint32_t index = i * (_model_config.HiddenUnits + 1) + j;
				float& val = raw_weights[index];
				if(i == 0 && j == 0)
				{
					continue;
				}
				else if(i == 0)
				{
					// hidden bias
					rbm->hidden_biases[j-1] = val;
				}
				else if(j == 0)
				{
					// visible bias
					rbm->visible_biases[i-1] = val;
				}
				else
				{
					// regular weight
					rbm->visible_features[i-1][j-1] = val;
					rbm->hidden_features[j-1][i-1] = val;
				}
			}
		}

		return rbm;
	}

	bool ContrastiveDivergence::DumpLastVisible( float* image, float* recon )
	{
		return false;
	}

	bool ContrastiveDivergence::DumpLastHidden( float* activations )
	{
		return false;
	}

	bool ContrastiveDivergence::DumpLastWeights( float* weights )
	{
		return false;
	}

	void ContrastiveDivergence::FreeKernels()
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

		//printf("%s\n", _calc_enabled_visible->GetSource().c_str());

		/// Calc Enabled Hidden
		SourceCalcEnabled src_calc_enabled_hidden;
		src_calc_enabled_hidden.DROPOUT_PROB = _training_config.HiddenDropout;
		src_calc_enabled_hidden.Parse();

		_calc_enabled_hidden = compiler.Build(src_calc_enabled_hidden);
		_calc_enabled_hidden->Initialize(_model_config.HiddenUnits, 1);

		//printf("%s\n", _calc_enabled_hidden->GetSource().c_str());

		/// Copy Visible
		SourceCopyVisible src_copy_visible;
		src_copy_visible.Parse();

		_copy_visible = compiler.Build(src_copy_visible);
		_copy_visible->Initialize(_model_config.VisibleUnits, _model_config.MinibatchSize);

		//printf("%s\n", _copy_visible->GetSource().c_str());

		/// Calc Hidden And States
		SourceCalcHiddenAndStates src_calc_hidden_and_states;
		src_calc_hidden_and_states.FUNCTION = _model_config.HiddenType;
		src_calc_hidden_and_states.VISIBLE_UNITS = _model_config.VisibleUnits;
		src_calc_hidden_and_states.VISIBLE_DROPOUT_PROB = _training_config.VisibleDropout;
		src_calc_hidden_and_states.Parse();

		_calc_hidden_states = compiler.Build(src_calc_hidden_and_states);
		_calc_hidden_states->Initialize(_model_config.HiddenUnits, _model_config.MinibatchSize);

		//printf("%s\n", _calc_hidden_states->GetSource().c_str());

		/// Calc Visible
		SourceCalcVisible src_calc_visible;
		src_calc_visible.FUNCTION = _model_config.VisibleType;
		src_calc_visible.HIDDEN_UNITS = _model_config.HiddenUnits;
		src_calc_visible.HIDDEN_DROPOUT_PROB = _training_config.HiddenDropout;
		src_calc_visible.Parse();

		_calc_visible = compiler.Build(src_calc_visible);
		_calc_visible->Initialize(_model_config.VisibleUnits, _model_config.MinibatchSize);

		//printf("%s\n", _calc_visible->GetSource().c_str());

		/// Calc Hidden
		SourceCalcHidden src_calc_hidden;
		src_calc_hidden.FUNCTION = _model_config.HiddenType;
		src_calc_hidden.VISIBLE_UNITS = _model_config.VisibleUnits;
		src_calc_hidden.VISIBLE_DROPOUT_PROB = _training_config.VisibleDropout;
		src_calc_hidden.Parse();

		_calc_hidden = compiler.Build(src_calc_hidden);	
		_calc_hidden->Initialize(_model_config.HiddenUnits, _model_config.MinibatchSize);

		//printf("%s\n", _calc_hidden->GetSource().c_str());

		/// Update Weights
		SourceCalcWeightUpdates src_update_weights;
		src_update_weights.MINIBATCH_SIZE = _model_config.MinibatchSize;
		src_update_weights.LEARNING_RATE = _training_config.LearningRate;
		src_update_weights.MOMENTUM = _training_config.Momentum;
		src_update_weights.L1_REGULARIZATION = _training_config.L1Regularization;
		src_update_weights.L2_REGULARIZATION = _training_config.L2Regularization;
		src_update_weights.Parse();

		_update_weights = compiler.Build(src_update_weights);
		_update_weights->Initialize(_model_config.HiddenUnits + 1, _model_config.VisibleUnits + 1);

		//printf("%s\n", _update_weights->GetSource().c_str());

		/// Calc Error
		SourceCalcErrorVector src_calc_error;
		src_calc_error.MINIBATCH_SIZE = _model_config.MinibatchSize;
		src_calc_error.Parse();

		_calc_error = compiler.Build(src_calc_error);
		_calc_error->Initialize(_model_config.VisibleUnits, 1);

		//printf("%s\n", _calc_error->GetSource().c_str());
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

		// allocate a weight buffer and copy it to buffer
		if(weight_buffer == nullptr)
		{
			// initialize weights to random values 
			uint32_t weight_count = (_model_config.VisibleUnits + 1) * (_model_config.HiddenUnits + 1);
			float* weight_buffer = (float*)malloc(sizeof(float) * weight_count);

			float weight_stdev = float(1.0 / std::sqrtf((float)(_model_config.VisibleUnits + _model_config.HiddenUnits)));
			std::normal_distribution<float> normal(0.0f, weight_stdev);

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
						weight_buffer[ index ] = normal(random);
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

		_error = OpenGLBuffer2D(_model_config.VisibleUnits, 1, ReturnType::Float, nullptr);

		// free freshly allocated seed buffers
		free(visible_dropout_seed_buffer);
		free(hidden_dropout_seed_buffer);
		free(hidden_seed_buffer);

		// acquire memory for our error buffer 
		_error_buffer.Acquire(_model_config.VisibleUnits);
	}
}