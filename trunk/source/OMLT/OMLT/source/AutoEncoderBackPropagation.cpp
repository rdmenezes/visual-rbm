
// std
#include <algorithm>
#include <random>
using std::swap;

#include <AutoEncoderBackPropagation.h>

namespace OMLT
{
	AutoEncoderBackPropagation::AutoEncoderBackPropagation(const ModelConfig& in_model_config, uint32_t in_minibatch_size)
		: _model_config(in_model_config),
		  _minibatch_size(in_minibatch_size),
		  CalcEnabledVisible(nullptr),
		  CalcEnabledHidden(nullptr),
		  CalcHidden(nullptr),
		  CalcHiddenSoftmax(nullptr),
		  CalcOutput(nullptr),
		  CalcOutputSoftmax(nullptr),
		  CalcOutputSensitivities(nullptr),
		  CalcHiddenSensitivities(nullptr),
		  UpdateWeights(nullptr),
		  _error_calculator(nullptr)
	{
		allocate_textures(nullptr);
	}

	AutoEncoderBackPropagation::AutoEncoderBackPropagation(const AutoEncoder* in_autoencoder, uint32_t in_minibatch_size)
		: _minibatch_size(in_minibatch_size),
		  CalcEnabledVisible(nullptr),
		  CalcEnabledHidden(nullptr),
		  CalcHidden(nullptr),
		  CalcHiddenSoftmax(nullptr),
		  CalcOutput(nullptr),
		  CalcOutputSoftmax(nullptr),
		  CalcOutputSensitivities(nullptr),
		  CalcHiddenSensitivities(nullptr),
		  UpdateWeights(nullptr),
		  _error_calculator(nullptr)
	{
		_model_config.VisibleCount = in_autoencoder->visible_count;
		_model_config.HiddenCount = in_autoencoder->hidden_count;
		_model_config.HiddenType = in_autoencoder->hidden_type;
		_model_config.OutputType = in_autoencoder->output_type;
		
		float* weight_buffer = new float[(_model_config.VisibleCount + 1) * (_model_config.HiddenCount + 1)];
		weight_buffer[0] = 0.0f;

		// hidden biases
		for(uint32_t j = 0; j < _model_config.HiddenCount; j++)
		{
			weight_buffer[(_model_config.VisibleCount + 1) * (j + 1)] = in_autoencoder->encoder.biases()[j];
		}

		// output biases
		for(uint32_t k = 0; k < _model_config.VisibleCount; k++)
		{
			weight_buffer[1 + k] = in_autoencoder->decoder.biases()[k];
		}

		// weights
		for(uint32_t j = 0; j < _model_config.HiddenCount; j++)
		{
			for(uint32_t k = 0; k < _model_config.VisibleCount; k++)
			{
				weight_buffer[(_model_config.VisibleCount + 1) * (j + 1) + k + 1] = in_autoencoder->encoder.feature(j)[k];
			}
		}

		allocate_textures(weight_buffer);

		delete[] weight_buffer;
	}

	AutoEncoderBackPropagation::~AutoEncoderBackPropagation()
	{
		free_kernels();
	}

	void AutoEncoderBackPropagation::SetTrainingConfig(const AutoEncoderBackPropagation::TrainingConfig& in_config)
	{
		// only set recompile flag if the new config is different
		if(memcmp(&in_config, &_training_config, sizeof(TrainingConfig)) != 0)
		{
			_training_config = in_config;
			_recompile_required = true;
		}
	}

