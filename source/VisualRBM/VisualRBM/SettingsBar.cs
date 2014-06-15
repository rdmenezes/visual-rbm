using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.IO;

using VisualRBMInterop;
using System.Text.RegularExpressions;
using System.Diagnostics;

namespace VisualRBM
{
	public partial class SettingsBar : UserControl
	{

		enum ProgramState
		{
			ProgramStarted,
			TrainerInitializing,
			TrainerRunning,
			TrainerPaused,
			TrainerStopped,
			TrainerScheduleLoaded,
			TrainerScheduleRunning,
		}

		bool training_data_loaded = false;
		bool validation_data_loaded = false;
		ProgramState current_state = ProgramState.ProgramStarted;
		ProgramState CurrentState
		{
			set
			{
				this.Invoke(new Action(() => 
				{
					switch(value)
					{
						case ProgramState.TrainerInitializing:
							Debug.Assert(current_state == ProgramState.TrainerStopped);
							
							selectTrainingIdxButton.Enabled = false;
							selectValidationIdxButton.Enabled = false;
							clearValidationDataButton.Enabled = false;

							modelTypeComboBox.Enabled = false;
							visibleTypeComboBox.Enabled = false;
							hiddenTypeComboBox.Enabled = false;
							hiddenUnitsTextBox.Enabled = false;
							
							learningRateTextBox.Enabled = false;
							momentumTextBox.Enabled = false;
							l1TextBox.Enabled = false;
							l2TextBox.Enabled = false;
							visibleDropoutTextBox.Enabled = false;
							hiddenDropoutTextBox.Enabled = false;
							
							minibatchSizeTextBox.Enabled = false;							
							epochsTextBox.Enabled = false;

							loadScheduleButton.Enabled = false;

							importButton.Enabled = false;
							startButton.Enabled = false;
							pauseButton.Enabled = false;
							stopButton.Enabled = false;
							exportButton.Enabled = false;
							break;
						case ProgramState.TrainerRunning:
							Debug.Assert(current_state == ProgramState.TrainerInitializing || current_state == ProgramState.TrainerPaused);

							selectTrainingIdxButton.Enabled = false;
							selectValidationIdxButton.Enabled = false;
							clearValidationDataButton.Enabled = false;

							modelTypeComboBox.Enabled = false;
							visibleTypeComboBox.Enabled = false;
							hiddenTypeComboBox.Enabled = false;
							hiddenUnitsTextBox.Enabled = false;
							
							learningRateTextBox.Enabled = false;
							momentumTextBox.Enabled = false;
							l1TextBox.Enabled = false;
							l2TextBox.Enabled = false;
							visibleDropoutTextBox.Enabled = false;
							hiddenDropoutTextBox.Enabled = false;
							
							minibatchSizeTextBox.Enabled = false;							
							epochsTextBox.Enabled = false;

							loadScheduleButton.Enabled = false;

							importButton.Enabled = false;
							startButton.Enabled = false;
							pauseButton.Enabled = true;
							stopButton.Enabled = true;
							exportButton.Enabled = true;

							break;
						case ProgramState.TrainerPaused:
							selectTrainingIdxButton.Enabled = false;
							selectValidationIdxButton.Enabled = false;
							clearValidationDataButton.Enabled = false;

							modelTypeComboBox.Enabled = false;
							visibleTypeComboBox.Enabled = false;
							hiddenTypeComboBox.Enabled = false;
							hiddenUnitsTextBox.Enabled = false;
							
							learningRateTextBox.Enabled = true;
							momentumTextBox.Enabled = true;
							l1TextBox.Enabled = true;
							l2TextBox.Enabled = true;
							
							visibleDropoutTextBox.Enabled = true;
							hiddenDropoutTextBox.Enabled = true;
							
							minibatchSizeTextBox.Enabled = false;							
							epochsTextBox.Enabled = true;

							loadScheduleButton.Enabled = true;

							importButton.Enabled = false;
							startButton.Enabled = true;
							pauseButton.Enabled = false;
							stopButton.Enabled = true;
							exportButton.Enabled = current_state != ProgramState.TrainerStopped;
							break;
						case ProgramState.TrainerStopped:
							selectTrainingIdxButton.Enabled = true;
							selectValidationIdxButton.Enabled = training_data_loaded;
							clearValidationDataButton.Enabled = validation_data_loaded;

							modelTypeComboBox.Enabled = true;
							visibleTypeComboBox.Enabled = true;
							hiddenTypeComboBox.Enabled = true;
							hiddenUnitsTextBox.Enabled = true;
							
							learningRateTextBox.Enabled = true;
							momentumTextBox.Enabled = true;
							l1TextBox.Enabled = true;
							l2TextBox.Enabled = true;
							visibleDropoutTextBox.Enabled = true;
							hiddenDropoutTextBox.Enabled = true;
							
							minibatchSizeTextBox.Enabled = true;							
							epochsTextBox.Enabled = true;

							loadScheduleButton.Enabled = true;

							importButton.Enabled = true;
							startButton.Enabled = true;
							pauseButton.Enabled = false;
							stopButton.Enabled = false;
							exportButton.Enabled = false;
							break;
						case ProgramState.TrainerScheduleLoaded:
							selectTrainingIdxButton.Enabled = false;
							selectValidationIdxButton.Enabled = false;
							clearValidationDataButton.Enabled = false;

							modelTypeComboBox.Enabled = false;
							visibleTypeComboBox.Enabled = false;
							hiddenTypeComboBox.Enabled = false;
							hiddenUnitsTextBox.Enabled = false;

							learningRateTextBox.Enabled = false;
							momentumTextBox.Enabled = false;
							l1TextBox.Enabled = false;
							l2TextBox.Enabled = false;

							visibleDropoutTextBox.Enabled = false;
							hiddenDropoutTextBox.Enabled = false;

							minibatchSizeTextBox.Enabled = false;
							epochsTextBox.Enabled = false;

							loadScheduleButton.Enabled = false;

							importButton.Enabled = false;
							startButton.Enabled = true;
							pauseButton.Enabled = false;
							stopButton.Enabled = true;
							exportButton.Enabled = false;
							break;
						case ProgramState.TrainerScheduleRunning:
							selectTrainingIdxButton.Enabled = false;
							selectValidationIdxButton.Enabled = false;
							clearValidationDataButton.Enabled = false;

							modelTypeComboBox.Enabled = false;
							visibleTypeComboBox.Enabled = false;
							hiddenTypeComboBox.Enabled = false;
							hiddenUnitsTextBox.Enabled = false;

							learningRateTextBox.Enabled = false;
							momentumTextBox.Enabled = false;
							l1TextBox.Enabled = false;
							l2TextBox.Enabled = false;

							visibleDropoutTextBox.Enabled = false;
							hiddenDropoutTextBox.Enabled = false;

							minibatchSizeTextBox.Enabled = false;
							epochsTextBox.Enabled = false;

							loadScheduleButton.Enabled = false;

							importButton.Enabled = false;
							startButton.Enabled = false;
							pauseButton.Enabled = false;
							stopButton.Enabled = true;
							exportButton.Enabled = true;
							break;
					}
					current_state = value;
				}));
			}
			get
			{
				return current_state;
			}
		}


