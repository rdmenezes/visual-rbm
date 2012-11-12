using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace VisualRBM
{
	public class Message
	{
		private string _type = "";
		public string Type
		{
			get
			{
				return _type;
			}
		}
		private Dictionary<string, object> _data;

		public Message(string in_type)
		{
			_type = in_type;
			_data = new Dictionary<string, object>();
		}

		public object this[string s]
		{
			get
			{
				lock (_type)
				{
					object result;
					if (_data.TryGetValue(s, out result))
						return result;
					return null;
				}
			}
			set
			{
				lock (_type)
				{
					_data[s] = value;
				}
			}
		}
	}

	public class MessageQueue
	{
		Queue _message_queue;
		public MessageQueue()
		{
			_message_queue = Queue.Synchronized(new Queue());
		}

		public void Enqueue(Message in_message)
		{
			_message_queue.Enqueue(in_message);
		}

		public Message Dequeue()
		{
			lock (_message_queue)
			{
				if (_message_queue.Count > 0)
					return _message_queue.Dequeue() as Message;
				return null;
			}
		}
	}
}
