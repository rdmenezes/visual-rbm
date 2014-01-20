// libc
#include <assert.h>

// stdc
#include <sstream>
#include <cstring>
#include <cmath>

// VisualRBM Interop
#include "RBMProcessor.h"
#include "MessageQueue.h"

// OMLT
#include <DataAtlas.h>
#include <ContrastiveDivergence.h>
#include <BackPropagation.h>
#include <RestrictedBoltzmannMachine.h>
#include <MultilayerPerceptron.h>
#include <Common.h>
#include <IDX.hpp>
#include <TrainingSchedule.h>
using namespace OMLT;

// msft
using namespace System::IO;

#define SAFE_DELETE(X) delete X; X = nullptr;
#define SAFE_ARRAY_DELETE(X) delete[] X; X = nullptr;

inline float sigmoid(float x)
{
	return 1.0f / (1.0f + exp(-x));
}

namespace QuickBoltzmann
{
	static RBMProcessor::RBMProcessorState currentState;

	// Backend Static Data
	static ContrastiveDivergence* cd = nullptr;
	static BackPropagation* bp = nullptr;
	static DataAtlas* training_data = nullptr;
	static DataAtlas* validation_data = nullptr;
	static TrainingSchedule<ContrastiveDivergence>* cd_schedule = nullptr;
	static RBM* loaded_rbm = nullptr;

	// various model parameters
	static ModelType model_type = ModelType::RBM;
	static uint32_t visible_count = 0;
	static uint32_t hidden_count = 100;
	static UnitType visible_type = UnitType::Sigmoid;
	static UnitType hidden_type = UnitType::Sigmoid;

	// various training parameters
	static float learning_rate = 0.001f;
	static float momentum = 0.5f;
	static float l1 = 0.0f;
	static float l2 = 0.0f;
	static float visible_dropout = 0.0f;
	static float hidden_dropout = 0.0f;
	
	static uint32_t epochs = 100;
	static uint32_t iterations = 0;
	static uint32_t total_iterations = 0;

	// number of training examples to present per training step
	static uint32_t minibatch_size = 10;

	// buffers we dump visible, hiddden and weights to for visualization
	AlignedMemoryBlock<float> visible_buffer;
	AlignedMemoryBlock<float> visible_recon_buffer;
	AlignedMemoryBlock<float> visible_diff_buffer;
	AlignedMemoryBlock<float> hidden_buffer;
	AlignedMemoryBlock<float> weight_buffer;

	// brings up a message box with an error message
	static inline void ShowError(String^ error)
	{
		System::Windows::Forms::MessageBox::Show(error, "Error", System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Error);
	}

	// load data in from .NET Stream into a std::string
	static std::string LoadFile(Stream^ in_stream)
	{
		std::stringstream ss;
		for(int32_t b = in_stream->ReadByte(); b >= 0; b = in_stream->ReadByte())
		{
			ss << (char)b;
		}
		in_stream->Close();
		return ss.str();
	}

	// returns true if training is finished
	template<typename T>
	static bool UpdateSchedule(OMLT::TrainingSchedule<T>* schedule, T* trainer)
	{
		T::TrainingConfig train_config;
		if(schedule->NextEpoch(train_config))
		{
			trainer->SetTrainingConfig(train_config);

			LogConfig(train_config);
			RBMProcessor::Epochs = schedule->GetEpochs();
		}
		else if(schedule->TrainingComplete())
		{
			return true;
		}
		return false;
	}

	void LogConfig(const CD::TrainingConfig& train_config)
	{
		RBMProcessor::LearningRate = train_config.LearningRate;
		RBMProcessor::Momentum = train_config.Momentum;
		RBMProcessor::L1Regularization = train_config.L1Regularization;
		RBMProcessor::L2Regularization = train_config.L2Regularization;
		RBMProcessor::VisibleDropout = train_config.VisibleDropout;
		RBMProcessor::HiddenDropout = train_config.HiddenDropout;
	}

