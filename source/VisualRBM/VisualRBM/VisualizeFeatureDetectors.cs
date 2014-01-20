using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;

using QuickBoltzmann;

namespace VisualRBM
{
	public class VisualizeFeatureDetectors : VisualizeReconstruction
	{
		override protected int ImageControlCount
		{
			get
			{
				switch (RBMProcessor.Model)
				{
					case ModelType.AutoEncoder:
						return RBMProcessor.HiddenUnits;
					case ModelType.RBM:
						return RBMProcessor.HiddenUnits + 1;
					default:
						return 0;
				}
			}
		}
	

		public VisualizeFeatureDetectors()
		{
			_draw_interval = 5.0;
			_count = 1;
		}

		float sigmoid(float x)
		{
			return (float)((1.0 / (1 + Math.Exp(-2.75 * x))));
		}

		protected override unsafe void Drawing()
		{
			List<IntPtr> weights = new List<IntPtr>();
			if (RBMProcessor.GetCurrentWeights(weights))
			{

				PixelFormat pf = _main_form.settingsBar.Format;

				float stddev = 0.0f;
				foreach (IntPtr ptr in weights)
				{
					float* raw_weights = (float*)ptr.ToPointer();
					for (uint k = 0; k < RBMProcessor.VisibleUnits; k++)
					{
						float val = raw_weights[k];
						stddev += val * val;
					}
				}
				stddev /= weights.Count * RBMProcessor.VisibleUnits;
				stddev = (float)Math.Sqrt(stddev);

				for (int j = 0; j < weights.Count; j++)
				{
					float* raw_weights = (float*)weights[j].ToPointer();

					RBMProcessor.RescaleWeights(raw_weights, stddev, (uint)RBMProcessor.VisibleUnits);
					UpdateImageControlContents(j, pf, (uint)RBMProcessor.VisibleUnits, raw_weights);
				}

				foreach (ImageControl ic in imageFlowPanel.Controls)
					ic.Invalidate();
			}
		}
	}
}
