using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Windows.Forms;
using QuickBoltzmann;

namespace VisualRBM
{
	static class Program
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			// init the boltzmann trainer and set it go
			Thread thr = new Thread(() =>
			{
				RBMProcessor.Run();
			});
			thr.Name = "RBM Processor";
			thr.IsBackground = true;
			thr.Start();
			
			while (RBMProcessor.CurrentState != RBMProcessor.RBMProcessorState.Ready) Thread.Sleep(16);

			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);

			Main m = new Main();
			Application.Run(m);
		}
	}
}
