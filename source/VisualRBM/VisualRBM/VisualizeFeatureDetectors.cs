using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;

using VisualRBMInterop;

namespace VisualRBM
{
	public class VisualizeFeatureDetectors : VisualizeReconstruction
	{
		override protected double DrawInterval
		{
			get
			{
				return 2.0 * Math.Log10(Processor.HiddenUnits) + 0.5;
			}
		}

		override protected int ImageControlCount
		{
			get
			{
				switch (Processor.Model)
				{
					case ModelType.AutoEncoder:
						return Processor.HiddenUnits + 1;
					case ModelType.RBM:
						return Processor.HiddenUnits + 1;
					default:
						return 0;
				}
			}
		}
	

		public VisualizeFeatureDetectors()
		{
			_count = 1;
		}

		float sigmoid(float x)
		{
			return (float)((1.0 / (1 + Math.Exp(-2.75 * x))));
		}

		protected override unsafe void Drawing()
		{
			List<IntPtr> weights = new List<IntPtr>();
			if (Processor.GetCurrentWeights(weights))
			{

				PixelFormat pf = _main_form.settingsBar.Format;

				// stddev calculations assume zero mean

				float* raw_weights = (float*)weights[0].ToPointer();
				float bias_stddev = 0.0f;
				for (uint k = 0; k < Processor.VisibleUnits; k++)
				{
					float val = raw_weights[k];
					bias_stddev += val * val;
				}
				bias_stddev /= Processor.VisibleUnits;
				bias_stddev = (float)Math.Sqrt(bias_stddev);

				Processor.RescaleWeights(raw_weights, bias_stddev, (uint)Processor.VisibleUnits);
				UpdateImageControlContents(0, pf, (uint)Processor.VisibleUnits, raw_weights);

				float weight_stddev = 0.0f;
				for(int j = 1; j < weights.Count; j++)
				{
					raw_weights = (float*)weights[j].ToPointer();
					for (uint k = 0; k < Processor.VisibleUnits; k++)
					{
						float val = raw_weights[k];
						weight_stddev += val * val;
					}
				}
				weight_stddev /= (weights.Count - 1) * Processor.VisibleUnits;
				weight_stddev = (float)Math.Sqrt(weight_stddev);



				for (int j = 1; j < weights.Count; j++)
				{
					raw_weights = (float*)weights[j].ToPointer();

					Processor.RescaleWeights(raw_weights, weight_stddev, (uint)Processor.VisibleUnits);
					UpdateImageControlContents(j, pf, (uint)Processor.VisibleUnits, raw_weights);
				}

				foreach (ImageControl ic in imageFlowPanel.Controls)
					ic.Invalidate();
			}
		}
	}
}
