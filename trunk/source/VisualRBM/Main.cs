using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using QuickBoltzmann;
using System.Diagnostics;

namespace VisualRBM
{


	public partial class Main : Form
	{
		public Main()
		{
			InitializeComponent();

			this.MinimumSize = new System.Drawing.Size(this.Size.Width, 350);

			settingsBar.MainForm = this;
			visualizeVisible.MainForm = this;
			visualizeHidden.MainForm = this;
			visualizeWeights.MainForm = this;

			this.FormClosing += new FormClosingEventHandler(Main_FormClosing);
		}

		void Main_FormClosing(object sender, FormClosingEventArgs e)
		{
			RBMProcessor.Shutdown();
		}
	}
}