	void AutoEncoderBackPropagation::Train(const OpenGLBuffer2D& in_example)
	{
		if(_recompile_required)
		{
			free_kernels();
			build_kernels();

			_recompile_required = false;
		}

		Target = in_example;
		Visible = in_example;

		// calc enabled visible units
		CalcEnabledVisible->SetInput(0, VisibleRandom0);
		CalcEnabledVisible->BindOutput(0, VisibleEnabled);
		CalcEnabledVisible->BindOutput(1, VisibleRandom1);
		CalcEnabledVisible->Run();

		swap(VisibleRandom0, VisibleRandom1);

		// calc enabled hidden units
		CalcEnabledHidden->SetInput(0, HiddenRandom0);
		CalcEnabledHidden->BindOutput(0, HiddenEnabled);
		CalcEnabledHidden->BindOutput(1, HiddenRandom1);
		CalcEnabledHidden->Run();

		swap(HiddenRandom0, HiddenRandom1);
		
		// calc hidden activation
		CalcHidden->SetInput(0, Visible);
		CalcHidden->SetInput(1, VisibleEnabled);
		CalcHidden->SetInput(2, NesterovWeight);
		CalcHidden->BindOutput(0, Hidden0);
		CalcHidden->Run();

		if(_model_config.HiddenType == ActivationFunction::Softmax)
		{
			CalcHiddenSoftmax->SetInput(0, Hidden0);
			CalcHiddenSoftmax->BindOutput(0, Hidden1);
			CalcHiddenSoftmax->Run();

			std::swap(Hidden0, Hidden1);
		}

		// calc output activation
		CalcOutput->SetInput(0, Hidden0);
		CalcOutput->SetInput(1, HiddenEnabled);
		CalcOutput->SetInput(2, NesterovWeight);
		CalcOutput->BindOutput(0, Output0);
		CalcOutput->Run();

		if(_model_config.OutputType == ActivationFunction::Softmax)
		{
			CalcOutputSoftmax->SetInput(0, Output0);
			CalcOutputSoftmax->BindOutput(0, Output1);
			CalcOutputSoftmax->Run();

			std::swap(Output0, Output1);
		}

		// calc output sensitivities
		CalcOutputSensitivities->SetInput(0, Target);
		CalcOutputSensitivities->SetInput(1, Output0);
		CalcOutputSensitivities->BindOutput(0, OutputSensitivities);
		CalcOutputSensitivities->Run();

		// calc hidden sensitivities
		CalcHiddenSensitivities->SetInput(0, OutputSensitivities);
		CalcHiddenSensitivities->SetInput(1, VisibleEnabled);
		CalcHiddenSensitivities->SetInput(2, NesterovWeight);
		CalcHiddenSensitivities->SetInput(3, Hidden0);
		CalcHiddenSensitivities->BindOutput(0, HiddenSensitivities);
		CalcHiddenSensitivities->Run();

		// update weights
		UpdateWeights->SetInput(0, OutputSensitivities);
		UpdateWeights->SetInput(1, HiddenSensitivities);
		UpdateWeights->SetInput(2, Hidden0);
		UpdateWeights->SetInput(3, Visible);
		UpdateWeights->SetInput(4, Weights0);
		UpdateWeights->SetInput(5, DeltaWeights0);
		UpdateWeights->SetInput(6, MeanSquareDelta0);
		UpdateWeights->BindOutput(0, Weights1);
		UpdateWeights->BindOutput(1, DeltaWeights1);
		UpdateWeights->BindOutput(2, MeanSquareDelta1);
		UpdateWeights->BindOutput(3, NesterovWeight);
		UpdateWeights->Run();

		swap(Weights0, Weights1);
		swap(DeltaWeights0, DeltaWeights1);
		swap(MeanSquareDelta0, MeanSquareDelta1);
	}

	AutoEncoder* AutoEncoderBackPropagation::GetAutoEncoder() const
	{
		AutoEncoder* result = new AutoEncoder(_model_config.VisibleCount, _model_config.HiddenCount, _model_config.HiddenType, _model_config.OutputType);

		float* raw_weights = nullptr;
		Weights0.GetData(raw_weights);

		// get the output biases

		for(uint32_t k = 0; k < _model_config.VisibleCount; k++)
		{
			result->decoder.biases()[k] = raw_weights[k + 1];
		}

		// get the hidden biases

		for(uint32_t j = 0; j < _model_config.HiddenCount; j++)
		{
			result->encoder.biases()[j] = raw_weights[(_model_config.VisibleCount + 1) *(j + 1)];
		}

		// now get the symmetrical weights

		for(uint32_t j = 0; j < _model_config.HiddenCount; j++)
		{
			for(uint32_t k = 0; k < _model_config.VisibleCount; k++)
			{
				const uint32_t& i = k;

				float w = raw_weights[(_model_config.VisibleCount + 1) * (j + 1) + k + 1];

				result->encoder.feature(j)[k] = w;
				result->decoder.feature(i)[j] = w;
			}
		}

		return result;
	}

