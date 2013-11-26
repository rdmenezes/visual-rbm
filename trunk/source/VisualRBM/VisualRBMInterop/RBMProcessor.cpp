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
#include <IDX.hpp>


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

	// Backend Static Data
	static OMLT::ContrastiveDivergence* cd = nullptr;
	static OMLT::BackPropagation* bp = nullptr;
	static OMLT::DataAtlas* training_data = nullptr;
	static OMLT::DataAtlas* validation_data = nullptr;

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
	OMLT::AlignedMemoryBlock<float> visible_buffer;
	OMLT::AlignedMemoryBlock<float> visible_recon_buffer;
	OMLT::AlignedMemoryBlock<float> visible_diff_buffer;
	OMLT::AlignedMemoryBlock<float> hidden_buffer;
	OMLT::AlignedMemoryBlock<float> weight_buffer;

	// brings up a message box with an error message
	static inline void ShowError(String^ error)
	{
		System::Windows::Forms::MessageBox::Show(error, "Error", System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Error);
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
		_currentState = RBMProcessorState::Ready;
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
									// setup model config
									OMLT::CD::ModelConfig m_config;
									{
										m_config.VisibleUnits = visible_count;
										m_config.HiddenUnits = hidden_count;
										m_config.VisibleType = (OMLT::ActivationFunction_t)visible_type;
										m_config.HiddenType = (OMLT::ActivationFunction_t)hidden_type;
									}

									cd = new OMLT::ContrastiveDivergence(m_config, minibatch_size);
								}

								OMLT::CD::TrainingConfig t_config;
								{
									t_config.LearningRate = learning_rate;
									t_config.Momentum = momentum;
									t_config.L1Regularization = l1;
									t_config.L2Regularization = l2;
									t_config.VisibleDropout = visible_dropout;
									t_config.HiddenDropout = hidden_dropout;
								}
								cd->SetTrainingConfig(t_config);
							}
							break;
						case ModelType::AutoEncoder:
							{
								if(bp == nullptr)
								{
									OMLT::BP::ModelConfig model_config;
									{
										model_config.InputCount = visible_count;
									}

									OMLT::BP::LayerConfig hidden_config;
									{
										hidden_config.OutputUnits = hidden_count;
										hidden_config.Function = (OMLT::ActivationFunction_t)hidden_type;
										hidden_config.Noisy = false;
										hidden_config.InputDropoutProbability = visible_dropout;
									}
									OMLT::BP::LayerConfig output_config;
									{
										output_config.OutputUnits = visible_count;
										output_config.Function = (OMLT::ActivationFunction_t)visible_type;
										output_config.Noisy = false;
										output_config.InputDropoutProbability = hidden_dropout;
									}

									model_config.LayerConfigs.push_back(hidden_config);
									model_config.LayerConfigs.push_back(output_config);

									bp = new OMLT::BP(model_config, minibatch_size);
								}

								OMLT::BP::TrainingConfig train_config;
								{
									train_config.LearningRate = learning_rate;
									train_config.Momentum = momentum;
									train_config.L1Regularization = l1;
									train_config.L2Regularization = l2;
								}
								bp->SetTrainingConfig(train_config);
								bp->SetInputDropoutProbability(0, visible_dropout);
								bp->SetInputDropoutProbability(1, hidden_dropout);
							}
							break;
						}

						Action^ callback = dynamic_cast<Action^>(msg["callback"]);
						if(callback != nullptr)
						{
							callback();
						}

						_currentState = RBMProcessorState::Training;
					}
					break;
				case MessageType::Pause:
					_currentState = RBMProcessorState::Paused;
					break;
				case MessageType::Stop:

					switch(model_type)
					{
					case ModelType::RBM:
						{
							SAFE_DELETE(cd);
						}
						break;
					case ModelType::AutoEncoder:
						{
							SAFE_DELETE(bp);
						}
						break;
					}
					epochs = 100;
					iterations = 0;
					total_iterations = 0;
					_currentState = RBMProcessorState::Ready;
					
					break;
				case MessageType::GetVisible:
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
								bp->DumpInput(0, &visible_ptr);
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

						msg["done"] = true;
					}
					break;
				case MessageType::GetHidden:
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

						msg["done"] = true;
					}
					break;
				case MessageType::GetWeights:
					switch(model_type)
					{
					case ModelType::RBM:
						{
							const uint32_t weight_size = (hidden_count + 1) * (visible_count + 1);
							weight_buffer.Acquire(weight_size);

							float* weight_ptr = (float*)weight_buffer;

							cd->DumpLastWeights(&weight_ptr);
							
							RescaleWeights(weight_buffer, weight_size);

							List<IntPtr>^ weight_list = dynamic_cast<List<IntPtr>^>(msg["weights"]);
							for(uint32_t j = 0; j <= hidden_count; j++)
							{
								weight_list->Add(IntPtr((float*)weight_buffer + 1 + (visible_count + 1) * j));
							}
						}
						msg["done"] = true;
						break;
					case ModelType::AutoEncoder:
						{

						}
						break;
					}
					break;
				case MessageType::ExportModel:
					switch(model_type)
					{
					case ModelType::RBM:
						{
							OMLT::RBM* rbm =  cd->GetRestrictedBoltzmannMachine();
							const std::string& rbm_json = rbm->ToJSON();

							Stream^ fs = (Stream^)msg["output_stream"];

							for(size_t k = 0; k < rbm_json.size(); k++)
							{
								fs->WriteByte(rbm_json[k]);
							}

							fs->Flush();
							fs->Close();
						}
						break;
					case ModelType::AutoEncoder:
						{

						}
						break;
					}
					break;
				case MessageType::ImportModel:
					{
						Stream^ fs = (Stream^)msg["input_stream"];

						// load rbm from disk
						std::stringstream ss;
						for(int32_t b = fs->ReadByte(); b >= 0; b = fs->ReadByte())
						{
							ss << (char)b;
						}
						fs->Close();

						const std::string& rbm_json = ss.str();

						// try parsing an RBM
						OMLT::RBM* rbm = OMLT::RBM::FromJSON(rbm_json);

						// success?
						if(rbm)
						{
							// ensure it has same dimensions as our loaded data
							if(rbm->visible_count == visible_count)
							{
								visible_count = rbm->visible_count;
								hidden_count = rbm->hidden_count;
								visible_type = (UnitType)rbm->visible_type;
								hidden_type = (UnitType)rbm->hidden_type;
								model_type = ModelType::RBM;
								
								msg["loaded"] = true;

								// create new CD from RBM
								delete cd;
								cd = new OMLT::ContrastiveDivergence(rbm, minibatch_size);
							}
							else
							{
								msg["loaded"] = false;
							}
						}
						else
						{
							msg["loaded"] = false;
						}

						// message we're done
						msg["done"] = true;
					}
					break;
				case MessageType::Shutdown:
					{
						// free objects
						SAFE_DELETE(cd)
						SAFE_DELETE(bp)
						SAFE_DELETE(training_data)
						SAFE_DELETE(validation_data)
					
						// shutdown opengl
						SiCKL::OpenGLRuntime::Finalize();

						// empty out the message queue
						_message_queue->Clear();
						_message_queue = nullptr;

						// alert caller that we're done
						msg["done"] = true;

						// and exit
						return;
					}
					break;
				}
			}
		
			switch(_currentState)
			{
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

						}
					}

					IterationCompleted(total_iterations, training_error, validation_error);

					if(iterations == training_data->GetTotalBatches())
					{
						iterations = 0;
						EpochCompleted(epochs--);

						if(epochs == 0)
						{
							TrainingCompleted();
							Pause();
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

		OMLT::IDX* idx = OMLT::IDX::Load(filename);
		System::Runtime::InteropServices::Marshal::FreeHGlobal(p);

		// verify it's the right data type
		if(idx->GetDataFormat() != OMLT::DataFormat::Single)
		{
			ShowError("Error: IDX training data must have type 'Float' (0x0D)");
			delete idx;
			return false;
		}

		// clear out the old training data
		delete training_data;
		// construct data atlas
		training_data = new OMLT::DataAtlas(idx);

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

			OMLT::IDX* idx = OMLT::IDX::Load(filename);
			System::Runtime::InteropServices::Marshal::FreeHGlobal(p);

			// verify it's the right data type
			if(idx->GetDataFormat() != OMLT::DataFormat::Single)
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
			validation_data = new OMLT::DataAtlas(idx);
			return true;
		}
	}

	void RBMProcessor::SaveModel(Stream^ in_stream)
	{
		Message^ msg = gcnew Message(MessageType::ExportModel);
		msg["output_stream"] = in_stream;
		_message_queue->Enqueue(msg);
	}

	bool RBMProcessor::LoadModel(Stream^ in_stream)
	{
		Message^ msg = gcnew Message(MessageType::ImportModel);
		msg["input_stream"] = in_stream;
		msg["done"] = false;

		_message_queue->Enqueue(msg);
		while((bool)msg["done"] == false)
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
		msg["done"] = false;

		_message_queue->Enqueue(msg);
		while((bool)msg["done"] == false)
		{
			Thread::Sleep(16);
		}
	}

	void RBMProcessor::GetCurrentVisible(List<IntPtr>^ visible, List<IntPtr>^ reconstruction, List<IntPtr>^ diffs)
	{
		Message^ msg = gcnew Message(MessageType::GetVisible);
		msg["done"] = false;
		msg["visible"] = visible;
		msg["reconstruction"] = reconstruction;
		msg["diffs"] = diffs;

		_message_queue->Enqueue(msg);
		while((bool)msg["done"] == false)
		{
			Thread::Sleep(16);
		}
	}

	void RBMProcessor::GetCurrentHidden( List<IntPtr>^ hidden_prob )
	{
		Message^ msg = gcnew Message(MessageType::GetHidden);
		msg["done"] = false;
		msg["hidden"] = hidden_prob;

		_message_queue->Enqueue(msg);
		while((bool)msg["done"] == false)
		{
			Thread::Sleep(16);
		}
	}

	void RBMProcessor::GetCurrentWeights( List<IntPtr>^ weights )
	{
		Message^ msg = gcnew Message(MessageType::GetWeights);
		msg["done"] = false;
		msg["weights"] = weights;

		_message_queue->Enqueue(msg);
		while((bool)msg["done"] == false)
		{
			Thread::Sleep(16);
		}
	}

