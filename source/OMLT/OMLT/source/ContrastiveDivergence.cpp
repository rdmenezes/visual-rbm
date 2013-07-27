// stdlib
#include <string.h>

// OMLT
#include "ContrastiveDivergence.h"
#include "Common.h"

namespace OMLT
{
	ContrastiveDivergence::ContrastiveDivergence( const ModelConfig in_config)
		: _model_config(in_config)
		, _calc_enabled_units(nullptr)
		, _copy_visible(nullptr)
		, _calc_hidden(nullptr)
		, _calc_hidden_states(nullptr)
		, _calc_visible(nullptr)
		, _update_weights(nullptr)
		, _calc_error(nullptr)
		, _recompile_required(true)
	{
		AllocateTextures();
	}

	ContrastiveDivergence::~ContrastiveDivergence()
	{
		SafeDelete(_calc_enabled_units);
		SafeDelete(_copy_visible);
		SafeDelete(_calc_hidden);
		SafeDelete(_calc_hidden_states);
		SafeDelete(_calc_visible);
		SafeDelete(_update_weights);
		SafeDelete(_calc_error);
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

	void ContrastiveDivergence::BuildKernels()
	{

	}

	void ContrastiveDivergence::AllocateTextures()
	{
		_visible_dropout_random0 = OpenGLBuffer2D(_model_config.VisibleUnits + 1, 1, ReturnType::UInt, nullptr);
		_visible_dropout_random1 = OpenGLBuffer2D(_model_config.VisibleUnits + 1, 1, ReturnType::UInt, nullptr);
		_hidden_dropout_random0 = OpenGLBuffer2D(_model_config.HiddenUnits + 1, 1, ReturnType::UInt, nullptr);
		_hidden_dropout_random1 = OpenGLBuffer2D(_model_config.HiddenUnits + 1, 1, ReturnType::UInt, nullptr);

		_enabled_visible = OpenGLBuffer2D(_model_config.VisibleUnits + 1, 1, ReturnType::UInt, nullptr);
		_enabled_hidden = OpenGLBuffer2D(_model_config.HiddenUnits + 1, 1, ReturnType::UInt, nullptr);

		_visible = OpenGLBuffer2D(_model_config.VisibleUnits + 1, _model_config.MinibatchSize, ReturnType::Float, nullptr);
		_hidden = OpenGLBuffer2D(_model_config.HiddenUnits + 1, _model_config.MinibatchSize, ReturnType::Float, nullptr);
		_hidden_states = OpenGLBuffer2D(_model_config.HiddenUnits + 1, _model_config.MinibatchSize, ReturnType::Float, nullptr);
		_visible_prime = OpenGLBuffer2D(_model_config.VisibleUnits + 1, _model_config.MinibatchSize, ReturnType::Float, nullptr);
		_hidden_prime = OpenGLBuffer2D(_model_config.HiddenUnits + 1, _model_config.MinibatchSize, ReturnType::Float, nullptr);

		_weights0 = OpenGLBuffer2D(_model_config.HiddenUnits + 1, _model_config.VisibleUnits + 1, ReturnType::Float, nullptr);
		_weights1 = OpenGLBuffer2D(_model_config.HiddenUnits + 1, _model_config.VisibleUnits + 1, ReturnType::Float, nullptr);
		_delta_weights0 = OpenGLBuffer2D(_model_config.HiddenUnits + 1, _model_config.VisibleUnits + 1, ReturnType::Float, nullptr);
		_delta_weights1 = OpenGLBuffer2D(_model_config.HiddenUnits + 1, _model_config.VisibleUnits + 1, ReturnType::Float, nullptr);
	}
}