	bool AutoEncoderBackPropagation::DumpLastVisible( float** image, float** recon )
	{
		assert(image != nullptr);
		assert(recon != nullptr);

		Visible.GetData(*image);
		Output0.GetData(*recon);

		return true;
	}

	bool AutoEncoderBackPropagation::DumpLastHidden( float** activations )
	{
		assert(activations != nullptr);

		Hidden0.GetData(*activations);

		return true;
	}

	bool AutoEncoderBackPropagation::DumpLastWeights( float** weights )
	{
		assert(weights != nullptr);

		Weights0.GetData(*weights);

		return true;
	}

	float AutoEncoderBackPropagation::GetLastError()
	{
		return _error_calculator->CalcError(Target, Output0);
	}
	
	float AutoEncoderBackPropagation::GetError(const OpenGLBuffer2D& in_example)
	{
		OpenGLBuffer2D previous = Target;
		Target = in_example;

		// calc hidden activation
		CalcHidden->SetInput(0, Visible);
		CalcHidden->SetInput(1, VisibleEnabled);
		CalcHidden->SetInput(2, Weights0);
		CalcHidden->BindOutput(0, Hidden0);
		CalcHidden->Run();

		if(_model_config.HiddenType == ActivationFunction::Softmax)
		{
			CalcHiddenSoftmax->SetInput(0, Hidden0);
			CalcHiddenSoftmax->BindOutput(0, Hidden1);
			CalcHiddenSoftmax->Run();

			std::swap(Hidden0, Hidden1);
		}

		// calc output activation
		CalcOutput->SetInput(0, Hidden0);
		CalcOutput->SetInput(1, HiddenEnabled);
		CalcOutput->SetInput(2, Weights0);
		CalcOutput->BindOutput(0, Output0);
		CalcOutput->Run();

		if(_model_config.OutputType == ActivationFunction::Softmax)
		{
			CalcOutputSoftmax->SetInput(0, Output0);
			CalcOutputSoftmax->BindOutput(0, Output1);
			CalcOutputSoftmax->Run();

			std::swap(Output0, Output1);
		}

		float result = GetLastError();

		Target = previous;
		return result;
	}

	void AutoEncoderBackPropagation::free_kernels()
	{
		SafeDelete(CalcEnabledVisible);
		SafeDelete(CalcEnabledHidden);
		SafeDelete(CalcHidden);
		SafeDelete(CalcHiddenSoftmax);
		SafeDelete(CalcOutput);
		SafeDelete(CalcOutputSoftmax);
		SafeDelete(CalcOutputSensitivities);
		SafeDelete(CalcHiddenSensitivities);
		SafeDelete(UpdateWeights);
		
		SafeDelete(_error_calculator);
	}