#pragma region Properties

	bool RBMProcessor::HasTrainingData::get()
	{
		return training_data != nullptr;
	}

	bool RBMProcessor::HasValidationData::get()
	{
		return validation_data != nullptr;
	}

	uint32_t RBMProcessor::Epochs::get()
	{
		return epochs;
	}

	void RBMProcessor::Epochs::set( uint32_t e )
	{
		epochs = e;
		iterations = 0;
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
		hidden_count = units;
	}


	int RBMProcessor::MinibatchSize::get()
	{
		return minibatch_size;
	}

	void RBMProcessor::MinibatchSize::set( int ms )
	{
		assert(ms > 0);
		minibatch_size = ms;
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
		model_type = in_type;
	}

	UnitType RBMProcessor::VisibleType::get()
	{
		return visible_type;
	}

	void RBMProcessor::VisibleType::set( UnitType ut )
	{
		visible_type = ut;
	}

	UnitType RBMProcessor::HiddenType::get()
	{
		return hidden_type;
	}

	void RBMProcessor::HiddenType::set( UnitType ut )
	{
		hidden_type = ut;
	}

	float RBMProcessor::LearningRate::get()
	{
		return learning_rate;
	}

	void RBMProcessor::LearningRate::set( float lr )
	{
		assert(lr >= 0.0f);
		learning_rate = lr;
	}


	float RBMProcessor::Momentum::get()
	{
		return momentum;
	}

	void RBMProcessor::Momentum::set( float m )
	{
		assert(m >= 0.0f && m <= 1.0f);
		momentum = m;
	}


	float RBMProcessor::L1Regularization::get()
	{
		return l1;
	}

	void RBMProcessor::L1Regularization::set( float reg )
	{
		assert(reg >= 0.0f);
		l1 = reg;
	}


	float RBMProcessor::L2Regularization::get()
	{
		return l2;
	}

	void RBMProcessor::L2Regularization::set( float reg )
	{
		assert(reg >= 0.0f);
		l2 = reg;
	}


	float RBMProcessor::VisibleDropout::get()
	{
		return visible_dropout;
	}

	void RBMProcessor::VisibleDropout::set( float vd )
	{
		assert(vd >= 0.0f && vd < 1.0f);
		visible_dropout = vd;
	}

	float RBMProcessor::HiddenDropout::get()
	{
		return hidden_dropout;
	}

	void RBMProcessor::HiddenDropout::set( float hd )
	{
		assert(hd >= 0.0f && hd < 1.0f);
		hidden_dropout = hd;
	}
#pragma endregion
}