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

using QuickBoltzmann;
using System.Text.RegularExpressions;
using System.Diagnostics;

namespace VisualRBM
{
	public partial class SettingsBar : UserControl
	{

		enum ProgramState
		{
			None = 0,
			Started,
			TrainerInitializing,
			TrainerRunning,
			TrainerPaused,
			TrainerStopped
		}

		bool training_data_loaded = false;
		bool validation_data_loaded = false;
		ProgramState current_state = ProgramState.None;
		ProgramState CurrentState
		{
			set
			{
				this.Invoke(new Action(() => 
				{
					switch(value)
					{
						case ProgramState.Started:
							Debug.Assert(current_state == ProgramState.None);

							selectTrainingIdxButton.Enabled = true;
							selectValidationIdxButton.Enabled = false;
							clearValidationDataButton.Enabled = false;

							modelTypeComboBox.Enabled = false;
							visibleTypeComboBox.Enabled = false;
							hiddenUnitsTextBox.Enabled = false;
							
							loadParametersButton.Enabled = false;
							saveParametersButton.Enabled = false;
							resetParametersButton.Enabled = false;
							
							learningRateTextBox.Enabled = false;
							momentumTextBox.Enabled = false;
							l1TextBox.Enabled = false;
							l2TextBox.Enabled = false;
							minibatchSizeTextBox.Enabled = false;
							epochsTextBox.Enabled = false;

							importButton.Enabled = false;
							startButton.Enabled = false;
							pauseButton.Enabled = false;
							stopButton.Enabled = false;
							exportButton.Enabled = false;

							break;
						case ProgramState.TrainerInitializing:
							Debug.Assert(current_state == ProgramState.TrainerStopped);
							
							selectTrainingIdxButton.Enabled = false;
							selectValidationIdxButton.Enabled = false;
							clearValidationDataButton.Enabled = false;

							modelTypeComboBox.Enabled = false;
							visibleTypeComboBox.Enabled = false;
							hiddenUnitsTextBox.Enabled = false;
							
							loadParametersButton.Enabled = false;
							saveParametersButton.Enabled = false;
							resetParametersButton.Enabled = false;
							
							learningRateTextBox.Enabled = false;
							momentumTextBox.Enabled = false;
							l1TextBox.Enabled = false;
							l2TextBox.Enabled = false;
							minibatchSizeTextBox.Enabled = false;							
							epochsTextBox.Enabled = false;

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
							hiddenUnitsTextBox.Enabled = false;
							
							loadParametersButton.Enabled = false;
							saveParametersButton.Enabled = false;
							resetParametersButton.Enabled = false;
							
							learningRateTextBox.Enabled = false;
							momentumTextBox.Enabled = false;
							l1TextBox.Enabled = false;
							l2TextBox.Enabled = false;
							minibatchSizeTextBox.Enabled = false;							
							epochsTextBox.Enabled = false;

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
							hiddenUnitsTextBox.Enabled = false;
							
							loadParametersButton.Enabled = true;
							saveParametersButton.Enabled = true;
							resetParametersButton.Enabled = true;
							
							learningRateTextBox.Enabled = true;
							momentumTextBox.Enabled = true;
							l1TextBox.Enabled = true;
							l2TextBox.Enabled = true;
							minibatchSizeTextBox.Enabled = false;							
							epochsTextBox.Enabled = true;

							importButton.Enabled = false;
							startButton.Enabled = true;
							pauseButton.Enabled = false;
							stopButton.Enabled = true;
							exportButton.Enabled = true;
							break;
						case ProgramState.TrainerStopped:
							selectTrainingIdxButton.Enabled = true;
							selectValidationIdxButton.Enabled = training_data_loaded;
							clearValidationDataButton.Enabled = validation_data_loaded;

							modelTypeComboBox.Enabled = true;
							visibleTypeComboBox.Enabled = true;
							hiddenUnitsTextBox.Enabled = true;
							
							loadParametersButton.Enabled = true;
							saveParametersButton.Enabled = true;
							resetParametersButton.Enabled = true;
							
							learningRateTextBox.Enabled = true;
							momentumTextBox.Enabled = true;
							l1TextBox.Enabled = true;
							l2TextBox.Enabled = true;
							minibatchSizeTextBox.Enabled = true;							
							epochsTextBox.Enabled = true;

							importButton.Enabled = true;
							startButton.Enabled = true;
							pauseButton.Enabled = false;
							stopButton.Enabled = false;
							exportButton.Enabled = false;
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
				modelTypeComboBox.SelectedItem = RBMProcessor.Model;
				visibleTypeComboBox.SelectedItem = RBMProcessor.VisibleType;
				hiddenUnitsTextBox.Text = RBMProcessor.HiddenUnits.ToString();
				// parameters
				learningRateTextBox.Text = RBMProcessor.LearningRate.ToString();
				momentumTextBox.Text = RBMProcessor.Momentum.ToString();
				l1TextBox.Text = RBMProcessor.L1Regularization.ToString();
				l2TextBox.Text = RBMProcessor.L2Regularization.ToString();
				minibatchSizeTextBox.Text = RBMProcessor.MinibatchSize.ToString();
				epochsTextBox.Text = RBMProcessor.Epochs.ToString();
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
			modelTypeComboBox.Items.Add(QuickBoltzmann.ModelType.RBM);
			modelTypeComboBox.Items.Add(QuickBoltzmann.ModelType.SRBM);
			// fill in visible type combobox
			visibleTypeComboBox.Items.Add(QuickBoltzmann.UnitType.Binary);
			visibleTypeComboBox.Items.Add(QuickBoltzmann.UnitType.Gaussian);

			// setup all our tooltips
			SetToolTip("Load training dataset", this.selectTrainingIdxButton);
			SetToolTip("Load validation dataset", this.selectValidationIdxButton);
			SetToolTip("Clear out the validation data", this.clearValidationDataButton);
			SetToolTip("Format of pixel data representation of training images (for visualization)", this.pixelFormatComboBox);
			SetToolTip("Width of training image in pixels (for visualization)", this.widthTextBox);
			SetToolTip("Height of training image in pixels (for visualization)", this.heightTextBox);
			SetToolTip("Model type to train", this.modelTypeComboBox);
			SetToolTip("Activation of function of visible units: Binary (Sigmoid) or Linear", this.visibleTypeComboBox);
			SetToolTip("Number of visible units in each training vector of loaded training data", this.visibleUnitsLabel);
			SetToolTip("Number of hidden units to use in trained model", this.hiddenUnitsTextBox);
			SetToolTip("Load saved parameters file", this.loadParametersButton);
			SetToolTip("Save parameters to disk", this.saveParametersButton);
			SetToolTip("Reset training parameters to default", this.resetParametersButton);
			SetToolTip("Speed of learning; reasonable values are around 0.001 (less for Gaussian visible units)", this.learningRateTextBox);
			SetToolTip("Smooths learning by having the previous weight update contribute to the current update by the given percent.  Valid values range from 0 to 1.", this.momentumTextBox);
			SetToolTip("L1 Regularization punishes all weights equally.  Many weights will be near zero with this method while others will grow large.", this.l1TextBox);
			SetToolTip("L2 Regularization more heavily punishes large weights, resulting in blurrier weights that can be harder to interpret", this.l2TextBox);
			SetToolTip("Number of training to vectors to show the model for each weight update", this.minibatchSizeTextBox);
			SetToolTip("The number of weight updates to perform", this.epochsTextBox);
			SetToolTip("Import RBM for further training", this.importButton);
			SetToolTip("Start Training", this.startButton);
			SetToolTip("Stop Training", this.stopButton);
			SetToolTip("Pause Training", this.pauseButton);
			SetToolTip("Save RBM to disk", this.exportButton);

			// add event to update epochs textbox
			RBMProcessor.EpochCompleted += new RBMProcessor.EpochCompletedHandler((uint i) => 
			{
				epochsTextBox.Invoke(new Action(() => 
				{
					epochsTextBox.Text = i.ToString();
				}));
			});
			RBMProcessor.TrainingCompleted += new RBMProcessor.TrainingCompletedHandler(() =>
			{
				this.Invoke(new Action(() =>
				{
					epochsTextBox.Text = "100";
					updateEpochs(epochsTextBox);

					CurrentState = ProgramState.TrainerPaused;
				}));
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
				// load IDX file
				_main_form.Cursor = Cursors.WaitCursor;

				if (RBMProcessor.SetTrainingData(ofd.FileName) == false)
				{
					_main_form.Cursor = Cursors.Default;
					return;
				}

				_main_form.Cursor = Cursors.Default;

				// update labels
				traingingIdxPathTextBox.Text = ofd.FileName;
				visibleUnitsLabel.Text = RBMProcessor.VisibleUnits.ToString();

				int total_vals = RBMProcessor.VisibleUnits;
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
				RBMProcessor.SetValidationData(null);
				validationIdxPathTextBox.Text = "";

				

				CurrentState = ProgramState.TrainerStopped;
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
				if (RBMProcessor.SetValidationData(ofd.FileName) == false)
				{
					Cursor.Current = Cursors.Default;
					return;
				}
				Cursor.Current = Cursors.Default;

				validationIdxPathTextBox.Text = ofd.FileName;

				_main_form.trainingLog.AddLog("Loaded Validation .IDX: {0}", ofd.FileName);

				validation_data_loaded = true;
				CurrentState = ProgramState.TrainerStopped;
			}
		}

		private void clearValidationDataButton_Click(object sender, EventArgs e)
		{
			RBMProcessor.SetValidationData(null);
			validationIdxPathTextBox.Text = "";

			_main_form.trainingLog.AddLog("Cleared Validation .IDX");

			validation_data_loaded = false;
			CurrentState = ProgramState.TrainerStopped;
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
			switch((QuickBoltzmann.ModelType)cb.SelectedItem)
			{
				case QuickBoltzmann.ModelType.RBM:
					break;
				case QuickBoltzmann.ModelType.SRBM:
					cb.SelectedItem = QuickBoltzmann.ModelType.RBM;
					MessageBox.Show("SRBM Training not yet implemented.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
					break;
			}
		}
		#endregion

		#region Visible Unit Select
		private void visibleTypeComboBox_SelectionChangeCommitted(object sender, EventArgs e)
		{
			ComboBox cb = sender as ComboBox;
			if (RBMProcessor.VisibleType != (QuickBoltzmann.UnitType)cb.SelectedItem)
			{
				RBMProcessor.VisibleType = (QuickBoltzmann.UnitType)cb.SelectedItem;
				_main_form.trainingLog.AddLog("Visible Type = {0}", RBMProcessor.VisibleType);
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
				if ((int)hidden != RBMProcessor.HiddenUnits)
				{
					RBMProcessor.HiddenUnits = (int)hidden;
					tb.Text = hidden.ToString();
					_main_form.trainingLog.AddLog("Hidden Units = {0}", RBMProcessor.HiddenUnits);
					return true;
				}
				return false;
			}
			tb.Text = RBMProcessor.HiddenUnits.ToString();
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
				if(learning_rate != RBMProcessor.LearningRate)
				{
					RBMProcessor.LearningRate = learning_rate;
					tb.Text = learning_rate.ToString();
					_main_form.trainingLog.AddLog("Learning Rate = {0}", RBMProcessor.LearningRate);
					return true;
				}
			}
			tb.Text = RBMProcessor.LearningRate.ToString();
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

				if (momentum != RBMProcessor.Momentum)
				{
					RBMProcessor.Momentum = momentum;
					tb.Text = momentum.ToString();
					_main_form.trainingLog.AddLog("Momentum = {0}", RBMProcessor.Momentum);
					return true;
				}
				return false;
			}
			tb.Text = RBMProcessor.Momentum.ToString();
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
				if (l1 != RBMProcessor.L1Regularization)
				{
					RBMProcessor.L1Regularization = l1;
					_main_form.trainingLog.AddLog("L1Regularization = {0}", RBMProcessor.L1Regularization);
					return true;
				}
				return false;
			}
			tb.Text = RBMProcessor.L1Regularization.ToString();
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
				if (l2 != RBMProcessor.L2Regularization)
				{
					RBMProcessor.L2Regularization = l2;
					_main_form.trainingLog.AddLog("L2Regularization = {0}", RBMProcessor.L2Regularization);
					return true;
				}
				return false;
			}
			tb.Text = RBMProcessor.L2Regularization.ToString();
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

		#region Set Minibatch
		private bool updateMinibatch(TextBox tb)
		{
			if (tb == null) return false;

			uint minibatch;
			if(uint.TryParse(tb.Text, out minibatch))
			{
				if ((int)minibatch != RBMProcessor.MinibatchSize)
				{
					RBMProcessor.MinibatchSize = (int)minibatch;
					tb.Text = minibatch.ToString();
					_main_form.trainingLog.AddLog("Minibatch Size = {0}", RBMProcessor.MinibatchSize);
					return true;
				}
				return false;
			}
			tb.Text = RBMProcessor.MinibatchSize.ToString();
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
				if ((uint)epochs != RBMProcessor.Epochs)
				{
					// user should not be able to set epochs to 0
					if (epochs == 0)
						epochs = 1;

					// set number of epochs to train for in the processor
					RBMProcessor.Epochs = (uint)epochs;

					tb.Text = epochs.ToString();
					// if we have data, then we are in the middle of training and start button should be enabled
					// even though training may have finished already
					if (RBMProcessor.HasTrainingData)
					{
						this.startButton.Enabled = true;
					}
					_main_form.trainingLog.AddLog("Epochs = {0}", RBMProcessor.Epochs);

					return true;
				}
				return false;
			}
			// set text based on value in rbmprocessor
			tb.Text = RBMProcessor.Epochs.ToString();
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
			if (RBMProcessor.HasTrainingData)
			{
				// start visualization
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
					RBMProcessor.Start(new Action(() =>
					{
						this.Invoke(new Action(() => _main_form.Cursor = Cursors.Default));
						this.CurrentState = ProgramState.TrainerRunning;
					}));
				}
				else if (CurrentState == ProgramState.TrainerPaused)
				{
					CurrentState = ProgramState.TrainerRunning;
					RBMProcessor.Start(null);	// no callback needed
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

			RBMProcessor.Stop();
		}

		private void pauseButton_Click(object sender, EventArgs e)
		{
			CurrentState = ProgramState.TrainerPaused;

			_main_form.visualizeVisible.Pause();
			_main_form.visualizeHidden.Pause();
			_main_form.visualizeWeights.Pause();

			_main_form.trainingLog.AddLog("Training Paused");

			RBMProcessor.Pause();
		}

		private void exportButton_Click(object sender, EventArgs e)
		{
			SaveFileDialog sfd = new SaveFileDialog();
			sfd.Filter = "RBM|*.rbm";

			if (sfd.ShowDialog() == DialogResult.OK)
			{
				RBMProcessor.SaveRBM(sfd.FileName);
				_main_form.trainingLog.AddLog("Exported RBM to {0}", sfd.FileName);
			}
		}

		private void importButton_Click(object sender, EventArgs e)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Filter = "RBM|*.rbm";

			if (ofd.ShowDialog() == DialogResult.OK)
			{
				uint hidden = 0;
				bool linear_units = false;

				RBMProcessor.LoadRBM(ofd.FileName, ref hidden, ref linear_units);
				_main_form.trainingLog.AddLog("Imported RBM from {0}", ofd.FileName);

				// set hidden unit count and visible unit type loaded
				hiddenUnitsTextBox.Text = hidden.ToString();
				visibleTypeComboBox.SelectedItem = linear_units == true ? QuickBoltzmann.UnitType.Gaussian : QuickBoltzmann.UnitType.Binary;


			}
		}

		#endregion

		#region Parameter Saving/Loading

		private void loadParametersButton_Click(object sender, EventArgs e)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Filter = "Visual RBM Training Parameters|*.vrbmparameters";
			if(ofd.ShowDialog() == DialogResult.OK)
			{
				try
				{
					Regex whitespace = new Regex("[ \t]+");
					Dictionary<String, String> parameter_map = new Dictionary<String, String>();
					uint line = 0;
					using (StreamReader sr = new StreamReader(new FileStream(ofd.FileName, FileMode.Open, FileAccess.Read)))
					{
						line++;

						String[] valid_keys = { "learning_rate", "momentum", "l1_regularization", "l2_regularization", "minibatch_size", "epochs" };

						for(String text = sr.ReadLine(); text != null; text = sr.ReadLine())
						{
							text = text.Trim();
							if (text != "")
							{
								String[] tokens = whitespace.Split(text);

								if(tokens.Length != 3 || tokens[1] != "=")
								{
									MessageBox.Show(String.Format("Problem parsing line {0}: {1}", line, text));
									return;
								}

								String key = tokens[0];
								String val = tokens[2];

								if(valid_keys.Contains(key) == false)
								{
									MessageBox.Show(String.Format("Given key \"{0}\" on line {1}, is not a valid parameter", key, line));
									return;
								}


								if (parameter_map.ContainsKey(key))
								{
									MessageBox.Show(String.Format("Parameter {0} appears multiple times", key));
									return;
								}

								parameter_map[key] = val;
							}
						}

						// parse out the parameters
						if (parameter_map.Count > 0)
						{

							String val;
							if (parameter_map.TryGetValue("learning_rate", out val))
							{
								float lr;
								if (float.TryParse(val, out lr))
								{
									RBMProcessor.LearningRate = lr;
									learningRateTextBox.Text = lr.ToString();
								}
								else
								{
									MessageBox.Show(String.Format("Could not parse \"learning_rate = {0}\"", val));
									return;
								}
							}

							if (parameter_map.TryGetValue("momentum", out val))
							{
								float mo;
								if (float.TryParse(val, out mo))
								{
									RBMProcessor.Momentum = mo;
									momentumTextBox.Text = mo.ToString();
								}
								else
								{
									MessageBox.Show(String.Format("Could not parse \"momentum = {0}\"", val));
									return;
								}
							}

							if (parameter_map.TryGetValue("minibatch_size", out val))
							{
								int ms;
								if (int.TryParse(val, out ms))
								{
									if (ms > 0)
									{
										RBMProcessor.MinibatchSize = ms;
										minibatchSizeTextBox.Text = ms.ToString();
									}
									else
									{
										MessageBox.Show("Minibatch Size must be greater than 0");
										return;
									}
								}
								else
								{
									MessageBox.Show(String.Format("Could not parse \"minibatch_size = {0}\"", val));
									return;
								}
							}

							if (parameter_map.TryGetValue("epochs", out val))
							{
								uint ep;
								if(uint.TryParse(val, out ep))
								{
									if(ep > 0)
									{
										RBMProcessor.Epochs = ep;
										epochsTextBox.Text = ep.ToString();
									}
									else
									{
										MessageBox.Show("Epochs must be greater than 0");
										return;
									}
								}
								else
								{
									MessageBox.Show(String.Format("Could not parse \"epochs = {0}\"", val));
									return;
								}
							}

							if(parameter_map. TryGetValue("l1_regularization", out val))
							{
								float l1;
								if(float.TryParse(val, out l1))
								{
									if(l1 >= 0.0f)
									{
										RBMProcessor.L1Regularization = l1;
										l1TextBox.Text = l1.ToString();
									}
									else
									{
										MessageBox.Show("L1 Regularization must be non-negative");
										return;
									}
								}
								else
								{
									MessageBox.Show(String.Format("Could not parse \"l1_regulrization = {0}\"", val));
									return;
								}
							}

							if(parameter_map. TryGetValue("l2_regularization", out val))
							{
								float l2;
								if(float.TryParse(val, out l2))
								{
									if(l2 >= 0.0f)
									{
										RBMProcessor.L2Regularization = l2;
										l2TextBox.Text = l2.ToString();
									}
									else
									{
										MessageBox.Show("L2 Regularization must be non-negative");
										return;
									}
								}
								else
								{
									MessageBox.Show(String.Format("Could not parse \"l2_regulrization = {0}\"", val));
									return;
								}
							}

							_main_form.trainingLog.AddLog("Loaded Parameters from {0}", ofd.FileName);
						}
					}
				}
				catch (System.Exception ex)
				{
					MessageBox.Show(String.Format("Problem Loading Parameters from \"{0}\"", ofd.FileName));
					return;
				}
			}
		}

		private void saveParametersButton_Click(object sender, EventArgs e)
		{
			SaveFileDialog sfd = new SaveFileDialog();
			sfd.Filter = "Visual RBM Training Parameters|*.vrbmparameters";
			if (sfd.ShowDialog() == DialogResult.OK)
			{
				// save some settings
				try
				{
					using (StreamWriter sw = new StreamWriter(new FileStream(sfd.FileName, FileMode.Create)))
					{
						sw.WriteLine("learning_rate = {0}", RBMProcessor.LearningRate);
						sw.WriteLine("momentum = {0}", RBMProcessor.Momentum);
						sw.WriteLine("l1_regularization = {0}", RBMProcessor.L1Regularization);
						sw.WriteLine("l2_regularization = {0}", RBMProcessor.L2Regularization);
						sw.WriteLine("minibatch_size = {0}", RBMProcessor.MinibatchSize);
						sw.WriteLine("epochs = {0}", RBMProcessor.Epochs);

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
			if (MessageBox.Show("Reset training parameters to defaults?", "", MessageBoxButtons.YesNo) == DialogResult.Yes)
			{
				RBMProcessor.LearningRate = 0.001f;
				RBMProcessor.Momentum = 0.5f;
				RBMProcessor.MinibatchSize = 10;
				RBMProcessor.Epochs = 100;
				RBMProcessor.L1Regularization = 0.0f;
				RBMProcessor.L2Regularization = 0.0001f;

				learningRateTextBox.Text = RBMProcessor.LearningRate.ToString();
				momentumTextBox.Text = RBMProcessor.Momentum.ToString();
				l1TextBox.Text = RBMProcessor.L1Regularization.ToString();
				l2TextBox.Text = RBMProcessor.L2Regularization.ToString();
				minibatchSizeTextBox.Text = RBMProcessor.MinibatchSize.ToString();
				epochsTextBox.Text = RBMProcessor.Epochs.ToString();
				_main_form.trainingLog.AddLog("Reset Parameters to Defaults");
			}
		}


		#endregion
	}
}
