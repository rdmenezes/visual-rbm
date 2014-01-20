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

	static class BaseTrainer* trainer = nullptr;

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

#pragma region Data Rescaling Methods

	void RBMProcessor::RescaleDiffs(float* buffer, uint32_t count, UnitType func)
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

		for(uint32_t k = 0; k < count; k++)
		{
			buffer[k] = sigmoid( diff_scale_factor / max_val *  (buffer[k]));
		}
	}

	void RBMProcessor::RescaleActivations(float* buffer, uint32_t count, UnitType type)
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

	void RBMProcessor::RescaleWeights(float* buffer, float stddev, uint32_t count)
	{
		for(uint32_t k = 0; k < count; k++)
		{
			buffer[k] = sigmoid(buffer[k] / (3.0f * stddev));
		}
	}

#pragma endregion

	static float CapError(float error)
	{
		const float max_val = (float)7.9E+27;

		if(error != error)
		{
			return max_val;
		}

		return Math::Min(error, max_val);
	}

	class BaseTrainer
	{
	public:
		virtual ~BaseTrainer() {};
		virtual void HandleStartMsg(Message^ msg) = 0;
		virtual void HandleStopMsg(Message^ msg) = 0;
		virtual void HandleGetVisibleMsg(Message^ msg) = 0;
		virtual void HandleGetHiddenMsg(Message^ msg) = 0;
		virtual void HandleGetWeightsMsg(Message^ msg) = 0;
		virtual void HandleExportModel(Stream^ s) = 0;
		virtual void HandleImportModel(OMLT::Model& model) = 0;
		virtual void HandleLoadScheduleMsg(Message^ msg) = 0;
		virtual float Train(OpenGLBuffer2D& train_example) = 0;
		virtual float Validation(OpenGLBuffer2D& validation_example) = 0;
		// returns true if training schedule is complete
		virtual bool HandleEpochCompleted() = 0;

	};

	class RBMTrainer : public BaseTrainer
	{
	public:
		RBMTrainer() : _cd(nullptr), _schedule(nullptr), _loaded_rbm(nullptr) {}

		virtual ~RBMTrainer()
		{
			assert(currentState == RBMProcessor::RBMProcessorState::Stopped);
			SAFE_DELETE(_cd);
			SAFE_DELETE(_schedule);
			SAFE_DELETE(_loaded_rbm);
		}

		virtual void HandleStartMsg( Message^ msg ) 
		{
			switch(currentState)
			{
			case RBMProcessor::RBMProcessorState::Paused:
				{
					assert(_cd != nullptr);
					assert(_schedule != nullptr);

					SAFE_DELETE(_schedule);
					build_schedule();
				
					currentState = RBMProcessor::RBMProcessorState::Running;
				}
				break;
			case RBMProcessor::RBMProcessorState::Stopped:
				{
					assert(_cd == nullptr);
					assert(_schedule == nullptr);

					build_schedule();
					build_cd();

					currentState = RBMProcessor::RBMProcessorState::Running;
				}
				break;
			case RBMProcessor::RBMProcessorState::ScheduleLoaded:
				{
					assert(_schedule != nullptr);
					assert(_cd != nullptr);

					currentState = RBMProcessor::RBMProcessorState::ScheduleRunning;
				}
				break;
			default:
				assert(false);
			}

			assert(_schedule != nullptr);

			_schedule->StartTraining();
			// verify we have a valid constructed schedule
			assert(_schedule->TrainingComplete() == false);

			// populate CD with our new training parameters
			CD::TrainingConfig config;
			bool populated = _schedule->GetTrainingConfig(config);
			assert(populated == true);

			// update CD with our new training parameters
			_cd->SetTrainingConfig(config);
		}

		virtual void HandleStopMsg( Message^ msg ) 
		{
			SAFE_DELETE(_cd);
			SAFE_DELETE(_schedule);
			currentState = RBMProcessor::RBMProcessorState::Stopped;
		}

		virtual void HandleGetVisibleMsg( Message^ msg ) 
		{
			assert(currentState == RBMProcessor::RBMProcessorState::Running ||
				currentState == RBMProcessor::RBMProcessorState::ScheduleRunning);

			assert(_cd != nullptr);

			// allocate buffer space if need be
			const uint32_t visible_size = visible_count * minibatch_size;
			visible_buffer.Acquire(visible_size);
			visible_recon_buffer.Acquire(visible_size);
			visible_diff_buffer.Acquire(visible_size);

			float* visible_ptr = (float*)visible_buffer;
			float* recon_ptr = (float*)visible_recon_buffer;
			float* diff_ptr = (float*)visible_diff_buffer;

			_cd->DumpLastVisible(&visible_ptr, &recon_ptr);

			for(uint32_t k = 0;  k < visible_count * minibatch_size; k++)
			{
				diff_ptr[k] = recon_ptr[k] - visible_ptr[k];
			}

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

		virtual void HandleGetHiddenMsg( Message^ msg ) 
		{
			assert(currentState == RBMProcessor::RBMProcessorState::Running ||
				currentState == RBMProcessor::RBMProcessorState::ScheduleRunning);

			assert(_cd != nullptr);

			// allocate buffer space if need be
			const uint32_t hidden_size = hidden_count * minibatch_size;
			hidden_buffer.Acquire(hidden_size);
			float* hidden_ptr = (float*)hidden_buffer;

			_cd->DumpLastHidden(&hidden_ptr);

			List<IntPtr>^ hidden_list = dynamic_cast<List<IntPtr>^>(msg["hidden"]);
			for(uint32_t k = 0; k < minibatch_size; k++)
			{
				hidden_list->Add(IntPtr((float*)hidden_buffer + k * hidden_count));
			}
		}

		virtual void HandleGetWeightsMsg( Message^ msg ) 
		{
			assert(currentState == RBMProcessor::RBMProcessorState::Running ||
				currentState == RBMProcessor::RBMProcessorState::ScheduleRunning);

			assert(_cd != nullptr);

			// allocate buffer space if need be
			const uint32_t weight_size = (hidden_count + 1) * (visible_count + 1);
			weight_buffer.Acquire(weight_size);

			float* weight_ptr = (float*)weight_buffer;

			_cd->DumpLastWeights(&weight_ptr);

			List<IntPtr>^ weight_list = dynamic_cast<List<IntPtr>^>(msg["weights"]);
			for(uint32_t j = 0; j <= hidden_count; j++)
			{
				weight_list->Add(IntPtr(weight_ptr + 1 + (visible_count + 1) * j));
			}
		}

		virtual void HandleExportModel(Stream^ s)
		{
			assert(_cd != nullptr);
			
			RBM* rbm = _cd->GetRestrictedBoltzmannMachine();
			assert(rbm != nullptr);
			std::string model_json = rbm->ToJSON();

			for(size_t k = 0; k < model_json.size(); k++)
			{
				s->WriteByte(model_json[k]);
			}
		}

		virtual void HandleImportModel(OMLT::Model& model)
		{
			assert(model.type == OMLT::ModelType::RBM);
			SAFE_DELETE(_loaded_rbm);
			_loaded_rbm = model.rbm;
		}

		virtual void HandleLoadScheduleMsg( Message^ msg)
		{
			IntPtr ptr = (IntPtr)msg["schedule"];
			assert(ptr.ToPointer() != nullptr);
			
			SAFE_DELETE(_schedule);
			
			_schedule = (TrainingSchedule<CD>*)ptr.ToPointer();

			_schedule->StartTraining();

			CD::ModelConfig model_config = _schedule->GetModelConfig();
			model_config.VisibleUnits = visible_count;
			model_config_to_ui(model_config);

			CD::TrainingConfig train_config;
			bool populated = _schedule->GetTrainingConfig(train_config);
			assert(populated == true);
			train_config_to_ui(train_config);

			minibatches_to_ui(_schedule->GetMinibatchSize());
			epochs_to_ui(_schedule->GetEpochs());

			build_cd();

			currentState = RBMProcessor::RBMProcessorState::ScheduleLoaded;
		}

		virtual float Train( OpenGLBuffer2D& train_example ) 
		{
			assert(currentState == RBMProcessor::RBMProcessorState::Running ||
				currentState == RBMProcessor::RBMProcessorState::ScheduleRunning);
			
			_cd->Train(train_example);
			return _cd->GetLastReconstructionError();
		}

		virtual float Validation( OpenGLBuffer2D& validation_example ) 
		{
			assert(currentState == RBMProcessor::RBMProcessorState::Running ||
				currentState == RBMProcessor::RBMProcessorState::ScheduleRunning);

			return _cd->GetReconstructionError(validation_example);
		}


		virtual bool HandleEpochCompleted()
		{
			// done with epochs?
			if(_schedule->NextEpoch())
			{
				// done with training?
				if(_schedule->TrainingComplete())
				{
					return true;
				}

				// new training config to use
				CD::TrainingConfig train_config;
				bool populated = _schedule->GetTrainingConfig(train_config);
				assert(populated);

				_cd->SetTrainingConfig(train_config);

				// now update our UI
				assert(populated == true);
				train_config_to_ui(train_config);

				epochs_to_ui(_schedule->GetEpochs());

			}
			return false;
		}
	private:

		void build_cd()
		{
			assert(_cd == nullptr);
			assert(_schedule != nullptr);

			CD::ModelConfig model_config = _schedule->GetModelConfig();
			model_config.VisibleUnits = visible_count;
			assert(_schedule->GetMinibatchSize() == minibatch_size);

			RBMProcessor::Model = ModelType::RBM;
			RBMProcessor::VisibleType = (UnitType)model_config.VisibleType;
			RBMProcessor::HiddenType = (UnitType)model_config.HiddenType;
			RBMProcessor::HiddenUnits = model_config.HiddenUnits;

			if(_loaded_rbm == nullptr)
			{
				_cd = new ContrastiveDivergence(model_config, _schedule->GetMinibatchSize());
			}
			else
			{
				assert(_loaded_rbm->visible_count == visible_count);
				_cd = new ContrastiveDivergence(_loaded_rbm, _schedule->GetMinibatchSize());
				SAFE_DELETE(_loaded_rbm);
			}
		}

		void build_schedule()
		{
			assert(_schedule == nullptr);
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

			_schedule = new TrainingSchedule<ContrastiveDivergence>(model_config, minibatch_size);
			_schedule->AddTrainingConfig(train_config, epochs);
		}

		void model_config_to_ui(const CD::ModelConfig& model_config)
		{
			RBMProcessor::Model = ModelType::RBM;
			RBMProcessor::VisibleType = (UnitType)model_config.VisibleType;
			RBMProcessor::HiddenType = (UnitType)model_config.HiddenType;
			RBMProcessor::HiddenUnits = model_config.HiddenUnits;
		}

		void train_config_to_ui(const CD::TrainingConfig& train_config)
		{
			RBMProcessor::LearningRate = train_config.LearningRate;
			RBMProcessor::Momentum = train_config.Momentum;
			RBMProcessor::L1Regularization = train_config.L1Regularization;
			RBMProcessor::L2Regularization = train_config.L2Regularization;
			RBMProcessor::VisibleDropout = train_config.VisibleDropout;
			RBMProcessor::HiddenDropout = train_config.HiddenDropout;
		}

		void epochs_to_ui(uint32_t epochs)
		{
			RBMProcessor::Epochs = epochs;
		}

		void minibatches_to_ui(uint32_t minibatches)
		{
			RBMProcessor::MinibatchSize = minibatches;
		}

		ContrastiveDivergence* _cd;
		TrainingSchedule<ContrastiveDivergence>* _schedule;
		RBM* _loaded_rbm;
	};

	class AETrainer
	{

	};

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
		currentState = RBMProcessorState::Stopped;
		// start with an RBMTrainer by default
		trainer = new RBMTrainer();

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

						trainer->HandleStartMsg(msg);

						Action^ callback = dynamic_cast<Action^>(msg["callback"]);
						if(callback != nullptr)
						{
							callback();
						}
					}
					break;
				case MessageType::Pause:
					assert(currentState == RBMProcessorState::Running ||
					       currentState == RBMProcessorState::ScheduleRunning);
					currentState = RBMProcessorState::Paused;
					break;
				case MessageType::Stop:
					iterations = 0;
					total_iterations = 0;
					Epochs = 100;
					trainer->HandleStopMsg(msg);
					break;
				case MessageType::GetVisible:
					if(currentState == RBMProcessor::RBMProcessorState::Running || currentState == RBMProcessor::RBMProcessorState::ScheduleRunning)
					{
						trainer->HandleGetVisibleMsg(msg);
					}
					break;
				case MessageType::GetHidden:
					if(currentState == RBMProcessor::RBMProcessorState::Running || currentState == RBMProcessor::RBMProcessorState::ScheduleRunning)
					{
						trainer->HandleGetHiddenMsg(msg);
					}
					break;
				case MessageType::GetWeights:
					if(currentState == RBMProcessor::RBMProcessorState::Running || currentState == RBMProcessor::RBMProcessorState::ScheduleRunning)
					{
						trainer->HandleGetWeightsMsg(msg);
					}
					break;
				case MessageType::ExportModel:
					{
						msg["saved"] = false;

						Stream^ fs = (Stream^)msg["output_stream"];

						trainer->HandleExportModel(fs);

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
								RBMProcessor::Model = ModelType::RBM;
								break;
							case OMLT::ModelType::MLP:
								assert(false);
								break;
							}

							trainer->HandleImportModel(model);
							msg["loaded"] = true;
						}
					}
					break;
				case MessageType::LoadSchedule:
					{
						Stream^ fs = (Stream^)msg["input_stream"];
						std::string schedule_json = LoadFile(fs);

						msg["loaded"] = false;

						// see if we can parse it as CD training schedule
						if(TrainingSchedule<CD>* schedule = TrainingSchedule<CD>::FromJSON(schedule_json))
						{
							msg["schedule"] = IntPtr(schedule);
							
							RBMProcessor::Model = ModelType::RBM;
							trainer->HandleLoadScheduleMsg(msg);
							msg["loaded"] = true;
						}
					}
					break;
				case MessageType::Shutdown:
					{
						// free objects
						SAFE_DELETE(trainer);
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
			case RBMProcessorState::Stopped:
			case RBMProcessorState::Paused:
				Thread::Sleep(16);
				break;
			case RBMProcessorState::Running:
			case RBMProcessorState::ScheduleRunning:
				{
					assert(training_data != nullptr);

					float training_error = -1.0f;
					float validation_error = -1.0f;

					// get next training example
					training_data->Next(training_example);
					training_error = trainer->Train(training_example);
					if(validation_data != nullptr)
					{
						validation_data->Next(validation_example);
						validation_error = trainer->Validation(validation_example);
					}

					// increment iteration counters
					iterations++;
					total_iterations++;

					training_error = CapError(training_error);
					validation_error = CapError(validation_error);

					IterationCompleted(total_iterations, training_error, validation_error);

					iterations = (iterations + 1) % training_data->GetTotalBatches();
					if(iterations == 0)
					{
						EpochCompleted(epochs--);
						if(trainer->HandleEpochCompleted())
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
		Message^ msg = gcnew Message(MessageType::LoadSchedule);
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

		

		if(model_type == QuickBoltzmann::ModelType::RBM)
		{
			assert(weights->Count == (hidden_count+1) || weights->Count == 0);
			return weights->Count == (hidden_count+1);
		}
		else if(model_type == QuickBoltzmann::ModelType::AutoEncoder)
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

			switch(model_type)
			{
			case ModelType::RBM:
				SAFE_DELETE(trainer);
				trainer = new RBMTrainer();
				break;
			case ModelType::AutoEncoder:
				SAFE_DELETE(trainer);
				assert(false);
				break;
			}
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