	static void BuildCDSchedule()
	{
		assert(cd_schedule == nullptr);
		CD::ModelConfig model_config;
		{
			model_config.VisibleUnits = visible_count;
			model_config.HiddenUnits = hidden_count;
			model_config.VisibleType = (ActivationFunction_t)visible_type;
			model_config.HiddenType = (ActivationFunction_t)hidden_type;
		}
		CD::TrainingConfig train_config;
		{
			train_config.LearningRate = learning_rate;
			train_config.Momentum = momentum;
			train_config.L1Regularization = l1;
			train_config.L2Regularization = l2;
			train_config.VisibleDropout = visible_dropout;
			train_config.HiddenDropout = hidden_dropout;
		}
		cd_schedule = new TrainingSchedule<ContrastiveDivergence>(model_config, minibatch_size);
		cd_schedule->AddTrainingConfig(train_config, epochs);
	}

	static void BuildCD()
	{
		if(cd == nullptr)
		{
			if(cd_schedule == nullptr)
			{
				BuildCDSchedule();
			}

			CD::ModelConfig model_config = cd_schedule->GetModelConfig();
			model_config.VisibleUnits = visible_count;
			assert(cd_schedule->GetMinibatchSize() == minibatch_size);

			RBMProcessor::Model = ModelType::RBM;
			RBMProcessor::VisibleType = (UnitType)model_config.VisibleType;
			RBMProcessor::HiddenType = (UnitType)model_config.HiddenType;
			RBMProcessor::HiddenUnits = model_config.HiddenUnits;

			if(loaded_rbm == nullptr)
			{
				cd = new ContrastiveDivergence(model_config, cd_schedule->GetMinibatchSize());
			}
			else
			{
				assert(loaded_rbm->visible_count == visible_count);
				cd = new ContrastiveDivergence(loaded_rbm, cd_schedule->GetMinibatchSize());
				SAFE_DELETE(loaded_rbm);
			}
		}

		// consume the first training 
		bool training_complete = UpdateSchedule(cd_schedule, cd);
		assert(training_complete == false);
	}

	void RescaleActivations(float* buffer, uint32_t count, UnitType type)
	{
		switch(type)
		{
		case UnitType::Sigmoid:
			break;
		case UnitType::Linear:
			{
				const float factor = logf(127.0f) + logf(2.0f);
				for(uint32_t k = 0; k < count; k++)
				{
					buffer[k] = sigmoid(factor / 3.0f * buffer[k]);
				}
			}
			break;
		case UnitType::RectifiedLinear:
			for(uint32_t k = 0; k < count; k++)
			{
				buffer[k] = std::min(buffer[k] / 5.0f, 1.0f);
			}
			break;
		}
	}

	void RescaleWeights(float* buffer, uint32_t count)
	{
		float stddev = 0.0f;
		for(uint32_t k = 0; k < count; k++)
		{
			const float val = buffer[k];
			stddev += val * val;
		}
		stddev /= count;
		stddev = std::sqrtf(stddev);

		for(uint32_t k = 0; k < count; k++)
		{
			buffer[k] = sigmoid(buffer[k] / (3.0f * stddev));
		}
	}

	static float CapError(float error)
	{
		const float max_val = (float)7.9E+27;

		if(error != error)
		{
			return max_val;
		}

		return Math::Min(error, max_val);
	}