		Main _main_form;

		internal Main MainForm
		{
			get
			{
				return _main_form;
			}
			set
			{
				_main_form = value;
				// set initial values of all our forms

				// model
				modelTypeComboBox.SelectedItem = Processor.Model;
				visibleTypeComboBox.SelectedItem = Processor.VisibleType;
				hiddenTypeComboBox.SelectedItem = Processor.HiddenType;
				hiddenUnitsTextBox.Text = Processor.HiddenUnits.ToString();
				// parameters
				learningRateTextBox.Text = Processor.LearningRate.ToString();
				momentumTextBox.Text = Processor.Momentum.ToString();
				l1TextBox.Text = Processor.L1Regularization.ToString();
				l2TextBox.Text = Processor.L2Regularization.ToString();
				visibleDropoutTextBox.Text = Processor.VisibleDropout.ToString();
				hiddenDropoutTextBox.Text = Processor.HiddenDropout.ToString();
				
				minibatchSizeTextBox.Text = Processor.MinibatchSize.ToString();
				epochsTextBox.Text = Processor.Epochs.ToString();
			}
		}

		// add a tool tip
		private void SetToolTip(string s, params Control[] c)
		{

			// makes lines try to fit on 50 character wide lines
			const int line_length = 50;

			StringWriter sw = new StringWriter();

			string[] tokens = s.Split(' ', '\t', '\n', '\r');

			int current_length = 0;
			for (int k = 0; k < tokens.Length; k++)
			{
				// new line time
				if (current_length + tokens[k].Length > line_length)
				{
					sw.Write('\n');
					current_length = 0;
				}

				sw.Write(tokens[k]);
				sw.Write(' ');
				current_length += tokens[k].Length + 1;
			}
			s = sw.ToString();

			// build or tooltip
			ToolTip t = new ToolTip();
			t.ShowAlways = true;
			t.InitialDelay= 100;
			foreach (Control cont in c)
			{
				t.SetToolTip(cont, s);
			}
		}

