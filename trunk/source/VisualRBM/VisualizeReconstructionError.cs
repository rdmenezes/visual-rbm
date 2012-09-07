using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Windows.Forms.DataVisualization.Charting;

using QuickBoltzmann;


namespace VisualRBM
{
	public partial class VisualizeReconstructionError : UserControl
	{
		float[] training_buffer;
		float training_sum = 0.0f;

		float[] validation_buffer;
		float validation_sum = 0.0f;

		float last_training_error = 0.0f;
		float last_validation_error = 0.0f;

		const uint BUFFER_LENGTH = 250;

		public VisualizeReconstructionError()
		{
			training_buffer = new float[BUFFER_LENGTH];
			training_sum = 0.0f;

			validation_buffer = new float[BUFFER_LENGTH];
			validation_sum = 0.0f;

			InitializeComponent();

			RBMProcessor.IterationCompleted += new RBMProcessor.IterationCompletedHandler(RBMProcessor_ItereationCompleted);
			RBMProcessor.EpochCompleted += new RBMProcessor.EpochCompletedHandler(RBMProcessor_EpochCompleted);
		}

		void RBMProcessor_ItereationCompleted(uint iteration, float training_error, float validation_error)
		{
			uint index = iteration % BUFFER_LENGTH;
			training_sum += training_error / (float)BUFFER_LENGTH - training_buffer[index];
			training_buffer[index] = training_error / (float)BUFFER_LENGTH;

			validation_sum += validation_error / (float)BUFFER_LENGTH - validation_buffer[index];
			validation_buffer[index] = validation_error / (float)BUFFER_LENGTH;

			last_training_error = training_sum;
			last_validation_error = validation_sum;

			if (iteration % BUFFER_LENGTH == 0)
			{
				this.Invoke(new Action(() =>
				{
					DataPoint dp = new DataPoint(iteration, training_sum);
					this.chart1.Series[0].Points.Add(dp);

					if (validation_error > 0.0f)
					{
						dp = new DataPoint(iteration, validation_sum);
						this.chart1.Series[1].Points.Add(dp);
					}
				}));
			}
		}

		void RBMProcessor_EpochCompleted(uint epoch)
		{
			if(last_validation_error > 0.0)
				(this.ParentForm as Main).trainingLog.AddLog("Epoch {0}; Training Error={1}; Validation Error={2}", epoch, last_training_error, last_validation_error);
			else
				(this.ParentForm as Main).trainingLog.AddLog("Epoch {0}; Training Error={1}", epoch, last_training_error);
		}

		public void Stop()
		{
			this.Invoke(new Action(() =>
			{
				this.chart1.Series[0].Points.Clear();
				this.chart1.Series[1].Points.Clear();

				for (int k = 0; k < training_buffer.Length; k++)
				{
					training_buffer[k] = 0.0f;
					validation_buffer[k] = 0.0f;
				}

				last_training_error = 0.0f;
				last_validation_error = 0.0f;

				training_sum = 0.0f;
				validation_sum = 0.0f;
			}));
		}
	}
}
