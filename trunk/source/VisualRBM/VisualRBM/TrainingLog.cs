using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace VisualRBM
{
	public partial class TrainingLog : UserControl
	{
		public TrainingLog()
		{
			InitializeComponent();
		}

		public void AddLog(String format, params object[] objects)
		{
			this.BeginInvoke( new Action(() =>
				{
					logTextBox.AppendText(String.Format(format, objects) + "\r\n");
				}));
		}
	}
}