		public SettingsBar()
		{
			InitializeComponent();
			// fill in model types combobox
			modelTypeComboBox.Items.Add(VisualRBMInterop.ModelType.RBM);
			modelTypeComboBox.Items.Add(VisualRBMInterop.ModelType.AutoEncoder);
			// fill in visible type combobox
			visibleTypeComboBox.Items.Add(VisualRBMInterop.UnitType.Sigmoid);
			visibleTypeComboBox.Items.Add(VisualRBMInterop.UnitType.Linear);
			visibleTypeComboBox.Items.Add(VisualRBMInterop.UnitType.RectifiedLinear);
			visibleTypeComboBox.Items.Add(VisualRBMInterop.UnitType.Softmax);
			// fill in hidden type combobox
			hiddenTypeComboBox.Items.Add(VisualRBMInterop.UnitType.Sigmoid);
			hiddenTypeComboBox.Items.Add(VisualRBMInterop.UnitType.Linear);
			hiddenTypeComboBox.Items.Add(VisualRBMInterop.UnitType.RectifiedLinear);
			hiddenTypeComboBox.Items.Add(VisualRBMInterop.UnitType.Softmax);

			// setup all our tooltips
			SetToolTip("Load training dataset", this.selectTrainingIdxButton);
			SetToolTip("Load validation dataset", this.selectValidationIdxButton);
			SetToolTip("Clear out the validation data", this.clearValidationDataButton);
			SetToolTip("Format of pixel data representation of training images (for visualization)", this.pixelFormatComboBox);
			SetToolTip("Width of training image in pixels (for visualization)", this.widthTextBox);
			SetToolTip("Height of training image in pixels (for visualization)", this.heightTextBox);
			SetToolTip("Model type to train", this.modelTypeComboBox);
			SetToolTip("Activation function for visible units", this.visibleTypeComboBox);
			SetToolTip("Activation function for hidden units", this.hiddenTypeComboBox);
			SetToolTip("Number of visible units in each training vector of loaded training data", this.visibleUnitsLabel);
			SetToolTip("Number of hidden units to use in trained model", this.hiddenUnitsTextBox);
			SetToolTip("Load schedule file", this.loadScheduleButton);
			SetToolTip("Speed of learning; reasonable values are around 0.001 (less for Gaussian visible units)", this.learningRateTextBox);
			SetToolTip("Smooths learning by having the previous weight update contribute to the current update by the given percent.  Valid values range from 0 to 1.", this.momentumTextBox);
			SetToolTip("L1 Regularization punishes all weights equally.  Many weights will be near zero with this method while others will grow large.", this.l1TextBox);
			SetToolTip("L2 Regularization more heavily punishes large weights, resulting in blurrier weights that can be harder to interpret", this.l2TextBox);
			SetToolTip("Probability any given visible unit will not be activated during a single training update.", this.visibleDropoutTextBox);
			SetToolTip("Probability any given hidden unit will not be activated during a single training update.", this.hiddenDropoutTextBox);
			SetToolTip("Number of training to vectors to show the model for each weight update", this.minibatchSizeTextBox);
			SetToolTip("The number of weight updates to perform", this.epochsTextBox);
			SetToolTip("Import RBM for further training", this.importButton);
			SetToolTip("Start Training", this.startButton);
			SetToolTip("Stop Training", this.stopButton);
			SetToolTip("Pause Training", this.pauseButton);
			SetToolTip("Save RBM to disk", this.exportButton);

			// add event to update epochs textbox
			Processor.EpochCompleted += new Processor.EpochCompletedHandler((uint epoch, uint total) => 
			{
				epochsTextBox.Invoke(new Action(() => 
				{
					epochsTextBox.Text = (total - epoch).ToString();
				}));
			});
			Processor.TrainingCompleted += new Processor.TrainingCompletedHandler(() =>
			{
				this.Invoke(new Action(() =>
				{
					epochsTextBox.Text = "100";
					updateEpochs(epochsTextBox);

					CurrentState = ProgramState.TrainerPaused;
				}));
			});
			// callbacks for our UI to change items
			// model parameters
			Processor.ModelTypeChanged += new Processor.ValueChangedHandler((Object obj) =>
			{
				this.BeginInvoke(new Action(() => { modelTypeComboBox.SelectedItem = (ModelType)obj; }));
				_main_form.trainingLog.AddLog("Model Type = {0}", obj);
			});
			Processor.VisibleTypeChanged += new Processor.ValueChangedHandler((Object obj) =>
			{
				this.BeginInvoke(new Action(() => { visibleTypeComboBox.SelectedItem = (UnitType)obj; }));
				_main_form.trainingLog.AddLog("Visible Type = {0}", obj);
			});
			Processor.HiddenTypeChanged += new Processor.ValueChangedHandler((Object obj) =>
			{
				this.BeginInvoke(new Action(() => { hiddenTypeComboBox.SelectedItem = (UnitType)obj; }));
				_main_form.trainingLog.AddLog("Hidden Type = {0}", obj);
			});
			Processor.HiddenUnitsChanged += new Processor.ValueChangedHandler((Object obj) =>
			{
				this.BeginInvoke(new Action(() => { hiddenUnitsTextBox.Text = obj.ToString(); }));
				_main_form.trainingLog.AddLog("Hidden Units = {0}", obj);
			});
			// training parameters
			Processor.LearningRateChanged += new Processor.ValueChangedHandler((Object obj) =>
			{
				this.BeginInvoke(new Action(() => { learningRateTextBox.Text = obj.ToString(); }));
				_main_form.trainingLog.AddLog("Learning Rate = {0}", obj);
			});
			Processor.MomentumChanged += new Processor.ValueChangedHandler((Object obj) =>
			{
				this.BeginInvoke(new Action(() => { momentumTextBox.Text = obj.ToString(); }));
				_main_form.trainingLog.AddLog("Momentum = {0}", obj);
			});
			Processor.L1RegularizationChanged += new Processor.ValueChangedHandler((Object obj) =>
			{
				this.BeginInvoke(new Action(() => { l1TextBox.Text = obj.ToString(); }));
				_main_form.trainingLog.AddLog("L1Regularization = {0}", obj);
			});
			Processor.L2RegularizationChanged += new Processor.ValueChangedHandler((Object obj) =>
			{
				this.BeginInvoke(new Action(() => { l2TextBox.Text = obj.ToString(); }));
				_main_form.trainingLog.AddLog("L2Regularization = {0}", obj);
			});
			Processor.VisibleDropoutChanged += new Processor.ValueChangedHandler((Object obj) =>
			{
				this.BeginInvoke(new Action(() => { visibleDropoutTextBox.Text = obj.ToString(); }));
				_main_form.trainingLog.AddLog("Visible Dropout = {0}", obj);
			});
			Processor.HiddenDropoutChanged += new Processor.ValueChangedHandler((Object obj) =>
			{
				this.BeginInvoke(new Action(() => { hiddenDropoutTextBox.Text = obj.ToString(); }));
				_main_form.trainingLog.AddLog("Hidden Dropout = {0}", obj);
			});

			Processor.MinibatchSizeChanged += new Processor.ValueChangedHandler((Object obj) =>
			{
				this.BeginInvoke(new Action(() => { minibatchSizeTextBox.Text = obj.ToString(); }));
				_main_form.trainingLog.AddLog("Minibatch Size = {0}", obj);
			});
			Processor.EpochsChanged += new Processor.ValueChangedHandler((Object obj) =>
			{
				this.BeginInvoke(new Action(() => { epochsTextBox.Text = obj.ToString(); }));
				_main_form.trainingLog.AddLog("Epochs = {0}", obj);
			});
		}

