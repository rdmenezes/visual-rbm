using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using QuickBoltzmann;

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
				return (int)Math.Max(1, Math.Sqrt(RBMProcessor.HiddenUnits));
			}
		}

		protected override int ImageControlHeight
		{
			get
			{
				int result = RBMProcessor.HiddenUnits / ImageControlWidth;
				if (result * ImageControlWidth < RBMProcessor.HiddenUnits)
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

			if (RBMProcessor.GetCurrentHidden(hidden))
			{ 

				for (int k = 0; k < RBMProcessor.MinibatchSize; k++)
				{
					float* raw_hidden = (float*)hidden[k].ToPointer();
					RBMProcessor.RescaleActivations(raw_hidden, (uint)RBMProcessor.HiddenUnits, RBMProcessor.HiddenType);
					UpdateImageControlContents(k, PixelFormat.Lightness, (uint)RBMProcessor.HiddenUnits, raw_hidden);
				}
			}
		}
	}
}
