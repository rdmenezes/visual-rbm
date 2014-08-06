using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Windows.Forms;
using VisualRBMInterop;

namespace VisualRBM
{
	static class Program
	{
		enum Arguments
		{
			AtlasSize,
			Count,
		}

		static string[] argumentStrings = 
		{
			"-atlasSize="
		};



		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			string[] arg_vals = new string[(int)Arguments.Count];

			foreach (string arg in args)
			{
				bool handled = false;
				for (int k = 0; k < (int)Arguments.Count; k++)
				{
					string current_arg = argumentStrings[k];
					if (arg.IndexOf(current_arg) == 0)
					{
						if (arg_vals[k] != null)
						{
							MessageBox.Show(String.Format("Duplicate argument for {0} found", current_arg), "Error", System.Windows.Forms.MessageBoxButtons.OK, System.Windows.Forms.MessageBoxIcon.Error);
							return;
						}
						arg_vals[k] = arg.Substring(current_arg.Length);
						handled = true;
					}
				}

				if (handled == false)
				{
					MessageBox.Show(String.Format("Do not know how to process command line argument: {0}", arg), "Error", System.Windows.Forms.MessageBoxButtons.OK, System.Windows.Forms.MessageBoxIcon.Error);
					return;
				}
			}

			uint atlasSize = 0;

			if (arg_vals[(int)Arguments.AtlasSize] == null)
			{
				atlasSize = 512;
			}
			else if(uint.TryParse(arg_vals[(int)Arguments.AtlasSize], out atlasSize) == false ||
				    atlasSize < 128)
			{
				MessageBox.Show(String.Format("Could not parse \"{0}\" as a valid size, must be at least 128 megabytes", arg_vals[(int)Arguments.AtlasSize]), "Error", System.Windows.Forms.MessageBoxButtons.OK, System.Windows.Forms.MessageBoxIcon.Error);
				return;
			}
			
			// init the boltzmann trainer and set it go
			Thread thr = new Thread(() =>
			{
				Processor.Run(atlasSize);
			});
			thr.Name = "RBM Processor";
			thr.IsBackground = true;
			thr.Start();
			
			while (Processor.IsInitialized()) Thread.Sleep(16);

			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);

			Main m = new Main();
			Application.Run(m);
		}
	}
}