		public int ImageWidth;
		public int ImageHeight;
		public PixelFormat Format = PixelFormat.Lightness;
		

		protected uint SquareRoot(uint a_nInput)
		{
			uint op = a_nInput;
			uint res = 0;
			uint one = 1u << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type

			// "one" starts at the highest power of four <= than the argument.
			while (one > op)
			{
				one >>= 2;
			}

			while (one != 0)
			{
				if (op >= res + one)
				{
					op = op - (res + one);
					res = res + 2 * one;
				}
				res >>= 1;
				one >>= 2;
			}
			return res;
		}

		/** Data Section Methods **/

		#region IDX File Select
		private void selectIdxButton_Click(object sender, EventArgs e)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Filter = "IDX|*.idx";
			ofd.Multiselect = false;
			if (ofd.ShowDialog() == DialogResult.OK)
			{
				_main_form.Cursor = Cursors.WaitCursor;
				// load IDX file
				if (Processor.SetTrainingData(ofd.FileName) == false)
				{
					_main_form.Cursor = Cursors.Default;
					return;
				}
				_main_form.Cursor = Cursors.Default;

				// update labels
				traingingIdxPathTextBox.Text = ofd.FileName;
				visibleUnitsLabel.Text = Processor.VisibleUnits.ToString();

				int total_vals = Processor.VisibleUnits;
				ImageWidth = (int)SquareRoot((uint)total_vals);
				ImageHeight = total_vals / ImageWidth;
				if (ImageHeight * ImageWidth < total_vals)
					++ImageHeight;

				widthTextBox.Text = ImageWidth.ToString();
				heightTextBox.Text = ImageHeight.ToString();
				pixelFormatComboBox.SelectedIndex = 0;

				training_data_loaded = true;
				validation_data_loaded = false;

				_main_form.trainingLog.AddLog("Loaded Training .IDX: {0}", ofd.FileName);

				// get rid of any validation data that's here
				Processor.SetValidationData(null);
				validationIdxPathTextBox.Text = "";

				if (current_state == ProgramState.ProgramStarted)
				{
					CurrentState = ProgramState.TrainerStopped;
				}
			}
		}