	void RBMProcessor::Run()
	{
		if(SiCKL::OpenGLRuntime::Initialize() == false)
		{
			ShowError("VisualRBM requires a GPU that supports at least OpenGL 3.3.");
			System::Windows::Forms::Application::Exit();
		}

		SiCKL::OpenGLBuffer2D training_example;
		SiCKL::OpenGLBuffer2D validation_example;

		_message_queue = gcnew MessageQueue();

		// update state to alert GUI we can go
		currentState = RBMProcessorState::Ready;
		while(true)
		{
			Message^ msg = _message_queue->Dequeue();

			if(msg != nullptr)
			{
				switch(msg->Type)
				{
				case MessageType::Start:
					{
						assert(epochs > 0);
						assert(training_data != nullptr);
						assert(model_type == ModelType::RBM ||
							   model_type == ModelType::AutoEncoder);

						assert(training_data != nullptr);

						if(training_data->GetIsInitialized())
						{
							training_data->Reset();
						}
						else
						{
							// construct data atlas
							training_data->Initialize(minibatch_size, 512);
						}

						if(validation_data)
						{
							if(validation_data->GetIsInitialized())
							{
								validation_data->Reset();
							}
							else
							{
								validation_data->Initialize(minibatch_size, 512);
							}
						}

						switch(model_type)
						{
						case ModelType::RBM:
							{
								// cd will be null if this is our first 'start' request (ie not coming back from pause)
								if(cd == nullptr)
								{
									BuildCD();
								}
								else if(currentState == RBMProcessorState::Paused)
								{
									SAFE_DELETE(cd_schedule);
									BuildCDSchedule();
									// consume the first training 
									bool training_complete = UpdateSchedule(cd_schedule, cd);
									assert(training_complete == false);
								}
							}
							break;
						case ModelType::AutoEncoder:
							{
								assert(false);
#if 0
								if(bp == nullptr)
								{
									BP::ModelConfig model_config;
									{
										model_config.InputCount = visible_count;
									}

									BP::LayerConfig hidden_config;
									{
										hidden_config.OutputUnits = hidden_count;
										hidden_config.Function = (ActivationFunction_t)hidden_type;
										hidden_config.Noisy = false;
										hidden_config.InputDropoutProbability = visible_dropout;
									}
									BP::LayerConfig output_config;
									{
										output_config.OutputUnits = visible_count;
										output_config.Function = (ActivationFunction_t)visible_type;
										output_config.Noisy = false;
										output_config.InputDropoutProbability = hidden_dropout;
									}

									model_config.LayerConfigs.push_back(hidden_config);
									model_config.LayerConfigs.push_back(output_config);

									bp = new BP(model_config, minibatch_size);
								}

								BP::TrainingConfig train_config;
								{
									train_config.LearningRate = learning_rate;
									train_config.Momentum = momentum;
									train_config.L1Regularization = l1;
									train_config.L2Regularization = l2;
								}
								bp->SetTrainingConfig(train_config);
								bp->SetInputDropoutProbability(0, visible_dropout);
								bp->SetInputDropoutProbability(1, hidden_dropout);
#endif
							}
							break;
						}

						Action^ callback = dynamic_cast<Action^>(msg["callback"]);
						if(callback != nullptr)
						{
							callback();
						}

						currentState = RBMProcessorState::Training;
					}
					break;
				case MessageType::Pause:
					currentState = RBMProcessorState::Paused;
					break;
				case MessageType::Stop:
					SAFE_DELETE(cd);
					SAFE_DELETE(bp);
					SAFE_DELETE(cd_schedule);

					epochs = 100;
					iterations = 0;
					total_iterations = 0;
					currentState = RBMProcessorState::Ready;
					
					break;
				case MessageType::GetVisible:
					if(cd || bp)
					{
						// allocate space for buffers if necessary
						const uint32_t visible_size = visible_count * minibatch_size;
						visible_buffer.Acquire(visible_size);
						visible_recon_buffer.Acquire(visible_size);
						visible_diff_buffer.Acquire(visible_size);

						float* visible_ptr = (float*)visible_buffer;
						float* recon_ptr = (float*)visible_recon_buffer;

						switch(model_type)
						{
						case ModelType::RBM:
							{
								cd->DumpLastVisible(&visible_ptr, &recon_ptr);
							}
							break;
						case ModelType::AutoEncoder:
							{
								bp->DumpLastLabel(&visible_ptr);
								bp->DumpActivation(1, &recon_ptr);
							}
							break;
						}
						
						/// visible diff vis
						{
							// this constant will have sigmoid range from 0 to 1
							const float diff_scale_factor = logf(127.0f) + logf(2.0f);

							float max_val;
							switch(visible_type)
							{
							case UnitType::Sigmoid:
								max_val = 1.0f;
								break;
							case UnitType::Linear:
								max_val = 6.0f;
								break;
							case UnitType::RectifiedLinear:
								max_val = 3.0f;
								break;
							}
						
							for(uint32_t k = 0; k < visible_size; k++)
							{
								visible_diff_buffer[k] = sigmoid( diff_scale_factor / max_val *  (visible_recon_buffer[k] - visible_buffer[k]));
							}
						}
						
						if(model_type == ModelType::AutoEncoder)
						{
							bp->DumpInput(0, &visible_ptr);
						}

						/// visible and recon vis
						RescaleActivations(visible_buffer, visible_count * minibatch_size, visible_type);
						RescaleActivations(visible_recon_buffer, visible_count * minibatch_size, visible_type);

						List<IntPtr>^ visible_list = dynamic_cast<List<IntPtr>^>(msg["visible"]);
						List<IntPtr>^ recon_list = dynamic_cast<List<IntPtr>^>(msg["reconstruction"]);
						List<IntPtr>^ diff_list = dynamic_cast<List<IntPtr>^>(msg["diffs"]);

						for(uint32_t k = 0; k < minibatch_size; k++)
						{
							visible_list->Add(IntPtr((float*)visible_buffer + k * visible_count));
							recon_list->Add(IntPtr((float*)visible_recon_buffer + k * visible_count));
							diff_list->Add(IntPtr((float*)visible_diff_buffer + k * visible_count));
						}
					}
					break;
				case MessageType::GetHidden:
					if(cd || bp)
					{
						const uint32_t hidden_size = hidden_count * minibatch_size;
						hidden_buffer.Acquire(hidden_size);
						float* hidden_ptr = (float*)hidden_buffer;

						switch(model_type)
						{
						case ModelType::RBM:
							{
								cd->DumpLastHidden(&hidden_ptr);
							}
							break;
						case ModelType::AutoEncoder:
							{
								bp->DumpActivation(0, &hidden_ptr);
							}
							break;
						}

						RescaleActivations(hidden_buffer, hidden_count * minibatch_size, hidden_type);

						List<IntPtr>^ hidden_list = dynamic_cast<List<IntPtr>^>(msg["hidden"]);
						for(uint32_t k = 0; k < minibatch_size; k++)
						{
							hidden_list->Add(IntPtr((float*)hidden_buffer + k * hidden_count));
						}
					}
					break;
				case MessageType::GetWeights:
					if(cd || bp)
					{
						List<IntPtr>^ weight_list = dynamic_cast<List<IntPtr>^>(msg["weights"]);
						switch(model_type)
						{
						case ModelType::RBM:
							{
								const uint32_t weight_size = (hidden_count + 1) * (visible_count + 1);
								weight_buffer.Acquire(weight_size);

								float* weight_ptr = (float*)weight_buffer;

								cd->DumpLastWeights(&weight_ptr);
							
								for(uint32_t j = 0; j <= hidden_count; j++)
								{
									weight_list->Add(IntPtr((float*)weight_buffer + 1 + (visible_count + 1) * j));
								}

								RescaleWeights(weight_buffer, weight_size);
							}
							break;
						case ModelType::AutoEncoder:
							{
								const uint32_t weight_size = (hidden_count) * (visible_count + 1);
								weight_buffer.Acquire(weight_size);

								float* weight_ptr = (float*)weight_buffer;
								bp->DumpWeightMatrix(0, &weight_ptr);

								for(uint32_t j = 0; j < hidden_count; j++)
								{
									weight_list->Add(IntPtr((float*)(weight_buffer + 1 + (visible_count + 1) * j)));
								}

								RescaleWeights(weight_buffer, weight_size);
							}
							break;
						}
					}
					break;
				case MessageType::ExportModel:
					{
						Stream^ fs = (Stream^)msg["output_stream"];

						std::string model_json;

						msg["saved"] = false;

						switch(model_type)
						{
						case ModelType::RBM:
							{
								RBM* rbm =  cd->GetRestrictedBoltzmannMachine();
								model_json = rbm->ToJSON();
							}
							break;
						case ModelType::AutoEncoder:
							{
								MLP* mlp = bp->GetMultilayerPerceptron(0, 1);
								model_json = mlp->ToJSON();
							}
							break;
						}

						for(size_t k = 0; k < model_json.size(); k++)
						{
							fs->WriteByte(model_json[k]);
						}

						fs->Flush();
						fs->Close();
						msg["saved"] = true;
					}
					break;
				case MessageType::ImportModel:
					{
						Stream^ fs = (Stream^)msg["input_stream"];
						
						const std::string& model_json = LoadFile(fs);

						msg["loaded"] = false;

						OMLT::Model model;
						if(FromJSON(model_json ,model))
						{
							switch(model.type)
							{
							case OMLT::ModelType::RBM:
								{
									assert(model.rbm != nullptr);
									delete loaded_rbm;
									loaded_rbm = model.rbm;

									msg["loaded"] = true;
								}
								break;
							case OMLT::ModelType::MLP:
								{
#if 0
									MLP* mlp = model.mlp;
									if(mlp->LayerCount() == 2)
									{
										const MLP::Layer* hidden_layer = mlp->GetLayer(0);
										const MLP::Layer* output_layer = mlp->GetLayer(1);

										if(hidden_layer->inputs == visible_count)
										{
											visible_count = hidden_layer->inputs;
											hidden_count = hidden_layer->outputs;
											visible_type = (UnitType)output_layer->function;
											hidden_type = (UnitType)hidden_layer->function;
											model_type = ModelType::AutoEncoder;

											msg["loaded"] = true;

											delete bp;
											bp = new BackPropagation(mlp, minibatch_size);
										}
									}
									delete mlp;
#endif
									assert(false);
								}
								break;
							}
						}
					}
					break;
				case MessageType::LoadTrainingSchedule:
					{
						Stream^ fs = (Stream^)msg["input_stream"];
						std::string schedule_json = LoadFile(fs);

						// see if we can parse it as CD training schedule
						TrainingSchedule<CD>* schedule = TrainingSchedule<CD>::FromJSON(schedule_json);
						if(schedule != nullptr)
						{
							delete cd_schedule;
							cd_schedule = schedule;

							currentState = RBMProcessorState::ScheduleLoaded;
							msg["loaded"] = true;
						}
						else
						{
							msg["loaded"] = false;
						}
					}
					break;
				case MessageType::Shutdown:
					{
						// free objects
						SAFE_DELETE(cd)
						SAFE_DELETE(bp)
						SAFE_DELETE(cd_schedule);
						SAFE_DELETE(training_data)
						SAFE_DELETE(validation_data)
					
						// shutdown opengl
						SiCKL::OpenGLRuntime::Finalize();

						// empty out the message queue
						_message_queue->Clear();
						_message_queue = nullptr;

						// alert caller we're finished
						msg->Handled = true;

						// and exit
						return;
					}
					break;
				}
				msg->Handled = true;
			}
		
			switch(currentState)
			{
			case RBMProcessorState::ScheduleLoaded:
				switch(model_type)
				{
				case ModelType::RBM:
					if(cd == nullptr)
					{
						BuildCD();
					}
					break;
				case ModelType::AutoEncoder:
					assert(false);
					break;
				}
				break;
			case RBMProcessorState::Ready:
			case RBMProcessorState::Paused:
				Thread::Sleep(16);
				break;
			case RBMProcessorState::Training:
				{
					assert(training_data != nullptr);

					// get next training example
					training_data->Next(training_example);
					if(validation_data != nullptr)
					{
						validation_data->Next(validation_example);
					}

					float training_error = -1.0f;
					float validation_error = -1.0f;

					// increment counters
					iterations++;
					total_iterations++;

					switch(model_type)
					{
					case ModelType::RBM:
						{
							assert(cd != nullptr);

							cd->Train(training_example);
							training_error = cd->GetLastReconstructionError();

							// calculate validation error we have a validation set
							if(validation_data != nullptr)
							{
								validation_error = cd->GetReconstructionError(validation_example);
							}
						}
						break;
					case ModelType::AutoEncoder:
						{
							assert(bp != nullptr);
							bp->Train(training_example, training_example);
							training_error = bp->GetLastOutputError();

							if(validation_data != nullptr)
							{
								validation_error = bp->GetOutputError(validation_example, validation_example);
							}
						}
					}

					training_error = CapError(training_error);
					validation_error = CapError(validation_error);

					IterationCompleted(total_iterations, training_error, validation_error);

					iterations = (iterations + 1) % training_data->GetTotalBatches();
					if(iterations == 0)
					{
						EpochCompleted(epochs--);
						switch(model_type)
						{
						case ModelType::RBM:
							if(UpdateSchedule(cd_schedule, cd))
							{
								TrainingCompleted();
								Pause();
								SAFE_DELETE(cd_schedule);
							}
							break;
						case ModelType::AutoEncoder:
							assert(false);
#if 0
							if(UpdateSchedule(bp_schedule, bp))
							{
								TrainingCompleted();
								Pause();
							}
#endif
							break;
						}
					}
				}
				break;
			}
		}
	}