	void AutoEncoderBackPropagation::build_kernels()
	{
		OpenGLCompiler comp;

		// calc enabled visible units
		{
			SourceCalcEnabled  source;
			source.DROPOUT_PROB = _training_config.VisibleDropout;
			source.Parse();

			CalcEnabledVisible = comp.Build(source);
			CalcEnabledVisible->Initialize(_model_config.VisibleCount, 1);
		}
		// calc enabled hidden units
		{
			SourceCalcEnabled source;
			source.DROPOUT_PROB = _training_config.HiddenDropout;
			source.Parse();

			CalcEnabledHidden = comp.Build(source);
			CalcEnabledHidden->Initialize(_model_config.HiddenCount, 1);
		}
		// calc hidden
		{
			SourceCalcHidden source;
			source.FUNC = _model_config.HiddenType;
			source.VISIBLE_DROPOUT_PROB = _training_config.VisibleDropout;
			source.VISIBLE_UNITS = _model_config.VisibleCount;
			source.Parse();

			CalcHidden = comp.Build(source);
			CalcHidden->Initialize(_model_config.HiddenCount, _minibatch_size);
		}
		// calc hidden softmax
		if(_model_config.HiddenType == ActivationFunction::Softmax)
		{
			SourceSoftmax source;
			source.ROW_LENGTH = _model_config.HiddenCount;
			source.Parse();

			CalcHiddenSoftmax = comp.Build(source);
			CalcHiddenSoftmax->Initialize(_model_config.HiddenCount, _minibatch_size);
		}
		// calc output
		{
			SourceCalcOutput source;
			source.FUNC = _model_config.OutputType;
			source.HIDDEN_DROPOUT_PROB = _training_config.HiddenDropout;
			source.HIDDEN_UNITS = _model_config.HiddenCount;
			source.Parse();

			CalcOutput = comp.Build(source);
			CalcOutput->Initialize(_model_config.VisibleCount, _minibatch_size);
		}
		// calc output softmax
		if(_model_config.OutputType == ActivationFunction::Softmax)
		{
			SourceSoftmax source;
			source.ROW_LENGTH = _model_config.VisibleCount;
			source.Parse();

			CalcOutputSoftmax = comp.Build(source);
			CalcOutputSoftmax->Initialize(_model_config.VisibleCount, _minibatch_size);
		}
		// calc output sensitivities
		{
			SourceCalcOutputSensitivities source;
			source.FUNC = _model_config.OutputType;
			source.Parse();

			CalcOutputSensitivities = comp.Build(source);
			CalcOutputSensitivities->Initialize(_model_config.VisibleCount, _minibatch_size);
		}
		// calc hidden sensitivities
		{
			SourceCalcHiddenSensitivities source;
			source.FUNC = _model_config.HiddenType;
			source.VISIBLE_UNITS = _model_config.VisibleCount;
			source.Parse();

			CalcHiddenSensitivities = comp.Build(source);
			CalcHiddenSensitivities->Initialize(_model_config.HiddenCount, _minibatch_size);
		}
		// update weight deltas and weights
		{
			SourceUpdateWeights source;
			source.MINIBATCH_SIZE = _minibatch_size;
			source.LEARNING_RATE = _training_config.LearningRate;
			source.MOMENTUM = _training_config.Momentum;
			source.L1_REGULARIZATION = _training_config.L1Regularization;
			source.L2_REGULARIZATION = _training_config.L2Regularization;
			source.VISIBLE_DROPOUT = _training_config.VisibleDropout;
			source.HIDDEN_DROPOUT = _training_config.HiddenDropout;
			source.ADADELTA_DECAY = _training_config.AdadeltaDecay;
			source.Parse();

			UpdateWeights = comp.Build(source);
			UpdateWeights->Initialize(_model_config.VisibleCount + 1, _model_config.HiddenCount + 1);
		}

		_error_calculator = new ErrorCalculator(_minibatch_size, _model_config.VisibleCount, _model_config.OutputType == ActivationFunction::Softmax ? ErrorFunction::CrossEntropy : ErrorFunction::SquareError);
	}

