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

		protected override void Drawing()
		{
			List<float[]> hidden = null;

			RBMProcessor.GetCurrentHidden(ref hidden);

			// rbm trainer was shutdown before thsi draw call made it in
			if (hidden == null)
			{
				return;
			}

			for (int k = 0; k < RBMProcessor.MinibatchSize; k++)
			{
				UpdateImageControlContents(k, PixelFormat.Lightness, hidden[k]);
			}
		}
	}
}