		private void selectValidationIdxButton_Click(object sender, EventArgs e)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Filter = "IDX|*.idx";
			ofd.Multiselect = false;
			if (ofd.ShowDialog() == DialogResult.OK)
			{
				Cursor.Current = Cursors.WaitCursor;
				if (Processor.SetValidationData(ofd.FileName) == false)
				{
					Cursor.Current = Cursors.Default;
					return;
				}
				Cursor.Current = Cursors.Default;

				validationIdxPathTextBox.Text = ofd.FileName;

				_main_form.trainingLog.AddLog("Loaded Validation .IDX: {0}", ofd.FileName);

				validation_data_loaded = true;
			}
		}

		private void clearValidationDataButton_Click(object sender, EventArgs e)
		{
			Processor.SetValidationData(null);
			validationIdxPathTextBox.Text = "";

			_main_form.trainingLog.AddLog("Cleared Validation .IDX");

			validation_data_loaded = false;
		}


		#endregion

		#region Set Data Dimensions and Color Format

		private void pixelFormatComboBox_SelectionChangeCommitted(object sender, EventArgs e)
		{
			ComboBox cb = sender as ComboBox;
			Format = (PixelFormat)cb.SelectedItem;
		}

		private void update_dimension(TextBox tb, ref int old_val)
		{
			if (tb == null) return;

			int new_val;
			if (int.TryParse(tb.Text, out new_val) && new_val > 0)
			{
				old_val = new_val;
				this._main_form.visualizeWeights.UpdateImageControlDimensions();
				this._main_form.visualizeVisible.UpdateImageControlDimensions();
				return;
			}

			tb.Text = old_val.ToString();

			
		}

		private void widthTextBox_KeyDown(object sender, KeyEventArgs e)
		{
			if(e.KeyCode == Keys.Enter)
				update_dimension(sender as TextBox, ref ImageWidth);
		}

		private void widthTextBox_Leave(object sender, EventArgs e)
		{
			update_dimension(sender as TextBox, ref ImageWidth);
		}

		private void heightTextBox_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Enter)
				update_dimension(sender as TextBox, ref ImageHeight);
		}

		private void heightTextBox_Leave(object sender, EventArgs e)
		{
			update_dimension(sender as TextBox, ref ImageHeight);
		}

		#endregion

		#region Model Select
		private void modelTypeComboBox_SelectionChangeCommitted(object sender, EventArgs e)
		{
			ComboBox cb = sender as ComboBox;

			Processor.Model = (VisualRBMInterop.ModelType)cb.SelectedItem;
		}
		#endregion

		#region Visible Unit Select
		private void visibleTypeComboBox_SelectionChangeCommitted(object sender, EventArgs e)
		{
			ComboBox cb = sender as ComboBox;
			if (Processor.VisibleType != (VisualRBMInterop.UnitType)cb.SelectedItem)
			{
				Processor.VisibleType = (VisualRBMInterop.UnitType)cb.SelectedItem;
			}
		}
		#endregion
		
		#region Hidden Unit Select
		private void hiddenTypeComboBox_SelectionChangeCommitted(object sender, EventArgs e)
		{
			ComboBox cb = sender as ComboBox;
			if (Processor.HiddenType != (VisualRBMInterop.UnitType)cb.SelectedItem)
			{
				Processor.HiddenType = (VisualRBMInterop.UnitType)cb.SelectedItem;
			}
		}
		#endregion

		
		#region Set Hidden Units
		private bool updateHiddenUnits(TextBox tb)
		{
			if (tb == null) return false;

			uint hidden; 
			if (uint.TryParse(tb.Text, out hidden))
			{
				if ((int)hidden != Processor.HiddenUnits)
				{
					Processor.HiddenUnits = (int)hidden;
					tb.Text = hidden.ToString();
					return true;
				}
				return false;
			}
			tb.Text = Processor.HiddenUnits.ToString();
			return false;
		}

		private void hiddenUnitsTextBox_KeyPress(object sender, KeyPressEventArgs e)
		{
			if (e.KeyChar == '\r' && updateHiddenUnits(sender as TextBox))
			{
				e.Handled = true;
			}
		}

		private void hiddenUnitsTextBox_Leave(object sender, EventArgs e)
		{
			updateHiddenUnits(sender as TextBox);
		}
		#endregion

		#region Set Learning Rate
		private bool updateLearningRate(TextBox tb)
		{
			if (tb == null) return false;

			float learning_rate;
			if(float.TryParse(tb.Text, out learning_rate) && learning_rate > 0.0f)
			{
				if(learning_rate != Processor.LearningRate)
				{
					Processor.LearningRate = learning_rate;
					tb.Text = learning_rate.ToString();
					return true;
				}
			}
			tb.Text = Processor.LearningRate.ToString();
			return false;
		}

		private void learningRateTextBox_KeyPress(object sender, KeyPressEventArgs e)
		{
			if (e.KeyChar == '\r' && updateLearningRate(sender as TextBox))
				e.Handled = true;
		}

		private void learningRateTextBox_Leave(object sender, EventArgs e)
		{
			updateLearningRate(sender as TextBox);
		}
		#endregion

		#region Set Momentum
		private bool updateMomentum(TextBox tb)
		{
			if (tb == null) return false;

			float momentum;
			if (float.TryParse(tb.Text, out momentum))
			{
				momentum = momentum > 0.0f ? (momentum < 1.0f ? momentum : 1.0f) : 0.0f;

				if (momentum != Processor.Momentum)
				{
					Processor.Momentum = momentum;
					tb.Text = momentum.ToString();
					return true;
				}
				return false;
			}
			tb.Text = Processor.Momentum.ToString();
			return false;
		}

		private void momentumTextBox_KeyPress(object sender, KeyPressEventArgs e)
		{
			if (e.KeyChar == '\r' && updateMomentum(sender as TextBox))
				e.Handled = true;
		}

		private void momentumTextBox_Leave(object sender, EventArgs e)
		{
			updateMomentum(sender as TextBox);
		}
		#endregion

		#region Set Regularization

		private bool updateL1(TextBox tb)
		{
			if (tb == null)
				return false;

			float l1;
			if (float.TryParse(tb.Text, out l1) && l1 >= 0.0f)
			{
				if (l1 != Processor.L1Regularization)
				{
					Processor.L1Regularization = l1;
					return true;
				}
				return false;
			}
			tb.Text = Processor.L1Regularization.ToString();
			return false;
		}

		private void l1TextBox_Leave(object sender, EventArgs e)
		{
			updateL1(sender as TextBox);
		}

		private void l1TextBox_KeyPress(object sender, KeyPressEventArgs e)
		{
			if (e.KeyChar == '\r' && updateL1(sender as TextBox))
				e.Handled = true;
		}

		private bool updateL2(TextBox tb)
		{
			if (tb == null)
				return false;

			float l2;
			if (float.TryParse(tb.Text, out l2) && l2 >= 0.0f)
			{
				if (l2 != Processor.L2Regularization)
				{
					Processor.L2Regularization = l2;
					return true;
				}
				return false;
			}
			tb.Text = Processor.L2Regularization.ToString();
			return false;
		}

		private void l2TextBox_Leave(object sender, EventArgs e)
		{
			updateL2(sender as TextBox);
		}

		private void l2TextBox_KeyPress(object sender, KeyPressEventArgs e)
		{
			if (e.KeyChar == '\r' && updateL2(sender as TextBox))
				e.Handled = true;
		}

		#endregion

		#region Set Dropout

		private bool updateDropout(TextBox tb)
		{
			if (tb == null)
				return false;

			float db;
			if (float.TryParse(tb.Text, out db) && db >= 0.0f && db < 1.0f)
			{
				if (tb == visibleDropoutTextBox)
				{
					if (db != Processor.VisibleDropout)
					{
						Processor.VisibleDropout = db;
					}
					else
					{
						return false;
					}
				}
				else if (tb == hiddenDropoutTextBox)
				{
					if (db != Processor.HiddenDropout)
					{
						Processor.HiddenDropout = db;
					}
					else
					{
						return false;
					}
				}
				return true;
			}
			else
			{
				if (tb == visibleDropoutTextBox)
				{
					visibleDropoutTextBox.Text = Processor.VisibleDropout.ToString();
				}
				else if (tb == hiddenDropoutTextBox)
				{
					hiddenDropoutTextBox.Text = Processor.HiddenDropout.ToString();
				}
			}

			return false;
		}

		private void visibleDropoutTextBox_Leave(object sender, EventArgs e)
		{
			updateDropout(sender as TextBox);
		}

		private void visibleDropoutTextBox_KeyPress(object sender, KeyPressEventArgs e)
		{
			if (e.KeyChar == '\r' && updateDropout(sender as TextBox))
				e.Handled = true;
		}

		private void hiddenDropoutTextBox_Leave(object sender, EventArgs e)
		{
			updateDropout(sender as TextBox);
		}

		private void hiddenDropoutTextBox_KeyPress(object sender, KeyPressEventArgs e)
		{
			if (e.KeyChar == '\r' && updateDropout(sender as TextBox))
				e.Handled = true;
		}

		#endregion

		#region Set Minibatch
		private bool updateMinibatch(TextBox tb)
		{
			if (tb == null) return false;

			uint minibatch;
			if(uint.TryParse(tb.Text, out minibatch))
			{
				if ((int)minibatch != Processor.MinibatchSize)
				{
					Processor.MinibatchSize = (int)minibatch;
					tb.Text = minibatch.ToString();
					return true;
				}
				return false;
			}
			tb.Text = Processor.MinibatchSize.ToString();
			return false;
		}

		private void minibatchSizeTextBox_KeyPress(object sender, KeyPressEventArgs e)
		{
			if (e.KeyChar == '\r' && updateMinibatch(sender as TextBox))
				e.Handled = true;
		}

		private void minibatchSizeTextBox_Leave(object sender, EventArgs e)
		{
			updateMinibatch(sender as TextBox);
		}
		#endregion

		#region Set Epochs

		private bool updateEpochs(TextBox tb)
		{
			if (tb == null) return false;
			uint epochs;
			if(uint.TryParse(tb.Text, out epochs))
			{
				if ((uint)epochs != Processor.Epochs)
				{
					// user should not be able to set epochs to 0
					if (epochs == 0)
						epochs = 1;

					// set number of epochs to train for in the processor
					Processor.Epochs = (uint)epochs;

					tb.Text = epochs.ToString();
					// if we have data, then we are in the middle of training and start button should be enabled
					// even though training may have finished already
					if (Processor.HasTrainingData)
					{
						this.startButton.Enabled = true;
					}

					return true;
				}
				return false;
			}
			// set text based on value in rbmprocessor
			tb.Text = Processor.Epochs.ToString();
			return false;
		}

		private void epochsTextBox_KeyPress(object sender, KeyPressEventArgs e)
		{
			if (e.KeyChar == '\r' && updateEpochs(sender as TextBox))
				e.Handled = true;
		}

		private void epochsTextBox_Leave(object sender, EventArgs e)
		{
			updateEpochs(sender as TextBox);
		}

		#endregion

		#region Big Buttons

		private void startButton_Click(object sender, EventArgs e)
		{
			// if we have no data then don't do anything
			if (Processor.HasTrainingData)
			{
				// start visualization
				_main_form.visualizeReconstructionError.Start();
				_main_form.visualizeVisible.Start();
				_main_form.visualizeHidden.Start();
				_main_form.visualizeWeights.Start();
				
				// log this shit
				_main_form.trainingLog.AddLog("Training Started");

				

				if (CurrentState == ProgramState.TrainerStopped)
				{
					_main_form.Cursor = Cursors.WaitCursor;
					CurrentState = ProgramState.TrainerInitializing;
					
					// start training, give a callback to update our current_state
					Processor.Start(new Action(() =>
					{
						this.Invoke(new Action(() => _main_form.Cursor = Cursors.Default));
						this.CurrentState = ProgramState.TrainerRunning;
					}));
				}
				else if (CurrentState == ProgramState.TrainerPaused)
				{
					CurrentState = ProgramState.TrainerRunning;
					Processor.Start(null);	// no callback needed
				}
				else if (CurrentState == ProgramState.TrainerScheduleLoaded)
				{
					CurrentState = ProgramState.TrainerScheduleRunning;
					Processor.Start(null);	// no callback needed
				}
				else
				{
					Debug.Assert(false);
				}
			}
		}

		private void stopButton_Click(object sender, EventArgs e)
		{
			CurrentState = ProgramState.TrainerStopped;

			_main_form.visualizeReconstructionError.Stop();
			_main_form.visualizeVisible.Stop();
			_main_form.visualizeHidden.Stop();
			_main_form.visualizeWeights.Stop();

			_main_form.trainingLog.AddLog("Training Stopped");

			Processor.Stop();
		}

		private void pauseButton_Click(object sender, EventArgs e)
		{
			CurrentState = ProgramState.TrainerPaused;

			_main_form.visualizeVisible.Pause();
			_main_form.visualizeHidden.Pause();
			_main_form.visualizeWeights.Pause();

			_main_form.trainingLog.AddLog("Training Paused");

			Processor.Pause();
		}

		private void exportButton_Click(object sender, EventArgs e)
		{
			SaveFileDialog sfd = new SaveFileDialog();
			sfd.Filter = "RBM,AutoEncoder|*.json";

			if (sfd.ShowDialog() == DialogResult.OK)
			{
				_main_form.Cursor = Cursors.WaitCursor;
				bool saved;
				try
				{
					saved = Processor.SaveModel(sfd.OpenFile());
				}
				catch (Exception)
				{
					saved = false;
				}

				if (saved)
				{
					_main_form.trainingLog.AddLog("Exported RBM to {0}", sfd.FileName);
					_main_form.Cursor = Cursors.Default;
				}
				else
				{
					String error = String.Format("Error saving model to {0}", sfd.FileName);
					_main_form.trainingLog.AddLog(error);
					_main_form.Cursor = Cursors.Default;
					MessageBox.Show(error);
				}
			}
		}

		private void importButton_Click(object sender, EventArgs e)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Filter = "RBM,AutoEncoder|*.json";

			if (ofd.ShowDialog() == DialogResult.OK)
			{
				_main_form.Cursor = Cursors.WaitCursor;
				bool loaded;
				try
				{
					loaded = Processor.LoadModel(ofd.OpenFile());
				}
				catch(Exception)
				{
					loaded = false;
				}

				if(loaded)
				{
					_main_form.trainingLog.AddLog("Imported RBM from {0}", ofd.FileName);
					
					visibleUnitsLabel.Text = Processor.VisibleUnits.ToString();
					hiddenUnitsTextBox.Text = Processor.HiddenUnits.ToString();
					visibleTypeComboBox.SelectedItem = Processor.VisibleType;
					hiddenTypeComboBox.SelectedItem = Processor.HiddenType;
					modelTypeComboBox.SelectedItem = Processor.Model;

					CurrentState = ProgramState.TrainerPaused;
					_main_form.Cursor = Cursors.Default;
				}
				else
				{
					String error = String.Format("Error loading model from {0}", ofd.FileName);
					_main_form.trainingLog.AddLog(error);
					_main_form.Cursor = Cursors.Default;
					MessageBox.Show(error);
				}
			}
		}

		#endregion

		#region Parameter Saving/Loading

		private void loadParametersButton_Click(object sender, EventArgs e)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Filter = "Visual RBM Training Schedule|*.json";
			if(ofd.ShowDialog() == DialogResult.OK)
			{
				_main_form.Cursor = Cursors.WaitCursor;
				Stream stream = null;
				try
				{
					stream = ofd.OpenFile();
				}
				catch (Exception) {}

				if (stream == null)
				{
					String error = String.Format("Problem loading training schedule from {0}", ofd.FileName);
					_main_form.trainingLog.AddLog(error);
					_main_form.Cursor = Cursors.Default;
					MessageBox.Show(error);
				}
				else if (Processor.LoadTrainingSchedule(stream) == false)
				{
					String error = String.Format("Error parsing training schedule JSON from {0}", ofd.FileName);
					_main_form.trainingLog.AddLog(error);
					_main_form.Cursor = Cursors.Default;
					MessageBox.Show(error);
				}
				else
				{
					this.CurrentState = ProgramState.TrainerScheduleLoaded;
					_main_form.Cursor = Cursors.Default;
					_main_form.trainingLog.AddLog("Loaded training schedule from {0}", ofd.FileName);
				}
			}
		}

		private void saveParametersButton_Click(object sender, EventArgs e)
		{
			return;
			SaveFileDialog sfd = new SaveFileDialog();
			sfd.Filter = "Visual RBM Training Parameters|*.vrbmparameters";
			if (sfd.ShowDialog() == DialogResult.OK)
			{
				// save some settings
				try
				{
					using (StreamWriter sw = new StreamWriter(new FileStream(sfd.FileName, FileMode.Create)))
					{
						sw.WriteLine("model = RBM");
						sw.WriteLine("visible_type = {0}", Processor.VisibleType);
						sw.WriteLine("hidden_units = {0}", Processor.HiddenUnits);
						sw.WriteLine("learning_rate = {0}", Processor.LearningRate);
						sw.WriteLine("momentum = {0}", Processor.Momentum);
						sw.WriteLine("l1_regularization = {0}", Processor.L1Regularization);
						sw.WriteLine("l2_regularization = {0}", Processor.L2Regularization);
						sw.WriteLine("visible_dropout = {0}", Processor.VisibleDropout);
						sw.WriteLine("hidden_dropout = {0}", Processor.HiddenDropout);
						sw.WriteLine("minibatch_size = {0}", Processor.MinibatchSize);
						sw.WriteLine("epochs = {0}", Processor.Epochs);

						_main_form.trainingLog.AddLog("Saved Parameters to {0}", sfd.FileName);
					}
				}
				catch (System.Exception ex)
				{
					MessageBox.Show(String.Format("Problem Writing Parameters to \"{0}\"", sfd.FileName));
				}
				
			}
		}

		private void resetParametersButton_Click(object sender, EventArgs e)
		{
			return;
			if (MessageBox.Show("Reset training parameters to defaults?", "", MessageBoxButtons.YesNo) == DialogResult.Yes)
			{
				Processor.LearningRate = 0.001f;
				Processor.Momentum = 0.5f;
				Processor.L1Regularization = 0.0f;
				Processor.L2Regularization = 0.0f;
				Processor.VisibleDropout = 0.0f;
				Processor.HiddenDropout = 0.5f;
				Processor.MinibatchSize = 10;
				Processor.Epochs = 100;

				learningRateTextBox.Text = Processor.LearningRate.ToString();
				momentumTextBox.Text = Processor.Momentum.ToString();
				l1TextBox.Text = Processor.L1Regularization.ToString();
				l2TextBox.Text = Processor.L2Regularization.ToString();
				visibleDropoutTextBox.Text = Processor.VisibleDropout.ToString();
				hiddenDropoutTextBox.Text = Processor.HiddenDropout.ToString();
				minibatchSizeTextBox.Text = Processor.MinibatchSize.ToString();
				epochsTextBox.Text = Processor.Epochs.ToString();
				
				_main_form.trainingLog.AddLog("Reset Parameters to Defaults");
			}
		}


		#endregion


	}
}
