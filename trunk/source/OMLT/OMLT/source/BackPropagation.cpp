// std
#include <algorithm>
using std::swap;
#include <assert.h>
#include <random>

// OMLT
#include "BackPropagation.h"

namespace OMLT
{

	BackPropagation::BackPropagation( uint32_t in_input_units, TrainerConfig in_config )
		: InputUnits(in_input_units)
		, LearningRate(in_config.LearningRate)
		, Momentum(in_config.Momentum)
		, MinibatchSize(in_config.MinibatchSize)
		, L1Regularization(in_config.L1Regularization)
		, L2Regularization(in_config.L2Regularization)
	{

	}

	BackPropagation::BackPropagation( MultilayerPerceptron* in_mlp, TrainerConfig in_config )
		: InputUnits(in_mlp->GetLayer(0)->inputs)
		, LearningRate(in_config.LearningRate)
		, Momentum(in_config.Momentum)
		, MinibatchSize(in_config.MinibatchSize)
		, L1Regularization(in_config.L1Regularization)
		, L2Regularization(in_config.L2Regularization)
	{
		for(uint32_t k = 0; k < in_mlp->LayerCount(); k++)
		{
			MLP::Layer* layer = in_mlp->GetLayer(k);
			
			LayerConfig layer_config;
			{
				layer_config.Function = layer->function;
				layer_config.InputDropoutProbability = 0.0f;
				layer_config.Noisy = false;
				layer_config.OutputUnits = layer->outputs;
			}

			// allocate contiguous memory block to put into texture memory
			float* weight_buffer = new float[(layer->inputs + 1) * layer->outputs];

			// copy weights from this layer in the MLP to the buffer
			for(uint32_t j = 0; j < layer->outputs; j++)
			{
				weight_buffer[j * (layer->inputs + 1)] = layer->biases[j];
				for(uint32_t i = 1; i <= layer->inputs; i++)
				{
					weight_buffer[j * (layer->inputs + 1) + i] = layer->weights[j][i];
				}
			}

			// add layer and use weight buffer as initial weights
			AddLayer(layer_config, weight_buffer);
			delete[] weight_buffer;
		}
	}

	void BackPropagation::AddLayer( LayerConfig config )
	{
		AddLayer(config, nullptr);
	}

	void BackPropagation::AddLayer( LayerConfig config, float* weights )
	{
		Layers.push_back(BuildLayer(config, weights));
	}

