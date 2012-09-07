#include "MessageQueue.h"

namespace QuickBoltzmann
{

	Message::Message(String^ in_type)
	{
		_type = in_type;
		_data = gcnew ConcurrentDictionary<String^, Object^>();
	}

	MessageQueue::MessageQueue()
	{
		_message_queue = gcnew Queue();
	}

	void MessageQueue::Enqueue(Message^ in_message)
	{
		System::Threading::Monitor::Enter(_message_queue);
		try
		{
			_message_queue->Enqueue(in_message);
		}
		finally
		{
			System::Threading::Monitor::Exit(_message_queue);
		}
	}

	Message^ MessageQueue::Dequeue()
	{
		Message^ result = nullptr;

		System::Threading::Monitor::Enter(_message_queue);
		try
		{
			if(_message_queue->Count > 0)
			{
				result = dynamic_cast<Message^>(_message_queue->Dequeue());
			}
		}
		finally
		{
			System::Threading::Monitor::Exit(_message_queue);
		}

		return result;
	}

}