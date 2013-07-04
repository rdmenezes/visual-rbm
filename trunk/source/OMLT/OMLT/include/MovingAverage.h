#include <stdint.h>

namespace OMLT
{
	class MovingAverage
	{
	public:
		static MovingAverage* Build(const uint32_t in_size);
		void operator delete(void* in_ptr);

		float GetAverage() const;
		void AddEntry(float in_val);

	private:
		MovingAverage() {}
		MovingAverage(const MovingAverage&) {}
		MovingAverage& operator=(const MovingAverage&) {return *this;}

		uint32_t _size;
		uint32_t _index;
		float _sum;
		float _value_buffer[];

	/*





		static float[] error_buffer = null;
		static float total_error = 0.0f;
		static int index = 0;

		public MovingAverage(int size)
		{
			error_buffer = new float[size];
		}

		public void AddEntry(float err)
		{
			total_error -= error_buffer[index];
			error_buffer[index] = err;

			total_error += err;

			index = (index + 1) % error_buffer.Length;
		}

		public float GetError()
		{
			return total_error / error_buffer.Length;
		}
	*/
	};
}