	float BackPropagation::Train( OpenGLBuffer2D& example_input, OpenGLBuffer2D& example_label )
	{
		assert(example_input.Width == InputUnits);
		assert(example_input.Height == MinibatchSize);
		assert(example_label.Width == Layers.back()->OutputUnits);
		assert(example_label.Height == MinibatchSize);

		// feed forward
		Layers.front()->Input = &example_input;
		for(auto it = Layers.begin(); it != Layers.end(); ++it)
		{
			Layer* lay = *it;

			OpenGLProgram* calc_enabled = lay->CalcEnabledInputs;
			{
				calc_enabled->SetInput(0, lay->InputRandom0);
				assert(lay->InputRandom0.Width == lay->InputUnits);
				assert(lay->InputRandom0.Height == 1);

				calc_enabled->BindOutput(0, lay->InputRandom1);
				calc_enabled->BindOutput(1, lay->InputEnabled);
				assert(lay->InputRandom1.Width == lay->InputUnits);
				assert(lay->InputRandom1.Height == 1);
				assert(lay->InputEnabled.Width == lay->InputUnits);
				assert(lay->InputEnabled.Height == 1);

				calc_enabled->Run();

				swap(lay->InputRandom0, lay->InputRandom1);
			}

			OpenGLProgram* feed_forward = lay->FeedForward;
			{
				feed_forward->SetInput(0, *lay->Input);
				feed_forward->SetInput(1, lay->InputEnabled);
				feed_forward->SetInput(2, lay->Weights0);
				feed_forward->SetInput(3, lay->OutputRandom0);
				assert(lay->Input->Width == lay->InputUnits);
				assert(lay->Input->Height == MinibatchSize);
				assert(lay->Weights0.Width == (lay->InputUnits + 1));
				assert(lay->Weights0.Height == lay->OutputUnits);
				assert(lay->OutputRandom0.Width == lay->OutputUnits);
				assert(lay->OutputRandom0.Height == MinibatchSize);

				feed_forward->BindOutput(0, lay->Activation);
				feed_forward->BindOutput(1, lay->OutputRandom1);
				assert(lay->Activation.Width == lay->OutputUnits);
				assert(lay->Activation.Height == MinibatchSize);
				assert(lay->OutputRandom1.Width == lay->OutputRandom0.Width);
				assert(lay->OutputRandom1.Height == lay->OutputRandom0.Height);

				feed_forward->Run();

				swap(lay->OutputRandom0, lay->OutputRandom1);
			}
		}

		// error calculation
		float err = 0.0f;
		{
			static float* calculate_output = nullptr;
			static float* desired_output = nullptr;

			Layers.back()->Activation.GetData(calculate_output);
			example_label.GetData(desired_output);

			uint32_t pixels = example_label.Width * example_label.Height;

			for(uint32_t i = 0; i < pixels; i++)
			{
				float diff = calculate_output[i] - desired_output[i];
				err += diff*diff;
			}

			err /= pixels;
		}
		// calc sensitivities, feed backward
		{
			auto it = Layers.rbegin();
			Layer* lay = *it;
			OpenGLProgram* calc_sensitivities = lay->CalcSensitivity;
			{
				// fill out calc_top_sensitivities (and set the training examples the labels!
				calc_sensitivities->SetInput(0, example_label);
				calc_sensitivities->SetInput(1, lay->Activation);
				assert(example_label.Width == lay->OutputUnits);
				assert(example_label.Height == MinibatchSize);
				assert(lay->Activation.Width == lay->OutputUnits);
				assert(lay->Activation.Height == MinibatchSize);
			
				calc_sensitivities->BindOutput(0, lay->Sensitivities);
				assert(lay->Sensitivities.Width == lay->OutputUnits);
				assert(lay->Sensitivities.Height == MinibatchSize);

				calc_sensitivities->Run();

				//float* sensitivities = nullptr;
				//lay->Sensitivities.GetData(sensitivities);

				//printf("");
			}
			while(++it != Layers.rend())
			{
				lay = *it;
				// fill out whatever
				calc_sensitivities = lay->CalcSensitivity;

				calc_sensitivities->SetInput(0, lay->NextLayer->Weights0);
				calc_sensitivities->SetInput(1, lay->NextLayer->Sensitivities);
				calc_sensitivities->SetInput(2, lay->Activation);
				assert(lay->NextLayer->Weights0.Width == (lay->OutputUnits + 1));

				calc_sensitivities->BindOutput(0, lay->Sensitivities);
				assert(lay->Sensitivities.Width == lay->OutputUnits);
				assert(lay->Sensitivities.Height == MinibatchSize);

				calc_sensitivities->Run();
			}
		}

		// now update all the weight matrices
		for(auto it = Layers.rbegin(); it != Layers.rend(); ++it)
		{
			Layer* lay = *it;
			OpenGLProgram* update_weights = lay->UpdateWeights;

			// fill out whatever		
			update_weights->SetInput(0, lay->Sensitivities);
			update_weights->SetInput(1, *lay->Input);
			update_weights->SetInput(2, lay->InputEnabled);
			update_weights->SetInput(3, *lay->OutputEnabled);
			update_weights->SetInput(4, lay->Weights0);
			update_weights->SetInput(5, lay->DeltaWeights0);
			assert(lay->Sensitivities.Width == lay->OutputUnits);
			assert(lay->Sensitivities.Height == MinibatchSize);
			assert(lay->Input->Width == lay->InputUnits);
			assert(lay->Input->Height == MinibatchSize);
			assert(lay->InputEnabled.Width == lay->InputUnits);
			assert(lay->InputEnabled.Height == 1);
			assert(lay->OutputEnabled->Width == lay->OutputUnits);
			assert(lay->OutputEnabled->Height == 1);
			assert(lay->Weights0.Width == (lay->InputUnits + 1));
			assert(lay->Weights0.Height == lay->OutputUnits);
			assert(lay->DeltaWeights0.Width == lay->Weights0.Width);
			assert(lay->DeltaWeights0.Height == lay->Weights0.Height);
		
			update_weights->BindOutput(0, lay->Weights1);
			update_weights->BindOutput(1, lay->DeltaWeights1);
			assert(lay->Weights1.Width == (lay->InputUnits + 1));
			assert(lay->Weights1.Height == lay->OutputUnits);
			assert(lay->DeltaWeights1.Width == lay->Weights1.Width);
			assert(lay->DeltaWeights1.Height == lay->Weights1.Height);


			update_weights->Run();

			//float* delta_weights = nullptr;
			//float* weights = nullptr;
			//lay->DeltaWeights1.GetData(delta_weights);
			//lay->Weights1.GetData(weights);

			swap(lay->Weights0, lay->Weights1);
			swap(lay->DeltaWeights0, lay->DeltaWeights1);
		}

		return err;
	}


