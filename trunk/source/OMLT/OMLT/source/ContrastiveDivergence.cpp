// windows
#include <windows.h>

// std
#include <string.h>
#include <algorithm>
using std::swap;
#include <random>
#include <assert.h>

// OMLT
#include "Common.h"
#include "ContrastiveDivergence.h"
#include "RestrictedBoltzmannMachine.h"

namespace OMLT
{
	ContrastiveDivergence::ContrastiveDivergence( const ModelConfig in_config, uint32_t in_minibatch_size, int32_t in_seed)
		: _model_config(in_config)
		, _minibatch_size(in_minibatch_size)
		, _calc_enabled_visible(nullptr)
		, _calc_enabled_hidden(nullptr)
		, _calc_hidden(nullptr)
		, _calc_hidden_softmax(nullptr)
		, _calc_hidden_states(nullptr)
		, _calc_hidden_softmax_states(nullptr)
		, _calc_visible(nullptr)
		, _calc_visible_softmax(nullptr)
		, _update_weights(nullptr)
		, _error_calculator(nullptr)
		, _recompile_required(true)
	{
		allocate_textures(nullptr, in_seed);
	}

	ContrastiveDivergence::ContrastiveDivergence( RestrictedBoltzmannMachine* in_rbm, uint32_t in_minibatch_size, int32_t in_seed )
		: _minibatch_size(in_minibatch_size)
		, _calc_enabled_visible(nullptr)
		, _calc_enabled_hidden(nullptr)
		, _calc_hidden(nullptr)
		, _calc_hidden_softmax(nullptr)
		, _calc_hidden_states(nullptr)
		, _calc_hidden_softmax_states(nullptr)
		, _calc_visible(nullptr)
		, _calc_visible_softmax(nullptr)
		, _update_weights(nullptr)
		, _error_calculator(nullptr)
		, _recompile_required(true)
	{
		assert(in_rbm != nullptr);
		_model_config.VisibleUnits = in_rbm->visible_count;
		_model_config.VisibleType = in_rbm->visible_type;
		_model_config.HiddenUnits = in_rbm->hidden_count;
		_model_config.HiddenType = in_rbm->hidden_type;

		// copy weights to a properly formatted buffer
		float* weight_buffer = new float[(in_rbm->hidden_count + 1) * (in_rbm->visible_count + 1)];
		weight_buffer[0] = 0.0f;

		// copy in visible biases
		memcpy(weight_buffer + 1, in_rbm->visible.biases(), sizeof(float) * in_rbm->visible_count);

		// now copy in each hidden feature (as well as hidden bias)
		for(uint32_t j = 1; j <= in_rbm->hidden_count; j++)
		{
			const uint32_t offset = (in_rbm->visible_count + 1) * j;
			// bias
			weight_buffer[offset] = in_rbm->hidden.biases()[j-1];
			// weight vector
			memcpy(weight_buffer + offset + 1, in_rbm->hidden.feature(j-1), sizeof(float) * in_rbm->visible_count);
		}

		allocate_textures(weight_buffer, in_seed);
		delete[] weight_buffer;
	}

	ContrastiveDivergence::~ContrastiveDivergence()
	{
		free_kernels();
		delete _error_calculator;
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
			free_kernels();
			build_kernels();

			_recompile_required = false;
		}

		_visible0 = in_example;

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