	extern uint32_t* GetSeedBuffer(uint32_t, uint32_t, std::mt19937_64&);
	void AutoEncoderBackPropagation::allocate_textures(float* weight_buffer)
	{
		std::mt19937_64 random;
		random.seed(1);

		uint32_t* visible_dropout_seed_buffer = GetSeedBuffer(_model_config.VisibleCount * 4, 1, random);
		uint32_t* hidden_dropout_seed_buffer = GetSeedBuffer(_model_config.HiddenCount * 4, 1, random);

		VisibleRandom0 = OpenGLBuffer2D(_model_config.VisibleCount, 1, ReturnType::UInt4, visible_dropout_seed_buffer);
		VisibleRandom1 = OpenGLBuffer2D(_model_config.VisibleCount, 1, ReturnType::UInt4, nullptr);
		VisibleEnabled = OpenGLBuffer2D(_model_config.VisibleCount, 1, ReturnType::UInt, nullptr);

		HiddenRandom0 = OpenGLBuffer2D(_model_config.HiddenCount, 1, ReturnType::UInt4, hidden_dropout_seed_buffer);
		HiddenRandom1 = OpenGLBuffer2D(_model_config.HiddenCount, 1, ReturnType::UInt4, nullptr);
		HiddenEnabled = OpenGLBuffer2D(_model_config.HiddenCount, 1, ReturnType::UInt, nullptr);

		if(weight_buffer == nullptr)
		{
			// initialize weights to random values 
			uint32_t weight_count = (_model_config.VisibleCount + 1) * (_model_config.HiddenCount + 1);
			weight_buffer = (float*)malloc(sizeof(float) * weight_count);

			float weight_stdev = float(1.0 / std::sqrtf((float)(_model_config.VisibleCount + _model_config.HiddenCount)));
			std::normal_distribution<float> normal(0.0f, weight_stdev);

			uint32_t index = 0;
			for(uint32_t j = 0; j <= _model_config.HiddenCount; j++)
			{
				for(uint32_t i = 0; i <= _model_config.VisibleCount; i++)				
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
			Weights0 = OpenGLBuffer2D(_model_config.VisibleCount + 1, _model_config.HiddenCount + 1, ReturnType::Float, weight_buffer);
			free(weight_buffer);
		}
		else
		{
			Weights0 = OpenGLBuffer2D(_model_config.VisibleCount + 1, _model_config.HiddenCount + 1, ReturnType::Float, weight_buffer);
		}
		Weights1 = OpenGLBuffer2D(_model_config.VisibleCount + 1, _model_config.HiddenCount + 1, ReturnType::Float, nullptr);
		DeltaWeights0 = OpenGLBuffer2D(_model_config.VisibleCount + 1, _model_config.HiddenCount + 1, ReturnType::Float, nullptr);
		DeltaWeights1 = OpenGLBuffer2D(_model_config.VisibleCount + 1, _model_config.HiddenCount + 1, ReturnType::Float, nullptr);
		NesterovWeight = OpenGLBuffer2D(_model_config.VisibleCount + 1, _model_config.HiddenCount + 1, ReturnType::Float, nullptr);
		MeanSquareDelta0 = OpenGLBuffer2D(_model_config.VisibleCount + 1, _model_config.HiddenCount + 1, ReturnType::Float2, nullptr);
		MeanSquareDelta1 = OpenGLBuffer2D(_model_config.VisibleCount + 1, _model_config.HiddenCount + 1, ReturnType::Float2, nullptr);

		Visible = OpenGLBuffer2D(_model_config.VisibleCount, _minibatch_size, ReturnType::Float, nullptr);
		Hidden0 = OpenGLBuffer2D(_model_config.HiddenCount, _minibatch_size, ReturnType::Float, nullptr);
		if(_model_config.HiddenType == ActivationFunction::Softmax)
		{
			Hidden1 = OpenGLBuffer2D(_model_config.HiddenCount, _minibatch_size, ReturnType::Float, nullptr);
		}
		Output0 = OpenGLBuffer2D(_model_config.VisibleCount, _minibatch_size, ReturnType::Float, nullptr);
		if(_model_config.OutputType == ActivationFunction::Softmax)
		{
			Output1 = OpenGLBuffer2D(_model_config.VisibleCount, _minibatch_size, ReturnType::Float, nullptr);
		}

		HiddenSensitivities = OpenGLBuffer2D(_model_config.HiddenCount, _minibatch_size, ReturnType::Float, nullptr);
		OutputSensitivities = OpenGLBuffer2D(_model_config.VisibleCount, _minibatch_size, ReturnType::Float, nullptr);

		free(visible_dropout_seed_buffer);
		free(hidden_dropout_seed_buffer);
	}
}