	BackPropagation::Layer::Layer()
	{

	}

	BackPropagation::Layer::~Layer()
	{

	}

	BackPropagation::Layer* BackPropagation::BuildLayer( LayerConfig in_Config, float* in_weights )
	{
		std::mt19937_64 random;
		random.seed(1);
		std::uniform_int_distribution<uint32_t> uniform(0, 0xFFFFFFFF);
		std::normal_distribution<float> normal;

		Layer* result = new Layer();

		if(Layers.size() == 0)
		{
			result->InputUnits = this->InputUnits;
		}
		else
		{
			result->InputUnits = Layers.back()->OutputUnits;
		}
		result->OutputUnits = in_Config.OutputUnits;
	
		result->Function = in_Config.Function;
		result->Noisy = in_Config.Noisy;
		result->InputDropoutProbability = in_Config.InputDropoutProbability;

		if(Layers.size() == 0)
		{
			result->Input = nullptr;
		}
		else
		{
			result->Input = &Layers.back()->Activation;
		}

		uint32_t width, height;
		// init input random
		{
			width = result->InputUnits;
			height = 1;

			result->InputEnabled = OpenGLBuffer2D(width, height, ReturnType::Float, nullptr);

			uint32_t* random_buffer = new uint32_t[width * height];
			for(uint32_t i = 0; i < width * height; i++)
			{
				random_buffer[i] = uniform(random);
			}
			result->InputRandom0 = OpenGLBuffer2D(width, height, ReturnType::UInt, random_buffer);
			delete[] random_buffer;
			result->InputRandom1 = OpenGLBuffer2D(width, height, ReturnType::UInt, nullptr);
		}

		// init weights, delta weights
		// weight format: j rows, each containing i + 1 values, first value in each row is bias
		{
			width = result->InputUnits + 1;
			height = result->OutputUnits;

			
			if(in_weights == nullptr)
			{
				float* weight_buffer = new float[width * height];
		
				for(uint32_t j = 0; j < height; j++)
				{
					// bias is first value in a row
					int i = 0;
					// always init bias to 0
					weight_buffer[j * width + 0] = 0.0f;
					for(i = 1; i < width; i++)
					{
						weight_buffer[j * width + i] = normal(random) * 0.1f;

						//printf("From %i to %i: %f\n", i, j, weight_buffer[j * width + i]);
					}
				}

				result->Weights0 = OpenGLBuffer2D(width, height, ReturnType::Float, weight_buffer);
				delete[] weight_buffer;
			}
			else
			{
				result->Weights0 = OpenGLBuffer2D(width, height, ReturnType::Float, in_weights);
			}
			result->Weights1 = OpenGLBuffer2D(width, height, ReturnType::Float, nullptr);
			result->DeltaWeights0 = OpenGLBuffer2D(width, height, ReturnType::Float, nullptr);
			result->DeltaWeights1 = OpenGLBuffer2D(width, height, ReturnType::Float, nullptr);
			result->OutputEnabled = nullptr;

			if(Layers.size() > 0)
			{
				Layers.back()->OutputEnabled = &result->InputEnabled;
			}
		}

		// now init our output related buffers
		{
			width = result->OutputUnits;
			height = MinibatchSize;
			result->Activation =  OpenGLBuffer2D(width, height, ReturnType::Float, nullptr);
			result->OutputRandom0 =  OpenGLBuffer2D(width, height, ReturnType::UInt, nullptr);
			result->OutputRandom1 =  OpenGLBuffer2D(width, height, ReturnType::UInt, nullptr);
		}

		// sensitivites all on their own
		{
			width = result->OutputUnits;
			height = MinibatchSize;
			result->Sensitivities =  OpenGLBuffer2D(width, height, ReturnType::Float, nullptr);
		}

		result->CalcEnabledInputs = nullptr;
		result->FeedForward = nullptr;
		result->CalcSensitivity = nullptr;
		result->UpdateWeights = nullptr;
	
		result->NextLayer = nullptr;
		if(Layers.size() > 0)
		{
			Layers.back()->NextLayer = result;
		}

		return result;
	}

