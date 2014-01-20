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
		delegate void ValueChangedHandler(Object^ new_value);

		static IterationCompletedHandler^ IterationCompleted;
		static EpochCompletedHandler^ EpochCompleted;
		static TrainingCompletedHandler^ TrainingCompleted;

		// value changed handlers
		static ValueChangedHandler^ ModelTypeChanged;
		static ValueChangedHandler^ VisibleTypeChanged;
		static ValueChangedHandler^ HiddenTypeChanged;
		static ValueChangedHandler^ HiddenUnitsChanged;
		static ValueChangedHandler^ LearningRateChanged;
		static ValueChangedHandler^ MomentumChanged;
		static ValueChangedHandler^ L1RegularizationChanged;
		static ValueChangedHandler^ L2RegularizationChanged;
		static ValueChangedHandler^ VisibleDropoutChanged;
		static ValueChangedHandler^ HiddenDropoutChanged;
		static ValueChangedHandler^ MinibatchSizeChanged;
		static ValueChangedHandler^ EpochsChanged;

		enum class RBMProcessorState
		{
			Running, 
			Paused,
			Stopped, 
			ScheduleLoaded,
			ScheduleRunning,
		};

		/** Properties **/

		static property RBMProcessorState CurrentState
		{
			RBMProcessorState get();
		}


		static property bool HasTrainingData
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

		static bool SaveModel(Stream^ stream);
		static bool LoadModel(Stream^ stream);

		static bool LoadTrainingSchedule(Stream^ stream);

		// public command methods
		static void Run();
		// takes a callback for when rbmtrainer init is complete
		static void Start(Action^);
		static void Pause();
		static void Stop();
		static void Shutdown();	// for application exit

		static bool GetCurrentVisible(List<IntPtr>^ visible, List<IntPtr>^ reconstruction, List<IntPtr>^ diffs);
		static bool GetCurrentHidden(List<IntPtr>^ hidden);
		static bool GetCurrentWeights(List<IntPtr>^ weights);

		// visualization methods
		static void RescaleActivations(float* buffer, uint32_t count, UnitType type);
		static void RescaleDiffs(float* buffer, uint32_t count, UnitType func);
		static void RescaleWeights(float* buffer, float stddev, uint32_t count);
		

	private:
		// our message queue
		static MessageQueue^ _message_queue;

	};
}