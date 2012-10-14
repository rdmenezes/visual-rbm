#pragma once

#include "../FreeGLUT/include/GL/freeglut.h"

#include "../QuickBoltzmannNative/RBMTrainer.h"
#include "MessageQueue.h"
#include <stdint.h>
using namespace System::Collections::Generic;

extern RBMTrainer* rbmtrainer;

namespace QuickBoltzmann
{
	public enum class ModelType
	{
		RBM,
		SRBM
	};

	public enum class UnitType 
	{ 
		Binary, 
		Gaussian 
	};
	public enum class RegularizationMethod
	{
		None,
		L1,
		L2
	};

	public ref class RBMProcessor
	{
	public:
		delegate void IterationCompletedHandler(uint32_t iteration, float training_error, float validation_error);
		delegate void EpochCompletedHandler(uint32_t epoch);
		delegate void TrainingCompletedHandler();


		static IterationCompletedHandler^ IterationCompleted;
		static EpochCompletedHandler^ EpochCompleted;
		static TrainingCompletedHandler^ TrainingCompleted;


		enum class RBMProcessorState
		{
			Unintialized, 
			Ready, 
			Training, 
			Paused
		};

		/** Properties **/

		static property RBMProcessorState CurrentState
		{
			RBMProcessorState get() {return _currentState;};
		}

		static property bool HasTrainingData
		{
			bool get()
			{
				return _has_training_data;
			}
		};

		static property bool HasValidationData
		{
			bool get()
			{
				return _has_validation_data;
			}
		};

		static property uint32_t Epochs
		{
			uint32_t get() {return _epochs;};
			void set(uint32_t e)
			{
				_epochs = e;
				_iterations = 0;
			}
		}

		static property int VisibleUnits
		{
			int get() {return rbmtrainer->GetVisibleCount();};
		}

		static property int HiddenUnits
		{
			int get() {return rbmtrainer->GetHiddenCount();};
			void set(int units)
			{
				rbmtrainer->SetHiddenCount(units);
			}
		}

		static property int MinibatchSize
		{
			int get() { return rbmtrainer->GetMinibatchSize();};
			void set(int ms)
			{
				rbmtrainer->SetMinibatchSize(ms);
			}
		}

		static property ModelType Model
		{
			ModelType get() {return 
				(ModelType)rbmtrainer->GetModelType();};
		}

		static property UnitType VisibleType
		{
			UnitType get() {return
				(QuickBoltzmann::UnitType)rbmtrainer->GetVisibleType();};
			void set(UnitType ut)
			{
				rbmtrainer->SetVisibleType((::UnitType)ut);
			}
		}

		static property float LearningRate
		{
			float get() {return rbmtrainer->GetLearningRate();};
			void set(float lr)
			{
				rbmtrainer->SetLearningRate(lr);
			}
		}

		static property float Momentum
		{
			float get() {return rbmtrainer->GetMomentum();};
			void set(float m)
			{
				rbmtrainer->SetMomentum(m);
			}
		}

		static property float L1Regularization
		{
			float get() {return rbmtrainer->GetL1Regularization();};
			void set(float reg)
			{
				rbmtrainer->SetL1Regularization(reg);
			}
		}

		static property float L2Regularization
		{
			float get() {return rbmtrainer->GetL2Regularization();};
			void set(float reg)
			{
				rbmtrainer->SetL2Regularization(reg);
			}
		}

		static property float VisibleDropout
		{
			float get() {return rbmtrainer->GetVisibleDropout();};
			void set(float vd)
			{
				rbmtrainer->SetVisibleDropout(vd);
			}
		}

		static property float HiddenDropout
		{
			float get() {return rbmtrainer->GetHiddenDropout();};
			void set(float hd)
			{
				rbmtrainer->SetHiddenDropout(hd);
			}
		}

		static float Sigmoid(float x)
		{
			return float(1.0 / (1.0 + Math::Exp(-x)));
		}

		static bool SetTrainingData(String^ filename, bool calculate_stats);
		static bool SetValidationData(String^ filename);
		static void SaveRBM(String^ filename);
		static void LoadRBM(String^ filename, uint32_t% hidden_units, bool% linear_units);

		// public command methods
		static void Run();
		// takes a callback for when rbmtrainer init is complete
		static void Start(Action^);
		static void Pause();
		static void Stop();
		static void Shutdown();	// for application exit

		static void GetCurrentVisible(List<array<float>^>^% visible_data, List<array<float>^>^% visible_reconstruction);
		static void GetCurrentHidden(List<array<float>^>^% hidden_prob);
		static void GetCurrentWeights(array<float>^% weights);
		static float GetReconstructionError();

		

	private:

		// number of epochs to go
		static uint32_t _epochs = 100;
		// number of iteratiosn run this epoch
		static uint32_t _iterations = 0;
		// total number of iteratiosn run
		static uint32_t _total_iterations = 0;
		// do we have training data?
		static bool _has_training_data = false;
		// do we have validation dadta?
		static bool _has_validation_data = false;

		// our message queue
		static MessageQueue^ _message_queue;

		static RBMProcessorState _currentState = RBMProcessorState::Unintialized;

	};
}