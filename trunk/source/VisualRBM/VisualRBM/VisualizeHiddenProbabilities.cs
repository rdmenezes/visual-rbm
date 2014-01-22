using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using VisualRBMInterop;

namespace VisualRBM
{
	public class VisualizeHiddenProbabilities : VisualizeReconstruction
	{
		public VisualizeHiddenProbabilities()
		{
			_count = 1;
			_scale = 8;
		}


		protected override int ImageControlWidth
		{
			get
			{
				return (int)Math.Max(1, Math.Sqrt(Processor.HiddenUnits));
			}
		}

		protected override int ImageControlHeight
		{
			get
			{
				int result = Processor.HiddenUnits / ImageControlWidth;
				if (result * ImageControlWidth < Processor.HiddenUnits)
					result++;
				return result;
			}
		}

		protected override PixelFormat ImageControlPixelFormat
		{
			get
			{
				return PixelFormat.Lightness;
			}
		}

		protected override unsafe void Drawing()
		{
			List<IntPtr> hidden = new List<IntPtr>();

			if (Processor.GetCurrentHidden(hidden))
			{ 

				for (int k = 0; k < Processor.MinibatchSize; k++)
				{
					float* raw_hidden = (float*)hidden[k].ToPointer();
					Processor.RescaleActivations(raw_hidden, (uint)Processor.HiddenUnits, Processor.HiddenType);
					UpdateImageControlContents(k, PixelFormat.Lightness, (uint)Processor.HiddenUnits, raw_hidden);
				}
			}
		}
	}
}
