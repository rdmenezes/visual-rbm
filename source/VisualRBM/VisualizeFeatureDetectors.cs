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
			return (float)((1.0 / (1 + Math.Exp(-x))));
		}

		protected override void Drawing()
		{
			float[] weights = null;
			RBMProcessor.GetCurrentWeights(ref weights);
			PixelFormat pf = _main_form.settingsBar.Format;


			for (int j = 0; j <= RBMProcessor.HiddenUnits; j++)
			{
				float[] pixels = new float[RBMProcessor.VisibleUnits];
				for (int i = 0; i < RBMProcessor.VisibleUnits; i++)
					pixels[i] = sigmoid(weights[(RBMProcessor.HiddenUnits + 1) * (i + 1) + j]);

				UpdateImageControlContents(j, pf, pixels);
			}

			foreach (ImageControl ic in imageFlowPanel.Controls)
				ic.Invalidate();
		}
	}
}
