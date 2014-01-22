using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using VisualRBMInterop;
using System.Diagnostics;

namespace VisualRBM
{


	public partial class Main : Form
	{
		public Main()
		{
			InitializeComponent();

			this.ClientSize = new Size(this.settingsBar.Width, 500);
			this.MinimumSize = this.Size;

			settingsBar.MainForm = this;
			visualizeVisible.MainForm = this;
			visualizeHidden.MainForm = this;
			visualizeWeights.MainForm = this;

			this.FormClosing += new FormClosingEventHandler(Main_FormClosing);
		}

		void Main_FormClosing(object sender, FormClosingEventArgs e)
		{
			Processor.Shutdown();
		}
	}
}
