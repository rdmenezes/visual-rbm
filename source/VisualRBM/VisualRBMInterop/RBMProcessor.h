#pragma once

#include <stdint.h>
#include "MessageQueue.h"

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;

namespace QuickBoltzmann
{
	public enum class ModelType
	{
		Invalid = -1,
		RBM,
		AutoEncoder,
	};

	public enum class UnitType 
	{ 
		Invalid = -1,
		Linear,
		RectifiedLinear,
		Sigmoid,
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
			bool get();
		};

		static property bool HasValidationData
		{
			bool get();
		};

		static property uint32_t Epochs
		{
			uint32_t get();
			void set(uint32_t e);
		}

		static property int VisibleUnits
		{
			int get();
		}

		static property int HiddenUnits
		{
			int get();
			void set(int units);
		}

		static property int MinibatchSize
		{
			int get();
			void set(int ms);
		}

		static property unsigned int MinibatchCount
		{
			unsigned int get();
		}

		static property ModelType Model
		{
			ModelType get();
			void set(ModelType);
		}

		static property UnitType VisibleType
		{
			UnitType get();
			void set(UnitType ut);
		}

		static property UnitType HiddenType
		{
			UnitType get();
			void set(UnitType);
		}

		static property float LearningRate
		{
			float get();
			void set(float lr);
		}

		static property float Momentum
		{
			float get();
			void set(float m);
		}

		static property float L1Regularization
		{
			float get();
			void set(float reg);
		}

		static property float L2Regularization
		{
			float get();
			void set(float reg);
		}

		static property float VisibleDropout
		{
			float get();
			void set(float vd);
		}

		static property float HiddenDropout
		{
			float get();
			void set(float hd);
		}

		static float Sigmoid(float x)
		{
			return float(1.0 / (1.0 + Math::Exp(-x)));
		}

		static bool SetTrainingData(String^ filename);
		static bool SetValidationData(String^ filename);

		static void SaveModel(Stream^ stream);
		static bool LoadModel(Stream^ stream);

		// public command methods
		static void Run();
		// takes a callback for when rbmtrainer init is complete
		static void Start(Action^);
		static void Pause();
		static void Stop();
		static void Shutdown();	// for application exit

		static void GetCurrentVisible(List<IntPtr>^ visible, List<IntPtr>^ reconstruction, List<IntPtr>^ diffs);
		static void GetCurrentHidden(List<IntPtr>^ hidden);
		static void GetCurrentWeights(List<IntPtr>^ weights);

	private:

		// our message queue
		static MessageQueue^ _message_queue;

		static RBMProcessorState _currentState = RBMProcessorState::Unintialized;

	};
}