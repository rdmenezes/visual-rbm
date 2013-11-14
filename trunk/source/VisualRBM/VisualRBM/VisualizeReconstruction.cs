using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;

using QuickBoltzmann;
using System.Diagnostics;

namespace VisualRBM
{

	public partial class VisualizeReconstruction : UserControl
	{
		protected Main _main_form;

		internal Main MainForm
		{
			get
			{
				return _main_form;
			}
			set
			{
				_main_form = value;
			}
		}

		// zoom factor for our image controls
		protected int _scale;
		// number of images each control has (3 for this case: original, reconstruction, difference)
		protected int _count;
		// how frequently we update the images (in seconds)
		protected double _draw_interval = 1.5;
		// thread-safe message queue used here
		protected MessageQueue _messages;

		// our current state
		protected enum State
		{
			Stopped,
			Running,
			Paused
		}

		protected State _current_state;

		// this virtual property determines how many 
		virtual protected int ImageControlCount
		{
			get
			{
				return RBMProcessor.MinibatchSize;
			}
		}

		virtual protected int ImageControlWidth
		{
			get
			{
				return _main_form.settingsBar.ImageWidth;
			}
		}

		virtual protected int ImageControlHeight
		{
			get
			{
				return _main_form.settingsBar.ImageHeight;
			}
		}

		virtual protected PixelFormat ImageControlPixelFormat
		{
			get
			{
				return _main_form.settingsBar.Format;
			}
		}

		public VisualizeReconstruction()
		{
			// 3x zoom seems like a good default
			_scale = 3;
			_count = 3;

			_current_state = State.Stopped;
			_messages = new MessageQueue();
			
			// start the async updating
			var thr = new Thread(() =>
			{
				this.AsyncUpdate();
			});
			thr.IsBackground = true;
			thr.Start();

			InitializeComponent();
		}


		public void Start()
		{
			_messages.Enqueue(new Message("Start"));
		}

		public void Pause()
		{
			_messages.Enqueue(new Message("Pause"));
		}

		public void Stop()
		{
			_messages.Enqueue(new Message("Stop"));
		}

		private void AsyncUpdate()
		{
			DateTime prev = DateTime.Now;
			double time_since_last_draw = 0;

			bool parent_previously_visible = false;
			bool drawn_atleast_once = false;

			while (true)
			{
				// update last time since last draw
				DateTime now = DateTime.Now;
				time_since_last_draw += now.Subtract(prev).TotalSeconds;
				prev = now;

				// get a command message if there is one
				Message m = _messages.Dequeue();
				if (m != null)
				{
					switch (m.Type)
					{
						case "Start":
							switch (_current_state)
							{
								case State.Paused:
									_current_state = State.Running;
									break;
								case State.Running:
									break;
								case State.Stopped:
									// image controls need to be added and modified on the main thread, so do an Invoke
									this.Invoke(new Action(() =>
									{
										int image_width = this.ImageControlWidth;
										int image_height = this.ImageControlHeight;
										PixelFormat pixel_format = this.ImageControlPixelFormat;

										for (int k = 0; k < this.ImageControlCount; k++)
										{
											ImageControl ic = new ImageControl(_count, image_width, image_height, _scale, pixel_format);
											imageFlowPanel.Controls.Add(ic);
										}
										this._current_state = State.Running;
									}));

									break;
							}
							break;
						case "Pause":
							// pausing just 
							_current_state = State.Paused;
							break;
						case "Stop":
							// stop the visualizer and clear out the images on the main thread
							_current_state = State.Stopped;
							drawn_atleast_once = false;
							this.Invoke(new Action(() =>
							{
								this.imageFlowPanel.Controls.Clear();
							}));
							break;
						case "Zoom":
							// update image controls on the main thread
							var delta = (int)m["delta"];
							// scale must be at least 1
							_scale = Math.Max(delta + _scale, 1);
							UpdateImageControlDimensions();
							break;
					}
				}
				else
				{
					// sleep when the message queue is empty
					Thread.Sleep(50);
				}

				if (_current_state == State.Running && drawn_atleast_once == false)
				{
					drawn_atleast_once = true;
					time_since_last_draw = 0.0;
					Drawing();
					prev = DateTime.Now;
				}

				if (_current_state == State.Running && time_since_last_draw >= _draw_interval)
				{
					if (this.Parent != null && this.Parent.Visible == true)
					{
						if (parent_previously_visible == false)
						{
							// sleep awhile before drawing 
							Thread.Sleep((int)(250 * _draw_interval));
						}

						// reset time_since_last_draw and update
						time_since_last_draw = 0.0;
						Drawing();
						prev = DateTime.Now;
					}
					parent_previously_visible = this.Parent != null ? this.Parent.Visible : false;
				}
				
			}
		}

		protected virtual unsafe void Drawing()
		{
			List<IntPtr> visible  = new List<IntPtr>();
			List<IntPtr> recon = new List<IntPtr>();
			List<IntPtr> diffs = new List<IntPtr>();

			RBMProcessor.GetCurrentVisible(visible, recon, diffs);

			PixelFormat pf = _main_form.settingsBar.Format;

			Debug.Assert(visible.Count == recon.Count && visible.Count == RBMProcessor.MinibatchSize);

			for (int k = 0; k < RBMProcessor.MinibatchSize; k++)
			{
				float* raw_image = (float*)visible[k].ToPointer();
				float* raw_recon = (float*)recon[k].ToPointer();
				float* raw_diffs = (float*)diffs[k].ToPointer();
				// post the data to the image control
				UpdateImageControlContents(k, pf, (uint)RBMProcessor.VisibleUnits, raw_image, raw_recon, raw_diffs);
			}
		}

		public unsafe void UpdateImageControlContents(int index, PixelFormat pf, uint length, params float*[] buffers)
		{
			this.Invoke(new Action(() =>
			{
				if (imageFlowPanel.Controls.Count > index)
				{
					ImageControl ic = imageFlowPanel.Controls[index] as ImageControl;
					ic.Format = pf;
					for (int k = 0; k < buffers.Length; k++)
						ic.SetImage(k, length, buffers[k]);
					ic.Invalidate();
				}
			}));
		}

		// sets the raw data for the given control, invokes on main thread
		public void UpdateImageControlContents(int index, PixelFormat pf, params float[][] buffers)
		{
			this.Invoke(new Action(() =>
			{
				if (imageFlowPanel.Controls.Count > index)
				{
					ImageControl ic = imageFlowPanel.Controls[index] as ImageControl;
					ic.Format = pf;
					for (int k = 0; k < buffers.Length; k++)
						ic.SetImage(k, buffers[k]);
					ic.Invalidate();
				}
			}));
		}

		// updates the control dimension/scale on main thread
		public void UpdateImageControlDimensions()
		{
			this.Invoke(new Action(() =>
			{
				int image_width = this.ImageControlWidth;
				int image_height = this.ImageControlHeight;

				imageFlowPanel.SuspendLayout();
				foreach (ImageControl ic in imageFlowPanel.Controls)
				{
					ic.UpdateSize(image_width, image_height, _scale);
				}
				imageFlowPanel.ResumeLayout();
				imageFlowPanel.Invalidate(true);
			}));
		}

		private void zoomInButton_Click(object sender, EventArgs e)
		{
			var m = new Message("Zoom");
			m["delta"] = 1;

			_messages.Enqueue(m);
		}

		private void zoomOutButton_Click(object sender, EventArgs e)
		{
			var m = new Message("Zoom");
			m["delta"] = -1;

			_messages.Enqueue(m);
		}
	}
}