	bool RBMProcessor::SetTrainingData( String^ in_filename )
	{
		IntPtr p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(in_filename);
		char* filename = static_cast<char*>(p.ToPointer());

		IDX* idx = IDX::Load(filename);
		System::Runtime::InteropServices::Marshal::FreeHGlobal(p);

		// verify it's the right data type
		if(idx->GetDataFormat() != DataFormat::Single)
		{
			ShowError("Error: IDX training data must have type 'Float' (0x0D)");
			delete idx;
			return false;
		}

		// clear out the old training data
		delete training_data;
		// construct data atlas
		training_data = new DataAtlas(idx);

		// get visible units required
		visible_count = idx->GetRowLength();


		return true;
	}

	bool RBMProcessor::SetValidationData( String^ in_filename )
	{
		assert(training_data != nullptr);

		// clear out current validation data?
		if(in_filename == nullptr)
		{
			delete validation_data;
			validation_data = nullptr;
			return true;
		}
		// load it up
		else
		{
			IntPtr p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(in_filename);
			char* filename = static_cast<char*>(p.ToPointer());

			IDX* idx = IDX::Load(filename);
			System::Runtime::InteropServices::Marshal::FreeHGlobal(p);

			// verify it's the right data type
			if(idx->GetDataFormat() != DataFormat::Single)
			{
				ShowError("Error: IDX validation data must have type 'Float' (0x0D)");
				delete idx;
				return false;
			}
			// ensure it's same format as training data
			else if(idx->GetRowLength() != visible_count)
			{
				ShowError(String::Format("Error: IDX validation data must have same row length ({0}) as training data", visible_count));
				delete idx;
				return false;
			}

			// clear out old validation data
			delete validation_data;
			// and set new one
			validation_data = new DataAtlas(idx);
			return true;
		}
	}

