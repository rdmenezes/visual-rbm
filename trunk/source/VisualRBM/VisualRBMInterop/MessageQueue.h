#pragma once

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Concurrent;
using namespace System::Threading;

namespace QuickBoltzmann
{
	public enum class MessageType
	{
		Invalid = -1,
		Start,
		Pause,
		Stop,
		GetVisible,
		GetHidden,
		GetWeights,
		ExportModel,
		ImportModel,
		LoadSchedule,
		Shutdown,
	};

	public ref class Message
	{
	public:

		property MessageType Type
		{
			MessageType get() { return _type;};
		}

		Message(MessageType in_type);

		property Object^ default [String^]
		{
			Object^ get(String^ s)
			{
				Object^ result;
				if(_data->TryGetValue(s, result))
					return result;
				return nullptr;
			}
			void set(String^ s, Object^ o)
			{
				_data[s] = o;
			}
		}
		bool Handled;
	private:
		MessageType _type;
		ConcurrentDictionary<String^, Object^>^ _data;
	};

	public ref class MessageQueue
	{
	public:
		MessageQueue();
		void Enqueue(QuickBoltzmann::Message^);
		QuickBoltzmann::Message^ Dequeue();
		void Clear();
	private:
		Queue^ _message_queue;
	};
}