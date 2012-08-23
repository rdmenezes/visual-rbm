#pragma once

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Concurrent;
using namespace System::Threading;

namespace QuickBoltzmann
{
	public ref class Message
	{
	public:

		property String^ Type
		{
			String^ get() { return _type;};
		}

		Message(String^ in_type);

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

	private:
		String^ _type;
		ConcurrentDictionary<String^, Object^>^ _data;
	};

	public ref class MessageQueue
	{
	public:
	
	
		MessageQueue();
		void Enqueue(QuickBoltzmann::Message^);
		QuickBoltzmann::Message^ Dequeue();
	private:
		ConcurrentQueue<QuickBoltzmann::Message^>^ _message_queue;
	};
}