	void BackPropagation::Initialize()
	{
		OpenGLCompiler comp;

		for(auto it = Layers.begin(); it != Layers.end(); ++it)
		{
			Layer* layer = *it;

	#		define DeleteAndNull(X) delete X; X = nullptr;
			DeleteAndNull(layer->CalcEnabledInputs)
			DeleteAndNull(layer->FeedForward)
			DeleteAndNull(layer->CalcSensitivity)
			DeleteAndNull(layer->UpdateWeights)
	#		undef DeleteAndNull

			// calc enabled inputs
			{
				SourceCalcEnabledUnits source;
				source.DROPOUT_PROB = layer->InputDropoutProbability;
			
				source.Parse();
				layer->CalcEnabledInputs = comp.Build(source);
				layer->CalcEnabledInputs->Initialize(layer->InputUnits, 1);
				//printf("%s\n", layer->CalcEnabledInputs->GetSource().c_str());
			}

			// feed forward
			{
				SourceFeedForward source;
				source.FUNC = layer->Function;
				source.INPUT_DROPOUT_PROB = layer->InputDropoutProbability;
				source.INPUT_COUNT = layer->InputUnits;
				source.NOISY = layer->Noisy;

				source.Parse();
				layer->FeedForward = comp.Build(source);
				layer->FeedForward->Initialize(layer->OutputUnits, MinibatchSize);
				//printf("%s\n", layer->FeedForward->GetSource().c_str());
			}

			// calc sensitivies
			{
				if(layer == Layers.back())
				{
					SourceCalcTopSensitivities source;
					source.FUNC = layer->Function;
					source.MINIBATCH_SIZE = MinibatchSize;
				
					source.Parse();
					layer->CalcSensitivity = comp.Build(source);
					layer->CalcSensitivity->Initialize(layer->OutputUnits, MinibatchSize);
					//printf("%s\n", layer->CalcSensitivity->GetSource().c_str());
				}
				else
				{
					SourceCalcSensitivities source;
					source.FUNC = layer->Function;
					source.MINIBATCH_SIZE = MinibatchSize;
					source.NEXT_OUTPUT_COUNT = layer->NextLayer->OutputUnits;

					source.Parse();
					layer->CalcSensitivity = comp.Build(source);
					layer->CalcSensitivity->Initialize(layer->OutputUnits, MinibatchSize);
					//printf("%s\n", layer->CalcSensitivity->GetSource().c_str());
				}
			}

			// update weights
			{
				SourceUpdateWeights source;
				source.LEARNING_RATE = LearningRate;
				source.MOMENTUM = Momentum;
				source.MINIBATCH_SIZE = MinibatchSize;
				source.L1_REGULARIZATION = L1Regularization;
				source.L2_REGULARIZATION = L2Regularization;

				source.Parse();
				layer->UpdateWeights = comp.Build(source);
				layer->UpdateWeights->Initialize(layer->InputUnits + 1, layer->OutputUnits);
				//printf("%s\n", layer->UpdateWeights->GetSource().c_str());
			}
		}	

		{
			Layer* last_layer = Layers.back();
			float* enabled = new float[last_layer->OutputUnits];
			for(uint32_t j = 0; j < last_layer->OutputUnits; j++)
			{
				enabled[j] = 1.0f;
			}

			last_layer->OutputEnabled = new OpenGLBuffer2D(last_layer->OutputUnits, 1, ReturnType::Float, enabled);
			delete[] enabled;
		}
	}

	MultilayerPerceptron* BackPropagation::GetMultilayerPerceptron() const
	{
		return GetMultilayerPerceptron(0, Layers.size() - 1);
	}

	MultilayerPerceptron* BackPropagation::GetMultilayerPerceptron(uint32_t begin_layer, uint32_t end_layer) const
	{
		assert(begin_layer < Layers.size() && end_layer < Layers.size());
		assert(begin_layer <= end_layer);

		MultilayerPerceptron* result = new MultilayerPerceptron();

		for(uint32_t k = begin_layer; k <= end_layer; k++)
		{
			BackPropagation::Layer* bp_layer = Layers[k];

			MultilayerPerceptron::Layer* layer = new MultilayerPerceptron::Layer(bp_layer->InputUnits, bp_layer->OutputUnits, bp_layer->Function);
			float* gpu_weights = nullptr;
			bp_layer->Weights0.GetData(gpu_weights);

			float* gpu_weights_head = gpu_weights;
			for(uint32_t j = 0; j < layer->outputs; j++)
			{
				layer->biases[j] = gpu_weights_head[0];
				memcpy(layer->weights[j], gpu_weights_head + 1, sizeof(float) * layer->inputs);

				gpu_weights_head += layer->inputs + 1;
			}

			bool added = result->AddLayer(layer);
			assert(added == true);

			free(gpu_weights);
		}

		return result;
	}

}