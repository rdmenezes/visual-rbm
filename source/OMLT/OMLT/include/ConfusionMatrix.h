#include <stdint.h>

namespace OMLT
{
	class ConfusionMatrix
	{
	public:
		ConfusionMatrix(uint32_t in_classes)
		{
			_classes = in_classes;
			_count = 0;
			_data = new uint32_t[_classes*_classes];
			memset(_data, 0x00, sizeof(uint32_t) * _classes * _classes);
		}
		~ConfusionMatrix()
		{
			delete[] _data;
		}

		void Add(uint32_t actual, uint32_t predicted)
		{
			(*this)(actual, predicted)++;
		
			_count++;
		}

		uint32_t TruePositive(uint32_t c)
		{
			return (*this)(c, c);
		}

		uint32_t TrueNegative(uint32_t c)
		{
			uint32_t tn = 0;
			for (uint32_t i = 0; i < _classes; i++)
			{
				for (uint32_t j = 0; j < _classes; j++)
				{
					if (i == c || j == c)
					{
						continue;
					}
					else
					{
						tn += (*this)(i, j);
					}
				}
			}
			return tn;
		}

		uint32_t FalsePositive(uint32_t c)
		{
			uint32_t fp = 0;
			for (uint32_t i = 0; i < _classes; i++)
			{
				if (i != c) 
				{
					fp += (*this)(i, c);
				}
			}
			return fp;
		}

		uint32_t FalseNegative(uint32_t c)
		{
			uint32_t fn = 0;
			for (uint32_t j = 0; j < _classes; j++)
			{
				if (j != c) fn += (*this)(c, j);
			}
			return fn;
		}

		float Accuracy()
		{
			int ac = 0;
			for (int i = 0; i < _classes; i++)
				ac += (*this)(i, i);
			return ac / (float)_count;
		}

		float Precision(uint32_t c)
		{
			uint32_t tp = TruePositive(c);
			return (float)tp / (float)(tp + FalsePositive(c));
		}

		float Recall(uint32_t c)
		{
			uint32_t tp = TruePositive(c);
			return (float)tp / (float)(tp + FalseNegative(c));
		}

		float Specificity(int c)
		{
			uint32_t tn = TrueNegative(c);
			return (float)tn / (float)(tn + FalsePositive(c));
		}


		void Print()
		{
			printf("ConfusionMatrix:\n");
			printf("Row - Actual\n");
			printf("Column - Predicted\n");
			printf("\n");

			for (uint32_t i = 0; i < _classes; i++)
			{
				for (uint32_t j = 0; j < _classes; j++)
					printf("%u,", (*this)(i, j));
				printf("\n");
			}
#if 0
			sb.AppendLine();


			sb.Append("Accuracy:,").Append(Accuracy()).AppendLine(",");
			sb.Append("Precision:,");
			for (int i = 0; i < classes; i++)
				sb.Append(Precision(i)).Append(",");
			sb.AppendLine();
			sb.Append("Recall:,");
			for (int i = 0; i < classes; i++)
				sb.Append(Recall(i)).Append(",");
			sb.AppendLine();
			sb.Append("Specificity:,");
			for (int i = 0; i < classes; i++)
				sb.Append(Specificity(i)).Append(",");
			sb.AppendLine();
			return sb.ToString();
#endif
		}
	private:
		uint32_t _count;
		uint32_t _classes;
		uint32_t* _data;

		ConfusionMatrix();

		uint32_t& operator()(uint32_t actual, uint32_t predicted)
		{
			return _data[actual * _classes + predicted];
		}
	};

}