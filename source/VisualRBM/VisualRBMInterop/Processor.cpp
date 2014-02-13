// libc
#include <assert.h>

// stdc
#include <sstream>
#include <cstring>
#include <cmath>

// VisualRBM Interop
#include "Processor.h"
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

namespace VisualRBMInterop
{
	enum ProcessorState
	{
		Invalid = -1,
		Running,
		Paused,
		Stopped, 
		ScheduleLoaded,
		ScheduleRunning,
	};


	static ProcessorState currentState = ProcessorState::Invalid;

	// Backend Static Data
	static DataAtlas* training_data = nullptr;
	static DataAtlas* validation_data = nullptr;

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
	
	static uint32_t epochs_to_train = 100;
	static uint32_t epochs_remaining = 100;
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

	void Processor::RescaleDiffs(float* buffer, uint32_t count, UnitType func)
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

	void Processor::RescaleActivations(float* buffer, uint32_t count, UnitType type)
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

	void Processor::RescaleWeights(float* buffer, float stddev, uint32_t count)
	{
		for(uint32_t k = 0; k < count; k++)
		{
			buffer[k] = sigmoid(buffer[k] / (3.0f * stddev));
		}
	}

#pragma endregion

	void InitializeDataAtlas()
	{
		if(training_data->GetBatchSize() == minibatch_size)
		{
			training_data->Reset();
		}
		else
		{
			training_data->Initialize(minibatch_size, 512);
		}

		if(validation_data)
		{
			if(validation_data->GetBatchSize() == minibatch_size)
			{
				validation_data->Reset();
			}
			else
			{
				validation_data->Initialize(minibatch_size, 512);
			}
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
		virtual bool HandleImportModel(OMLT::Model& model) = 0;
		virtual void HandleLoadScheduleMsg(Message^ msg) = 0;
		virtual float Train(OpenGLBuffer2D& train_example) = 0;
		virtual float Validation(OpenGLBuffer2D& validation_example) = 0;
		// returns true if training schedule is complete
		virtual bool HandleEpochCompleted() = 0;

	};

	template<class MODEL, class TRAINER>
	class ITrainer : public BaseTrainer
	{
	public:
		ITrainer() : _trainer(nullptr), _schedule(nullptr), _loaded_model(nullptr) {}
		virtual ~ITrainer()
		{
			assert(currentState == ProcessorState::Stopped);
			SAFE_DELETE(_trainer);
			SAFE_DELETE(_schedule);
			SAFE_DELETE(_loaded_model);
		};
		virtual void HandleStartMsg( Message^ msg )
		{
			switch(currentState)
			{
			case ProcessorState::Paused:
				{
					assert(_trainer != nullptr);
					assert(_schedule != nullptr);

					SAFE_DELETE(_schedule);
					build_schedule();

					currentState = ProcessorState::Running;
				}
				break;
			case ProcessorState::Stopped:
				{
					assert(_trainer == nullptr);
					assert(_schedule == nullptr);

					build_schedule();
					build_trainer();

					InitializeDataAtlas();

					currentState = ProcessorState::Running;
				}
				break;
			case ProcessorState::ScheduleLoaded:
				{
					assert(_schedule != nullptr);
					assert(_trainer != nullptr);

					InitializeDataAtlas();

					currentState = ProcessorState::ScheduleRunning;
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
			struct TRAINER::TrainingConfig config;
			bool populated = _schedule->GetTrainingConfig(config);
			assert(populated == true);

			// update CD with our new training parameters
			_trainer->SetTrainingConfig(config);
		}
		virtual void HandleStopMsg( Message^ msg )
		{
			SAFE_DELETE(_trainer);
			SAFE_DELETE(_schedule);
			currentState = ProcessorState::Stopped;
		}
		virtual void HandleLoadScheduleMsg( Message^ msg)
		{
			IntPtr ptr = (IntPtr)msg["schedule"];
			assert(ptr.ToPointer() != nullptr);

			SAFE_DELETE(_schedule);

			_schedule = (TrainingSchedule<TRAINER>*)ptr.ToPointer();

			_schedule->StartTraining();

			struct TRAINER::ModelConfig model_config = _schedule->GetModelConfig();
			model_config_to_ui(model_config);

			struct TRAINER::TrainingConfig train_config;
			bool populated = _schedule->GetTrainingConfig(train_config);
			assert(populated == true);
			train_config_to_ui(train_config);

			minibatches_to_ui(_schedule->GetMinibatchSize());
			epochs_to_ui(_schedule->GetEpochs());

			build_trainer();

			currentState = ProcessorState::ScheduleLoaded;
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
				struct TRAINER::TrainingConfig train_config;
				bool populated = _schedule->GetTrainingConfig(train_config);
				assert(populated);

				_trainer->SetTrainingConfig(train_config);

				// now update our UI
				assert(populated == true);
				train_config_to_ui(train_config);

				epochs_to_ui(_schedule->GetEpochs());

			}
			return false;
		}
	protected:

		void build_schedule();
		void build_trainer();

		void model_config_to_ui(const struct TRAINER::ModelConfig& model_config);
		void train_config_to_ui(const struct TRAINER::TrainingConfig& train_config);

		void epochs_to_ui(uint32_t epochs)
		{
			Processor::Epochs = epochs;
		}
		void minibatches_to_ui(uint32_t minibatches)
		{
			Processor::MinibatchSize = minibatches;
		}

		TRAINER* _trainer;
		TrainingSchedule<TRAINER>* _schedule;
		MODEL* _loaded_model;
	};

	class RBMTrainer : public ITrainer<RBM,ContrastiveDivergence>
	{
		virtual void HandleGetVisibleMsg( Message^ msg ) 
		{
			assert(currentState == ProcessorState::Running ||
				currentState == ProcessorState::ScheduleRunning);

			assert(_trainer != nullptr);

			// allocate buffer space if need be
			const uint32_t visible_size = visible_count * minibatch_size;
			visible_buffer.Acquire(visible_size);
			visible_recon_buffer.Acquire(visible_size);
			visible_diff_buffer.Acquire(visible_size);

			float* visible_ptr = (float*)visible_buffer;
			float* recon_ptr = (float*)visible_recon_buffer;
			float* diff_ptr = (float*)visible_diff_buffer;

			_trainer->DumpLastVisible(&visible_ptr, &recon_ptr);

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
			assert(currentState == ProcessorState::Running ||
				currentState == ProcessorState::ScheduleRunning);

			assert(_trainer != nullptr);

			// allocate buffer space if need be
			const uint32_t hidden_size = hidden_count * minibatch_size;
			hidden_buffer.Acquire(hidden_size);
			float* hidden_ptr = (float*)hidden_buffer;

			_trainer->DumpLastHidden(&hidden_ptr);

			List<IntPtr>^ hidden_list = dynamic_cast<List<IntPtr>^>(msg["hidden"]);
			for(uint32_t k = 0; k < minibatch_size; k++)
			{
				hidden_list->Add(IntPtr((float*)hidden_buffer + k * hidden_count));
			}
		}
		virtual void HandleGetWeightsMsg( Message^ msg ) 
		{
			assert(currentState == ProcessorState::Running ||
				currentState == ProcessorState::ScheduleRunning);

			assert(_trainer != nullptr);

			// allocate buffer space if need be
			const uint32_t weight_size = (hidden_count + 1) * (visible_count + 1);
			weight_buffer.Acquire(weight_size);

			float* weight_ptr = (float*)weight_buffer;

			_trainer->DumpLastWeights(&weight_ptr);

			List<IntPtr>^ weight_list = dynamic_cast<List<IntPtr>^>(msg["weights"]);
			for(uint32_t j = 0; j <= hidden_count; j++)
			{
				weight_list->Add(IntPtr(weight_ptr + 1 + (visible_count + 1) * j));
			}
		}
		virtual void HandleExportModel(Stream^ s)
		{
			assert(_trainer != nullptr);

			RBM* rbm = _trainer->GetRestrictedBoltzmannMachine();
			assert(rbm != nullptr);
			std::string model_json = rbm->ToJSON();
			delete rbm;
			for(size_t k = 0; k < model_json.size(); k++)
			{
				s->WriteByte(model_json[k]);
			}
		}
		virtual bool HandleImportModel(OMLT::Model& model)
		{
			_loaded_model = model.rbm;
			if(visible_count != _loaded_model->visible_count)
			{
				SAFE_DELETE(_loaded_model);
				return false;
			}
			Processor::HiddenUnits = _loaded_model->hidden_count;
			Processor::VisibleType = (UnitType)_loaded_model->visible_type;
			Processor::HiddenType = (UnitType)_loaded_model->hidden_type;

			return true;
		}
		virtual float Train( OpenGLBuffer2D& train_example )
		{
			assert(currentState == ProcessorState::Running ||
				currentState == ProcessorState::ScheduleRunning);

			_trainer->Train(train_example);
			return _trainer->GetLastReconstructionError();
		}
		virtual float Validation( OpenGLBuffer2D& validation_example )
		{
			assert(currentState == ProcessorState::Running ||
				currentState == ProcessorState::ScheduleRunning);

			return _trainer->GetReconstructionError(validation_example);
		}
	};

	template<>
	void ITrainer<RBM,CD>::build_schedule()
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
		_schedule->AddTrainingConfig(train_config, epochs_remaining);
	}

	template<>
	void ITrainer<RBM,CD>::build_trainer()
	{
		assert(_trainer == nullptr);
		assert(_schedule != nullptr);

		CD::ModelConfig model_config = _schedule->GetModelConfig();
		model_config.VisibleUnits = visible_count;
		assert(_schedule->GetMinibatchSize() == minibatch_size);

		model_config_to_ui(model_config);

		if(_loaded_model == nullptr)
		{
			_trainer = new ContrastiveDivergence(model_config, _schedule->GetMinibatchSize());
		}
		else
		{
			assert(_loaded_model->visible_count == visible_count);
			_trainer = new ContrastiveDivergence(_loaded_model, _schedule->GetMinibatchSize());
			SAFE_DELETE(_loaded_model);
		}
	}

	template<>
	void ITrainer<RBM,CD>::model_config_to_ui(const CD::ModelConfig& model_config)
	{
		Processor::Model = ModelType::RBM;
		Processor::VisibleType = (UnitType)model_config.VisibleType;
		Processor::HiddenType = (UnitType)model_config.HiddenType;
		Processor::HiddenUnits = model_config.HiddenUnits;
	}

	template<>
	void ITrainer<RBM,CD>::train_config_to_ui(const CD::TrainingConfig& train_config)
	{
		Processor::LearningRate = train_config.LearningRate;
		Processor::Momentum = train_config.Momentum;
		Processor::L1Regularization = train_config.L1Regularization;
		Processor::L2Regularization = train_config.L2Regularization;
		Processor::VisibleDropout = train_config.VisibleDropout;
		Processor::HiddenDropout = train_config.HiddenDropout;
	}


	class AETrainer : public ITrainer<MLP,BP>
	{
		virtual void HandleGetVisibleMsg( Message^ msg ) 
		{
			assert(currentState == ProcessorState::Running ||
				currentState == ProcessorState::ScheduleRunning);

			assert(_trainer != nullptr);

			// allocate buffer space if need be
			const uint32_t visible_size = visible_count * minibatch_size;
			visible_buffer.Acquire(visible_size);
			visible_recon_buffer.Acquire(visible_size);
			visible_diff_buffer.Acquire(visible_size);

			float* visible_ptr = (float*)visible_buffer;
			float* recon_ptr = (float*)visible_recon_buffer;
			float* diff_ptr = (float*)visible_diff_buffer;

			_trainer->DumpLastLabel(&visible_ptr);
			_trainer->DumpActivation(1, &recon_ptr);

			for(uint32_t k = 0; k < visible_size; k++)
			{
				diff_ptr[k] = visible_ptr[k] - recon_ptr[k];
			}

			_trainer->DumpInput(0, &visible_ptr);

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
			assert(currentState == ProcessorState::Running ||
				currentState == ProcessorState::ScheduleRunning);

			assert(_trainer != nullptr);
			// allocate buffer space if need be
			const uint32_t hidden_size = hidden_count * minibatch_size;
			hidden_buffer.Acquire(hidden_size);
			float* hidden_ptr = (float*)hidden_buffer;

			_trainer->DumpActivation(0, &hidden_ptr);

			List<IntPtr>^ hidden_list = dynamic_cast<List<IntPtr>^>(msg["hidden"]);
			for(uint32_t k = 0; k < minibatch_size; k++)
			{
				hidden_list->Add(IntPtr((float*)hidden_buffer + k * hidden_count));
			}
		}
		virtual void HandleGetWeightsMsg( Message^ msg ) 
		{
			assert(currentState == ProcessorState::Running ||
				currentState == ProcessorState::ScheduleRunning);

			assert(_trainer != nullptr);

			// allocate buffer space if need be
			const uint32_t weight_size = (hidden_count) * (visible_count + 1);
			weight_buffer.Acquire(weight_size);

			float* weight_ptr = (float*)weight_buffer;
			_trainer->DumpWeightMatrix(0, &weight_ptr);

			List<IntPtr>^ weight_list = dynamic_cast<List<IntPtr>^>(msg["weights"]);
			for(uint32_t j = 0; j < hidden_count; j++)
			{
				weight_list->Add(IntPtr(weight_ptr + 1 + (visible_count + 1) * j));
			}
		}
		virtual void HandleExportModel(Stream^ s)
		{
			assert(_trainer != nullptr);

			MLP* mlp = _trainer->GetMultilayerPerceptron(0, 1);
			assert(mlp != nullptr);
			std::string model_json = mlp->ToJSON();
			delete mlp;

			for(size_t k = 0; k < model_json.size(); k++)
			{
				s->WriteByte(model_json[k]);
			}
		}

		virtual bool HandleImportModel(OMLT::Model& model)
		{
			_loaded_model = model.mlp;
			if(visible_count != _loaded_model->GetLayer(0)->inputs ||
			   visible_count != _loaded_model->GetLayer(1)->outputs ||
			   _loaded_model->LayerCount() != 2)
			{
				SAFE_DELETE(_loaded_model);
				return false;
			}

			Processor::HiddenUnits = _loaded_model->GetLayer(1)->inputs;
			Processor::VisibleType = (UnitType)_loaded_model->GetLayer(1)->function;
			Processor::HiddenType = (UnitType)_loaded_model->GetLayer(0)->function;

			return true;
		}

		virtual float Train( OpenGLBuffer2D& train_example )
		{
			assert(currentState == ProcessorState::Running ||
				currentState == ProcessorState::ScheduleRunning);
			
			_trainer->Train(train_example, train_example);
			return _trainer->GetLastOutputError();

		}
		virtual float Validation( OpenGLBuffer2D& validation_example )
		{
			assert(currentState == ProcessorState::Running ||
				currentState == ProcessorState::ScheduleRunning);
			return _trainer->GetOutputError(validation_example, validation_example);
		}
	};

	template<>
	void ITrainer<MLP,BP>::build_schedule()
	{
		assert(_schedule == nullptr);
		BP::ModelConfig model_config;
		{
			model_config.InputCount = visible_count;
		}

		BP::LayerConfig hidden_config;
		{
			hidden_config.OutputUnits = hidden_count;
			hidden_config.Function = (ActivationFunction_t)hidden_type;
			hidden_config.Noisy = false;
		}
		BP::LayerConfig output_config;
		{
			output_config.OutputUnits = visible_count;
			output_config.Function = (ActivationFunction_t)visible_type;
			output_config.Noisy = false;
		}

		model_config.LayerConfigs.push_back(hidden_config);
		model_config.LayerConfigs.push_back(output_config);

		BP::TrainingConfig train_config;
		train_config.Initialize(2);
		{
			train_config.LearningRate = learning_rate;
			train_config.Momentum = momentum;
			train_config.L1Regularization = l1;
			train_config.L2Regularization = l2;
			train_config.Dropout[0] = visible_dropout;
			train_config.Dropout[1] = hidden_dropout;
		}

		_schedule = new TrainingSchedule<BackPropagation>(model_config, minibatch_size);
		_schedule->AddTrainingConfig(train_config, epochs_remaining);
	}

	template<>
	void ITrainer<MLP,BP>::build_trainer()
	{
		assert(_trainer == nullptr);
		assert(_schedule != nullptr);

		BP::ModelConfig model_config = _schedule->GetModelConfig();
		model_config.InputCount = visible_count;
		assert(_schedule->GetMinibatchSize() == minibatch_size);

		model_config_to_ui(model_config);

		if(_loaded_model == nullptr)
		{
			_trainer = new BackPropagation(model_config, _schedule->GetMinibatchSize());
		}
		else
		{
			assert(_loaded_model->GetLayer(0)->inputs == visible_count);
			_trainer = new BackPropagation(_loaded_model, _schedule->GetMinibatchSize());
			SAFE_DELETE(_loaded_model);
		}
	}

	template<>
	void ITrainer<MLP,BP>::model_config_to_ui(const BP::ModelConfig& model_config)
	{
		Processor::Model = ModelType::AutoEncoder;
		Processor::VisibleType = (UnitType)model_config.LayerConfigs[1].Function;
		Processor::HiddenType = (UnitType)model_config.LayerConfigs[0].Function;
		Processor::HiddenUnits = model_config.LayerConfigs[0].OutputUnits;
	}

	template<>
	void ITrainer<MLP,BP>::train_config_to_ui(const BP::TrainingConfig& train_config)
	{
		Processor::LearningRate = train_config.LearningRate;
		Processor::Momentum = train_config.Momentum;
		Processor::L1Regularization = train_config.L1Regularization;
		Processor::L2Regularization = train_config.L2Regularization;
		Processor::VisibleDropout = train_config.Dropout[0];
		Processor::HiddenDropout = train_config.Dropout[1];
	}

	void Processor::Run()
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
		currentState = ProcessorState::Stopped;
		// start with an RBMTrainer by default
		trainer = new RBMTrainer();

		while(true)
		{
			Message^ msg = _message_queue->Dequeue();

			if(msg != nullptr)
			{
				switch(msg->Type)
				{
				case MessageType::DeleteData:
					{
						DataAtlas* data = (DataAtlas*)((IntPtr)msg["data"]).ToPointer();
						delete data;
					}
					break;
				case MessageType::Start:
					{
						assert(epochs_remaining > 0);
						assert(training_data != nullptr);
						assert(model_type == ModelType::RBM ||
							   model_type == ModelType::AutoEncoder);

						assert(training_data != nullptr);

						trainer->HandleStartMsg(msg);

						Action^ callback = dynamic_cast<Action^>(msg["callback"]);
						if(callback != nullptr)
						{
							callback();
						}
					}
					break;
				case MessageType::Pause:
					assert(currentState == ProcessorState::Running ||
					       currentState == ProcessorState::ScheduleRunning);
					currentState = ProcessorState::Paused;
					break;
				case MessageType::Stop:
					iterations = 0;
					total_iterations = 0;
					Epochs = 100;
					trainer->HandleStopMsg(msg);
					break;
				case MessageType::GetVisible:
					if(currentState == ProcessorState::Running || currentState == ProcessorState::ScheduleRunning)
					{
						trainer->HandleGetVisibleMsg(msg);
					}
					break;
				case MessageType::GetHidden:
					if(currentState == ProcessorState::Running || currentState == ProcessorState::ScheduleRunning)
					{
						trainer->HandleGetHiddenMsg(msg);
					}
					break;
				case MessageType::GetWeights:
					if(currentState == ProcessorState::Running || currentState == ProcessorState::ScheduleRunning)
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
								Processor::Model = ModelType::RBM;
								break;
							case OMLT::ModelType::MLP:
								Processor::Model = ModelType::AutoEncoder;
								break;
							}

							if(trainer->HandleImportModel(model))
							{
								msg["loaded"] = true;
							}

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
							if(schedule->GetModelConfig().VisibleUnits == visible_count)
							{
								msg["schedule"] = IntPtr(schedule);

								Processor::Model = ModelType::RBM;
								trainer->HandleLoadScheduleMsg(msg);
								msg["loaded"] = true;
							}
							else
							{
								delete schedule;
							}
							
						}
						else if(TrainingSchedule<BP>* schedule = TrainingSchedule<BP>::FromJSON(schedule_json))
						{
							if(schedule->GetModelConfig().InputCount == visible_count)
							{
								msg["schedule"] = IntPtr(schedule);

								Processor::Model = ModelType::AutoEncoder;
								trainer->HandleLoadScheduleMsg(msg);
								msg["loaded"] = true;
							}
							else
							{
								delete schedule;
							}
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
			case ProcessorState::ScheduleLoaded:
			case ProcessorState::Stopped:
			case ProcessorState::Paused:
				Thread::Sleep(16);
				break;
			case ProcessorState::Running:
			case ProcessorState::ScheduleRunning:
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

					training_error = CapError(training_error);
					validation_error = CapError(validation_error);

					// increment iteration counters
					total_iterations++;
					iterations = (iterations + 1) % training_data->GetTotalBatches();

					IterationCompleted(total_iterations, training_error, validation_error);
					
					if(iterations == 0)
					{
						EpochCompleted(epochs_to_train - --epochs_remaining);
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

	bool Processor::SetTrainingData( String^ in_filename )
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
		else if(idx->GetRowLength() >= SiCKL::OpenGLRuntime::GetMaxTextureSize())
		{
			ShowError(String::Format("Error: IDX training data may not have more than {0} values", SiCKL::OpenGLRuntime::GetMaxTextureSize() - 1));
			delete idx;
			return false;
		}

		// clear out the old training data
		Message^ msg = gcnew Message(MessageType::DeleteData);
		msg["data"] = IntPtr(training_data);
		_message_queue->Enqueue(msg);
		while(!msg->Handled)
		{
			Thread::Sleep(16);
		}

		// construct data atlas
		training_data = new DataAtlas(idx);

		// get visible units required
		visible_count = idx->GetRowLength();


		return true;
	}

	bool Processor::SetValidationData( String^ in_filename )
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
			Message^ msg = gcnew Message(MessageType::DeleteData);
			msg["data"] = IntPtr(validation_data);
			_message_queue->Enqueue(msg);
			while(!msg->Handled)
			{
				Thread::Sleep(16);
			}

			// and set new one
			validation_data = new DataAtlas(idx);
			return true;
		}
	}

	bool Processor::SaveModel(Stream^ in_stream)
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

	bool Processor::LoadModel(Stream^ in_stream)
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

	bool Processor::LoadTrainingSchedule(Stream^ in_stream)
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

	void Processor::Start( Action^ act)
	{
		Message^ msg = gcnew Message(MessageType::Start);
		msg["callback"] = act;
		_message_queue->Enqueue(msg);
	}

	void Processor::Pause()
	{
		_message_queue->Enqueue(gcnew Message(MessageType::Pause));
	}

	void Processor::Stop()
	{
		_message_queue->Enqueue(gcnew Message(MessageType::Stop));
	}

	void Processor::Shutdown()
	{
		Stop();
		Message^ msg = gcnew Message(MessageType::Shutdown);

		_message_queue->Enqueue(msg);
		while(!msg->Handled)
		{
			Thread::Sleep(16);
		}
	}

	bool Processor::GetCurrentVisible(List<IntPtr>^ visible, List<IntPtr>^ reconstruction, List<IntPtr>^ diffs)
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

	bool Processor::GetCurrentHidden( List<IntPtr>^ hidden_prob )
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

	bool Processor::GetCurrentWeights( List<IntPtr>^ weights )
	{
		Message^ msg = gcnew Message(MessageType::GetWeights);
		msg["weights"] = weights;

		_message_queue->Enqueue(msg);
		while(!msg->Handled)
		{
			Thread::Sleep(16);
		}

		

		if(model_type == VisualRBMInterop::ModelType::RBM)
		{
			assert(weights->Count == (hidden_count+1) || weights->Count == 0);
			return weights->Count == (hidden_count+1);
		}
		else if(model_type == VisualRBMInterop::ModelType::AutoEncoder)
		{
			// no bias for auto encoder currently
			assert(weights->Count == hidden_count || weights->Count == 0);
			return weights->Count == hidden_count;
		}
	}

#pragma region Properties

	bool Processor::IsInitialized()
	{
		return currentState != ProcessorState::Invalid;
	}

	bool Processor::HasTrainingData::get()
	{
		return training_data != nullptr;
	}

	uint32_t Processor::Epochs::get()
	{
		return epochs_remaining;
	}

	void Processor::Epochs::set( uint32_t e )
	{
		if(e != epochs_remaining)
		{
			epochs_remaining = e;
			epochs_to_train = e;
			iterations = 0;
			EpochsChanged(e);
		}
	}


	int Processor::VisibleUnits::get()
	{
		return visible_count;
	}


	int Processor::HiddenUnits::get()
	{
		return hidden_count;
	}

	void Processor::HiddenUnits::set( int units )
	{
		assert(units > 0);
		if(units != hidden_count)
		{
			if(units >= SiCKL::OpenGLRuntime::GetMaxTextureSize())
			{
				ShowError(String::Format("Maximum number of hidden units supported is {0}", SiCKL::OpenGLRuntime::GetMaxTextureSize() - 1));
				hidden_count = SiCKL::OpenGLRuntime::GetMaxTextureSize() - 1;
			}
			else
			{
				hidden_count = units;
			}
			HiddenUnitsChanged(hidden_count);
		}
	}


	int Processor::MinibatchSize::get()
	{
		return minibatch_size;
	}

	void Processor::MinibatchSize::set( int ms )
	{
		assert(ms > 0);
		if(ms != minibatch_size)
		{
			if(ms > SiCKL::OpenGLRuntime::GetMaxTextureSize())
			{
				ShowError(String::Format("Maximum minibatch size supported is {0}", SiCKL::OpenGLRuntime::GetMaxTextureSize()));
				minibatch_size = SiCKL::OpenGLRuntime::GetMaxTextureSize();
			}
			else
			{
				minibatch_size = ms;
			}
			MinibatchSizeChanged(minibatch_size);
		}
	}


	unsigned int Processor::MinibatchCount::get()
	{
		assert(training_data != nullptr);
		return training_data->GetTotalBatches();
	}

	VisualRBMInterop::ModelType Processor::Model::get()
	{
		return model_type;
	}

	void Processor::Model::set(VisualRBMInterop::ModelType in_type)
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
				trainer = new AETrainer();
				break;
			}
		}
	}

	UnitType Processor::VisibleType::get()
	{
		return visible_type;
	}

	void Processor::VisibleType::set( UnitType ut )
	{
		if(ut != visible_type)
		{
			visible_type = ut;
			VisibleTypeChanged(visible_type);
		}
	}

	UnitType Processor::HiddenType::get()
	{
		return hidden_type;
	}

	void Processor::HiddenType::set( UnitType ut )
	{
		if(ut != hidden_type)
		{
			hidden_type = ut;
			HiddenTypeChanged(hidden_type);
		}
	}

	float Processor::LearningRate::get()
	{
		return learning_rate;
	}

	void Processor::LearningRate::set( float lr )
	{
		assert(lr >= 0.0f);
		if(lr != learning_rate)
		{
			learning_rate = lr;
			LearningRateChanged(learning_rate);
		}
	}


	float Processor::Momentum::get()
	{
		return momentum;
	}

	void Processor::Momentum::set( float m )
	{
		assert(m >= 0.0f && m <= 1.0f);
		if(momentum != m)
		{
			momentum = m;
			MomentumChanged(momentum);
		}
	}


	float Processor::L1Regularization::get()
	{
		return l1;
	}

	void Processor::L1Regularization::set( float reg )
	{
		assert(reg >= 0.0f);
		if(reg != l1)
		{
			l1 = reg;
			L1RegularizationChanged(l1);
		}
	}


	float Processor::L2Regularization::get()
	{
		return l2;
	}

	void Processor::L2Regularization::set( float reg )
	{
		assert(reg >= 0.0f);
		if(reg != l2)
		{
			l2 = reg;
			L2RegularizationChanged(l2);
		}
	}


	float Processor::VisibleDropout::get()
	{
		return visible_dropout;
	}

	void Processor::VisibleDropout::set( float vd )
	{
		assert(vd >= 0.0f && vd < 1.0f);
		if(vd != visible_dropout)
		{
			visible_dropout = vd;
			VisibleDropoutChanged(visible_dropout);
		}
	}

	float Processor::HiddenDropout::get()
	{
		return hidden_dropout;
	}

	void Processor::HiddenDropout::set( float hd )
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