	bool RBMProcessor::SaveModel(Stream^ in_stream)
	{
		Message^ msg = gcnew Message(MessageType::ExportModel);
		msg["output_stream"] = in_stream;

		_message_queue->Enqueue(msg);
		while(!msg->Handled)
		{
			Thread::Sleep(16);
		}

		return (bool)msg["saved"];
	}

	bool RBMProcessor::LoadModel(Stream^ in_stream)
	{
		Message^ msg = gcnew Message(MessageType::ImportModel);
		msg["input_stream"] = in_stream;

		_message_queue->Enqueue(msg);
		while(!msg->Handled)
		{
			Thread::Sleep(16);
		}

		return (bool)msg["loaded"];
	}

	bool RBMProcessor::LoadTrainingSchedule(Stream^ in_stream)
	{
		Message^ msg = gcnew Message(MessageType::LoadTrainingSchedule);
		msg["input_stream"] = in_stream;

		_message_queue->Enqueue(msg);
		while(!msg->Handled)
		{
			Thread::Sleep(16);
		}

		return (bool)msg["loaded"];
	}

	void RBMProcessor::Start( Action^ act)
	{
		Message^ msg = gcnew Message(MessageType::Start);
		msg["callback"] = act;
		_message_queue->Enqueue(msg);
	}

