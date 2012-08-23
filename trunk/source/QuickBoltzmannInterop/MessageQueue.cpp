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
		_message_queue = gcnew ConcurrentQueue<Message^>();
	}

	void MessageQueue::Enqueue(Message^ in_message)
	{
		_message_queue->Enqueue(in_message);
	}

	Message^ MessageQueue::Dequeue()
	{
		Message^ result;
		if(_message_queue->TryDequeue(result))
			return result;
		return nullptr;
	}

}