		/// Calc Hidden and States from Visible
		if(_model_config.HiddenType != ActivationFunction::Softmax)
		{
			_calc_hidden_states->SetInput(0, _visible0);
			_calc_hidden_states->SetInput(1, _nesterov_weight);
			_calc_hidden_states->SetInput(2, _enabled_visible);
			_calc_hidden_states->SetInput(3, _hidden_random0);
			_calc_hidden_states->BindOutput(0, _hidden_random1);
			_calc_hidden_states->BindOutput(1, _hidden0);
			_calc_hidden_states->BindOutput(2, _hidden_states);
			_calc_hidden_states->Run();

			swap(_hidden_random0, _hidden_random1);
		}
		/// Calc Hidden Softmax and States from Visible
		else
		{
			_calc_hidden->SetInput(0, _visible0);
			_calc_hidden->SetInput(1, _nesterov_weight);
			_calc_hidden->SetInput(2, _calc_enabled_visible);
			_calc_hidden->BindOutput(0, _hidden0);
			_calc_hidden->Run();

			_calc_hidden_softmax_states->SetInput(0, _hidden0);
			_calc_hidden_softmax_states->SetInput(1, _hidden_random0);
			_calc_hidden_softmax_states->BindOutput(0, _hidden_random1);
			_calc_hidden_softmax_states->BindOutput(1, _hidden1);
			_calc_hidden_softmax_states->BindOutput(2, _hidden_states);
			_calc_hidden_softmax_states->Run();

			swap(_hidden_random0, _hidden_random1);
			swap(_hidden0, _hidden1);
		}

		/// Calc Visible Prime

		_calc_visible->SetInput(0, _hidden_states);
		_calc_visible->SetInput(1, _nesterov_weight);
		_calc_visible->SetInput(2, _enabled_hidden);
		_calc_visible->BindOutput(0, _visible_prime0);
		_calc_visible->Run();

		/// Calc Visible Softmax

		if(_model_config.VisibleType == ActivationFunction::Softmax)
		{
			_calc_visible_softmax->SetInput(0, _visible_prime0);
			_calc_visible_softmax->BindOutput(0, _visible_prime1);
			_calc_visible_softmax->Run();

			swap(_visible_prime0, _visible_prime1);
		}

		/// Calc Hidden Prime

		_calc_hidden->SetInput(0, _visible_prime0);
		_calc_hidden->SetInput(1, _nesterov_weight);
		_calc_hidden->SetInput(2, _enabled_visible);
		_calc_hidden->BindOutput(0, _hidden_prime0);
		_calc_hidden->Run();

		/// Calc Hidden Softmax

		if(_model_config.HiddenType == ActivationFunction::Softmax)
		{
			_calc_hidden_softmax->SetInput(0, _hidden_prime0);
			_calc_hidden_softmax->BindOutput(0, _hidden_prime1);
			_calc_hidden_softmax->Run();

			swap(_hidden_prime0, _hidden_prime1);
		}

		/// Update Weights

		_update_weights->SetInput(0, _visible0);
		_update_weights->SetInput(1, _hidden0);
		_update_weights->SetInput(2, _visible_prime0);
		_update_weights->SetInput(3, _hidden_prime0);
		_update_weights->SetInput(4, _delta_weights0);
		_update_weights->SetInput(5, _weights0);
		_update_weights->SetInput(6, _enabled_visible);
		_update_weights->SetInput(7, _enabled_hidden);
		_update_weights->SetInput(8, _mean_square_delta0);
		_update_weights->BindOutput(0, _delta_weights1);
		_update_weights->BindOutput(1, _weights1);
		_update_weights->BindOutput(2, _mean_square_delta1);
		_update_weights->BindOutput(3, _nesterov_weight);
		_update_weights->Run();

		swap(_delta_weights0, _delta_weights1);
		swap(_weights0, _weights1);
		swap(_mean_square_delta0, _mean_square_delta1);

