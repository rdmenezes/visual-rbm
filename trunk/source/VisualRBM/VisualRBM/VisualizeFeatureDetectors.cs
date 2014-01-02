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
				return RBMProcessor.HiddenUnits + 1;
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

				for (int j = 0; j < weights.Count; j++)
				{
					float* raw_weights = (float*)weights[j].ToPointer();
					UpdateImageControlContents(j, pf, (uint)RBMProcessor.VisibleUnits, raw_weights);
				}

				foreach (ImageControl ic in imageFlowPanel.Controls)
					ic.Invalidate();
			}
		}
	}
}