	void RBMProcessor::Pause()
	{
		_message_queue->Enqueue(gcnew Message(MessageType::Pause));
	}

	void RBMProcessor::Stop()
	{
		_message_queue->Enqueue(gcnew Message(MessageType::Stop));
	}

	void RBMProcessor::Shutdown()
	{
		Message^ msg = gcnew Message(MessageType::Shutdown);

		_message_queue->Enqueue(msg);
		while(!msg->Handled)
		{
			Thread::Sleep(16);
		}
	}

	bool RBMProcessor::GetCurrentVisible(List<IntPtr>^ visible, List<IntPtr>^ reconstruction, List<IntPtr>^ diffs)
	{
		Message^ msg = gcnew Message(MessageType::GetVisible);
		msg["visible"] = visible;
		msg["reconstruction"] = reconstruction;
		msg["diffs"] = diffs;

		_message_queue->Enqueue(msg);
		while(!msg->Handled)
		{
			Thread::Sleep(16);
		}

		assert((visible->Count == minibatch_size &&
			reconstruction->Count == minibatch_size &&
			diffs->Count == minibatch_size) || 
			(visible->Count == 0 && reconstruction->Count == 0 && diffs->Count == 0));

		return visible->Count == minibatch_size;
	}

	bool RBMProcessor::GetCurrentHidden( List<IntPtr>^ hidden_prob )
	{
		Message^ msg = gcnew Message(MessageType::GetHidden);
		msg["hidden"] = hidden_prob;

		_message_queue->Enqueue(msg);
		while(!msg->Handled)
		{
			Thread::Sleep(16);
		}

		assert(hidden_prob->Count == minibatch_size || hidden_prob->Count == 0);

		return hidden_prob->Count == minibatch_size;
	}