		/// Done!
	}

	float ContrastiveDivergence::GetLastReconstructionError()
	{
		return _error_calculator->CalcError(_visible0, _visible_prime0);
	}

	float ContrastiveDivergence::GetReconstructionError( const OpenGLBuffer2D& in_example)
	{
		if(_recompile_required)
		{
			free_kernels();
			build_kernels();

			_recompile_required = false;
		}

		OpenGLBuffer2D prev_visible0 = _visible0;
		_visible0 = in_example;

		/// Calc Hidden and States from Visible
		if(_model_config.HiddenType != ActivationFunction::Softmax)
		{
			_calc_hidden_states->SetInput(0, _visible0);
			_calc_hidden_states->SetInput(1, _nesterov_weight);
			_calc_hidden_states->SetInput(2, _enabled_visible);
			_calc_hidden_states->SetInput(3, _hidden_random0);
			_calc_hidden_states->BindOutput(0, _hidden_random1);
			_calc_hidden_states->BindOutput(1, _hidden0);
			_calc_hidden_states->BindOutput(2, _hidden_states);
			_calc_hidden_states->Run();

			swap(_hidden_random0, _hidden_random1);
		}
		/// Calc Hidden Softmax and States from Visible
		else
		{
			_calc_hidden->SetInput(0, _visible0);
			_calc_hidden->SetInput(1, _nesterov_weight);
			_calc_hidden->SetInput(2, _calc_enabled_visible);
			_calc_hidden->BindOutput(0, _hidden0);
			_calc_hidden->Run();

			_calc_hidden_softmax_states->SetInput(0, _hidden0);
			_calc_hidden_softmax_states->SetInput(1, _hidden_random0);
			_calc_hidden_softmax_states->BindOutput(0, _hidden_random1);
			_calc_hidden_softmax_states->BindOutput(1, _hidden1);
			_calc_hidden_softmax_states->BindOutput(2, _hidden_states);
			_calc_hidden_softmax_states->Run();

			swap(_hidden_random0, _hidden_random1);
			swap(_hidden0, _hidden1);
		}

		/// Calc Visible Prime

		_calc_visible->SetInput(0, _hidden_states);
		_calc_visible->SetInput(1, _nesterov_weight);
		_calc_visible->SetInput(2, _enabled_hidden);
		_calc_visible->BindOutput(0, _visible_prime0);
		_calc_visible->Run();

		/// Calc Visible Softmax

		if(_model_config.VisibleType == ActivationFunction::Softmax)
		{
			_calc_visible_softmax->SetInput(0, _visible_prime0);
			_calc_visible_softmax->BindOutput(0, _visible_prime1);
			_calc_visible_softmax->Run();

			swap(_visible_prime0, _visible_prime1);
		}

		float error = GetLastReconstructionError();

		_visible0 = prev_visible0;
		return error;
	}

	RestrictedBoltzmannMachine* ContrastiveDivergence::GetRestrictedBoltzmannMachine() const
	{
		RestrictedBoltzmannMachine* rbm = new RestrictedBoltzmannMachine(_model_config.VisibleUnits, _model_config.HiddenUnits, _model_config.VisibleType, _model_config.HiddenType);	
		
		uint32_t weight_count = (_model_config.VisibleUnits + 1) * (_model_config.HiddenUnits + 1);
		float* raw_weights = new float[weight_count];

		// pull weights from GPU
		_weights0.GetData(raw_weights);

		// fill in our RBM object
		uint32_t index = 0;
		for(uint32_t j = 0; j <= _model_config.HiddenUnits; j++)
		{
			for(uint32_t i = 0; i <= _model_config.VisibleUnits; i++)
			{
				float& val = raw_weights[index++];
				if(i == 0 && j == 0)
				{
					continue;
				}
				else if(i == 0)
				{
					// hidden bias
					rbm->hidden.biases()[j-1] = val;
				}
				else if(j == 0)
				{
					// visible bias
					rbm->visible.biases()[i - 1] = val;
				}
				else
				{
					// regular weight
					rbm->visible.feature(i-1)[j-1] = val;
					rbm->hidden.feature(j-1)[i-1] = val;
				}
			}
		}

		return rbm;
	}

	bool ContrastiveDivergence::DumpLastVisible( float** image, float** recon )
	{
		assert(image != nullptr);
		assert(recon != nullptr);

		_visible0.GetData(*image);
		_visible_prime0.GetData(*recon);

		return true;
	}

	bool ContrastiveDivergence::DumpLastHidden( float** activations )
	{
		assert(activations != nullptr);

		_hidden0.GetData(*activations);

		return true;
	}

	bool ContrastiveDivergence::DumpLastWeights( float** weights )
	{
		assert(weights != nullptr);

		_weights0.GetData(*weights);

		return true;
	}

	void ContrastiveDivergence::free_kernels()
	{
		// delete our kernels
		SafeDelete(_calc_enabled_visible);
		SafeDelete(_calc_enabled_hidden);
		SafeDelete(_calc_hidden);
		SafeDelete(_calc_hidden_softmax);
		SafeDelete(_calc_hidden_states);
		SafeDelete(_calc_hidden_softmax_states);
		SafeDelete(_calc_visible);
		SafeDelete(_calc_visible_softmax);
		SafeDelete(_update_weights);
	}

	void ContrastiveDivergence::build_kernels()
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

		/// Calc Hidden And States
		if(_model_config.HiddenType != ActivationFunction::Softmax)
		{
			SourceCalcHiddenAndStates src_calc_hidden_and_states;
			src_calc_hidden_and_states.FUNCTION = _model_config.HiddenType;
			src_calc_hidden_and_states.VISIBLE_UNITS = _model_config.VisibleUnits;
			src_calc_hidden_and_states.VISIBLE_DROPOUT_PROB = _training_config.VisibleDropout;
			src_calc_hidden_and_states.Parse();

			_calc_hidden_states = compiler.Build(src_calc_hidden_and_states);
			_calc_hidden_states->Initialize(_model_config.HiddenUnits, _minibatch_size);

			//printf("%s\n", _calc_hidden_states->GetSource().c_str());
		}
		/// Calc Hidden Softmax States
		else
		{
			SourceCalcHiddenSoftmaxStates src_calc_hiddden_softmax_states;
			src_calc_hiddden_softmax_states.ROW_LENGTH = _model_config.HiddenUnits;
			src_calc_hiddden_softmax_states.Parse();

			_calc_hidden_softmax_states = compiler.Build(src_calc_hiddden_softmax_states);
			_calc_hidden_softmax_states->Initialize(_model_config.HiddenUnits, _minibatch_size);

			//printf("%s\n", _calc_hidden_softmax_states->GetSource().c_str());
		}
		

		/// Calc Visible
		SourceCalcVisible src_calc_visible;
		src_calc_visible.FUNCTION = _model_config.VisibleType;
		src_calc_visible.HIDDEN_UNITS = _model_config.HiddenUnits;
		src_calc_visible.HIDDEN_DROPOUT_PROB = _training_config.HiddenDropout;
		src_calc_visible.Parse();

		_calc_visible = compiler.Build(src_calc_visible);
		_calc_visible->Initialize(_model_config.VisibleUnits, _minibatch_size);

		//printf("%s\n", _calc_visible->GetSource().c_str());

		if(_model_config.VisibleType == ActivationFunction::Softmax)
		{
			SourceCalcSoftmax src_calc_visible_softmax;
			src_calc_visible_softmax.ROW_LENGTH = _model_config.VisibleUnits;
			src_calc_visible_softmax.Parse();

			_calc_visible_softmax = compiler.Build(src_calc_visible_softmax);
			_calc_visible_softmax->Initialize(_model_config.VisibleUnits, _minibatch_size);

			//printf("%s\n", _calc_visible_softmax->GetSource().c_str());
		}

		/// Calc Hidden
		SourceCalcHidden src_calc_hidden;
		src_calc_hidden.FUNCTION = _model_config.HiddenType;
		src_calc_hidden.VISIBLE_UNITS = _model_config.VisibleUnits;
		src_calc_hidden.VISIBLE_DROPOUT_PROB = _training_config.VisibleDropout;
		src_calc_hidden.Parse();

		_calc_hidden = compiler.Build(src_calc_hidden);	
		_calc_hidden->Initialize(_model_config.HiddenUnits, _minibatch_size);

		//printf("%s\n", _calc_hidden->GetSource().c_str());

		if(_model_config.HiddenType == ActivationFunction::Softmax)
		{
			SourceCalcSoftmax src_calc_hidden_softmax;
			src_calc_hidden_softmax.ROW_LENGTH = _model_config.HiddenUnits;
			src_calc_hidden_softmax.Parse();

			_calc_hidden_softmax = compiler.Build(src_calc_hidden_softmax);
			_calc_hidden_softmax->Initialize(_model_config.HiddenUnits, _minibatch_size);

			//printf("%s\n", _calc_hidden_softmax->GetSource().c_str());
		}

		/// Update Weights
		SourceCalcWeightUpdates src_update_weights;
		src_update_weights.MINIBATCH_SIZE = _minibatch_size;
		src_update_weights.LEARNING_RATE = _training_config.LearningRate;
		src_update_weights.MOMENTUM = _training_config.Momentum;
		src_update_weights.L1_REGULARIZATION = _training_config.L1Regularization;
		src_update_weights.L2_REGULARIZATION = _training_config.L2Regularization;
		src_update_weights.ADADELTA_DECAY = _training_config.AdadeltaDecay;
		src_update_weights.Parse();

		_update_weights = compiler.Build(src_update_weights);
		_update_weights->Initialize(_model_config.VisibleUnits + 1, _model_config.HiddenUnits + 1);

		//printf("%s\n", _update_weights->GetSource().c_str());
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
			auto u = uniform(random);
			assert(u != 0);
			result[k] = u;
		}

		return result;
	}

	void ContrastiveDivergence::allocate_textures(float* weight_buffer, int32_t seed)
	{	
		std::mt19937_64 random;
		random.seed(static_cast<uint32_t>(seed));

		uint32_t* visible_dropout_seed_buffer = GetSeedBuffer(_model_config.VisibleUnits * 4, 1, random);
		uint32_t* hidden_dropout_seed_buffer = GetSeedBuffer(_model_config.HiddenUnits * 4, 1, random);
		uint32_t* hidden_seed_buffer = GetSeedBuffer(_model_config.HiddenUnits * 4, _minibatch_size, random);

		_visible_dropout_random0 = OpenGLBuffer2D(_model_config.VisibleUnits, 1, ReturnType::UInt4, visible_dropout_seed_buffer);
		_visible_dropout_random1 = OpenGLBuffer2D(_model_config.VisibleUnits, 1, ReturnType::UInt4, nullptr);
		_hidden_dropout_random0 = OpenGLBuffer2D(_model_config.HiddenUnits, 1, ReturnType::UInt4, hidden_dropout_seed_buffer);
		_hidden_dropout_random1 = OpenGLBuffer2D(_model_config.HiddenUnits, 1, ReturnType::UInt4, nullptr);

		_enabled_visible = OpenGLBuffer2D(_model_config.VisibleUnits, 1, ReturnType::UInt, nullptr);
		_enabled_hidden = OpenGLBuffer2D(_model_config.HiddenUnits, 1, ReturnType::UInt, nullptr);

		_visible0 = OpenGLBuffer2D(_model_config.VisibleUnits, _minibatch_size, ReturnType::Float, nullptr);
		_hidden0 = OpenGLBuffer2D(_model_config.HiddenUnits, _minibatch_size, ReturnType::Float, nullptr);
		_hidden_random0 = OpenGLBuffer2D(_model_config.HiddenUnits, _minibatch_size, ReturnType::UInt4, hidden_seed_buffer);
		_hidden_random1 = OpenGLBuffer2D(_model_config.HiddenUnits, _minibatch_size, ReturnType::UInt4, nullptr);
		_hidden_states = OpenGLBuffer2D(_model_config.HiddenUnits, _minibatch_size, ReturnType::Float, nullptr);
		_visible_prime0 = OpenGLBuffer2D(_model_config.VisibleUnits, _minibatch_size, ReturnType::Float, nullptr);
		_hidden_prime0 = OpenGLBuffer2D(_model_config.HiddenUnits, _minibatch_size, ReturnType::Float, nullptr);

		if(_model_config.VisibleType == ActivationFunction::Softmax)
		{
			_visible1 = OpenGLBuffer2D(_model_config.VisibleUnits, _minibatch_size, ReturnType::Float, nullptr);
			_visible_prime1 = OpenGLBuffer2D(_model_config.VisibleUnits, _minibatch_size, ReturnType::Float, nullptr);
		}

		if(_model_config.HiddenType == ActivationFunction::Softmax)
		{
			_hidden1 = OpenGLBuffer2D(_model_config.HiddenUnits, _minibatch_size, ReturnType::Float, nullptr);
			_hidden_prime1 = OpenGLBuffer2D(_model_config.HiddenUnits, _minibatch_size, ReturnType::Float, nullptr);
		}

		// allocate a weight buffer and copy it to buffer
		if(weight_buffer == nullptr)
		{
			// initialize weights to random values 
			uint32_t weight_count = (_model_config.VisibleUnits + 1) * (_model_config.HiddenUnits + 1);
			float* weight_buffer = (float*)malloc(sizeof(float) * weight_count);

			float weight_stdev = float(1.0 / std::sqrtf((float)(_model_config.VisibleUnits + _model_config.HiddenUnits)));
			std::normal_distribution<float> normal(0.0f, weight_stdev);

			uint32_t index = 0;
			for(uint32_t j = 0; j <= _model_config.HiddenUnits; j++)
			{
				for(uint32_t i = 0; i <= _model_config.VisibleUnits; i++)				
				{
					if(i == 0 || j == 0)
					{
						weight_buffer[ index ] = 0.0f;
					}
					else
					{
						weight_buffer[ index ] = normal(random);
					}

					index++;
				}
			}
			_weights0 = OpenGLBuffer2D(_model_config.VisibleUnits + 1, _model_config.HiddenUnits + 1, ReturnType::Float, weight_buffer);
			free(weight_buffer);
		}
		else
		{
			// just use weights received from model
			_weights0 = OpenGLBuffer2D(_model_config.VisibleUnits + 1, _model_config.HiddenUnits + 1, ReturnType::Float, weight_buffer);
		}
		_weights1 = OpenGLBuffer2D(_model_config.VisibleUnits + 1, _model_config.HiddenUnits + 1, ReturnType::Float, nullptr);
		_delta_weights0 = OpenGLBuffer2D(_model_config.VisibleUnits + 1, _model_config.HiddenUnits + 1, ReturnType::Float, nullptr);
		_delta_weights1 = OpenGLBuffer2D(_model_config.VisibleUnits + 1, _model_config.HiddenUnits + 1, ReturnType::Float, nullptr);
		_nesterov_weight = OpenGLBuffer2D(_model_config.VisibleUnits + 1, _model_config.HiddenUnits + 1, ReturnType::Float, nullptr);
		_mean_square_delta0 = OpenGLBuffer2D(_model_config.VisibleUnits + 1, _model_config.HiddenUnits + 1, ReturnType::Float2, nullptr);
		_mean_square_delta1 = OpenGLBuffer2D(_model_config.VisibleUnits + 1, _model_config.HiddenUnits + 1, ReturnType::Float2, nullptr);

		// free freshly allocated seed buffers
		free(visible_dropout_seed_buffer);
		free(hidden_dropout_seed_buffer);
		free(hidden_seed_buffer);

		_error_calculator = new ErrorCalculator(_minibatch_size, _model_config.VisibleUnits, _model_config.VisibleType == ActivationFunction::Softmax ? ErrorFunction::CrossEntropy : ErrorFunction::SquareError);
	}
}