	bool RBMProcessor::GetCurrentWeights( List<IntPtr>^ weights )
	{
		Message^ msg = gcnew Message(MessageType::GetWeights);
		msg["weights"] = weights;

		_message_queue->Enqueue(msg);
		while(!msg->Handled)
		{
			Thread::Sleep(16);
		}

		if(cd)
		{
			assert(weights->Count == (hidden_count+1) || weights->Count == 0);
			return weights->Count == (hidden_count+1);
		}
		else if(bp)
		{
			// no bias for auto encoder currently
			assert(weights->Count == hidden_count || weights->Count == 0);
			return weights->Count == hidden_count;
		}
		
	}

#pragma region Properties

	RBMProcessor::RBMProcessorState RBMProcessor::CurrentState::get()
	{
		return currentState;
	}

	bool RBMProcessor::HasTrainingData::get()
	{
		return training_data != nullptr;
	}

	uint32_t RBMProcessor::Epochs::get()
	{
		return epochs;
	}

	void RBMProcessor::Epochs::set( uint32_t e )
	{
		if(e != epochs)
		{
			epochs = e;
			iterations = 0;
			EpochsChanged(e);
		}
	}


	int RBMProcessor::VisibleUnits::get()
	{
		return visible_count;
	}


	int RBMProcessor::HiddenUnits::get()
	{
		return hidden_count;
	}

	void RBMProcessor::HiddenUnits::set( int units )
	{
		assert(units > 0);
		if(units != hidden_count)
		{
			hidden_count = units;
			HiddenUnitsChanged(hidden_count);
		}
	}


	int RBMProcessor::MinibatchSize::get()
	{
		return minibatch_size;
	}

	void RBMProcessor::MinibatchSize::set( int ms )
	{
		assert(ms > 0);
		if(ms != minibatch_size)
		{
			minibatch_size = ms;
			MinibatchSizeChanged(minibatch_size);
		}
	}


	unsigned int RBMProcessor::MinibatchCount::get()
	{
		assert(training_data != nullptr);
		return training_data->GetTotalBatches();
	}

	QuickBoltzmann::ModelType RBMProcessor::Model::get()
	{
		return model_type;
	}

	void RBMProcessor::Model::set(QuickBoltzmann::ModelType in_type)
	{
		if(in_type != model_type)
		{
			model_type = in_type;
			ModelTypeChanged(model_type);
		}
	}

	UnitType RBMProcessor::VisibleType::get()
	{
		return visible_type;
	}

	void RBMProcessor::VisibleType::set( UnitType ut )
	{
		if(ut != visible_type)
		{
			visible_type = ut;
			VisibleTypeChanged(visible_type);
		}
	}

	UnitType RBMProcessor::HiddenType::get()
	{
		return hidden_type;
	}

	void RBMProcessor::HiddenType::set( UnitType ut )
	{
		if(ut != hidden_type)
		{
			hidden_type = ut;
			HiddenTypeChanged(hidden_type);
		}
	}

	float RBMProcessor::LearningRate::get()
	{
		return learning_rate;
	}

	void RBMProcessor::LearningRate::set( float lr )
	{
		assert(lr >= 0.0f);
		if(lr != learning_rate)
		{
			learning_rate = lr;
			LearningRateChanged(learning_rate);
		}
	}


	float RBMProcessor::Momentum::get()
	{
		return momentum;
	}

	void RBMProcessor::Momentum::set( float m )
	{
		assert(m >= 0.0f && m <= 1.0f);
		if(momentum != m)
		{
			momentum = m;
			MomentumChanged(momentum);
		}
	}


	float RBMProcessor::L1Regularization::get()
	{
		return l1;
	}

	void RBMProcessor::L1Regularization::set( float reg )
	{
		assert(reg >= 0.0f);
		if(reg != l1)
		{
			l1 = reg;
			L1RegularizationChanged(l1);
		}
	}


	float RBMProcessor::L2Regularization::get()
	{
		return l2;
	}

	void RBMProcessor::L2Regularization::set( float reg )
	{
		assert(reg >= 0.0f);
		if(reg != l2)
		{
			l2 = reg;
			L2RegularizationChanged(l2);
		}
	}


	float RBMProcessor::VisibleDropout::get()
	{
		return visible_dropout;
	}

	void RBMProcessor::VisibleDropout::set( float vd )
	{
		assert(vd >= 0.0f && vd < 1.0f);
		if(vd != visible_dropout)
		{
			visible_dropout = vd;
			VisibleDropoutChanged(visible_dropout);
		}
	}

	float RBMProcessor::HiddenDropout::get()
	{
		return hidden_dropout;
	}

	void RBMProcessor::HiddenDropout::set( float hd )
	{
		assert(hd >= 0.0f && hd < 1.0f);
		if(hd != hidden_dropout)
		{
			hidden_dropout = hd;
			HiddenDropoutChanged(hidden_dropout);
		}
	}
#pragma endregion
}