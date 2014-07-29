namespace VisualRBM
{
	partial class SettingsBar
	{
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SettingsBar));
			this.panelModel = new System.Windows.Forms.Panel();
			this.panelModelRow3 = new System.Windows.Forms.Panel();
			this.hiddenUnitsTextBox = new System.Windows.Forms.TextBox();
			this.labelHiddenUnits = new System.Windows.Forms.Label();
			this.visibleUnitsLabel = new System.Windows.Forms.Label();
			this.labelVisibleUnits = new System.Windows.Forms.Label();
			this.panelModelRow2 = new System.Windows.Forms.Panel();
			this.hiddenTypeComboBox = new System.Windows.Forms.ComboBox();
			this.labelHiddenType = new System.Windows.Forms.Label();
			this.visibleTypeComboBox = new System.Windows.Forms.ComboBox();
			this.labelVisibleType = new System.Windows.Forms.Label();
			this.panelModelRow1 = new System.Windows.Forms.Panel();
			this.modelTypeComboBox = new System.Windows.Forms.ComboBox();
			this.labelModelType = new System.Windows.Forms.Label();
			this.labelModel = new System.Windows.Forms.Label();
			this.panelParameters = new System.Windows.Forms.Panel();
			this.panelParametersRow3 = new System.Windows.Forms.Panel();
			this.adadeltaDecayTextBox = new System.Windows.Forms.TextBox();
			this.labelAdadeltaDecay = new System.Windows.Forms.Label();
			this.epochsTextBox = new System.Windows.Forms.TextBox();
			this.labelEpochs = new System.Windows.Forms.Label();
			this.minibatchSizeTextBox = new System.Windows.Forms.TextBox();
			this.labelMinibatchSize = new System.Windows.Forms.Label();
			this.panelParametersRow2 = new System.Windows.Forms.Panel();
			this.hiddenDropoutTextBox = new System.Windows.Forms.TextBox();
			this.labelHiddenDropout = new System.Windows.Forms.Label();
			this.l2TextBox = new System.Windows.Forms.TextBox();
			this.labelL2 = new System.Windows.Forms.Label();
			this.momentumTextBox = new System.Windows.Forms.TextBox();
			this.labelMomentum = new System.Windows.Forms.Label();
			this.panelParametersRow1 = new System.Windows.Forms.Panel();
			this.visibleDropoutTextBox = new System.Windows.Forms.TextBox();
			this.labelVisibleDropout = new System.Windows.Forms.Label();
			this.l1TextBox = new System.Windows.Forms.TextBox();
			this.labelL1 = new System.Windows.Forms.Label();
			this.learningRateTextBox = new System.Windows.Forms.TextBox();
			this.labelLearningRate = new System.Windows.Forms.Label();
			this.labelParameters = new System.Windows.Forms.Label();
			this.loadScheduleButton = new System.Windows.Forms.Button();
			this.panelDataRow1 = new System.Windows.Forms.Panel();
			this.traingingIdxPathTextBox = new System.Windows.Forms.TextBox();
			this.selectTrainingIdxButton = new System.Windows.Forms.Button();
			this.labelInputData = new System.Windows.Forms.Label();
			this.labelData = new System.Windows.Forms.Label();
			this.labelPixelFormat = new System.Windows.Forms.Label();
			this.panelData = new System.Windows.Forms.Panel();
			this.panelDataRow3 = new System.Windows.Forms.Panel();
			this.heightTextBox = new System.Windows.Forms.TextBox();
			this.labelHeight = new System.Windows.Forms.Label();
			this.widthTextBox = new System.Windows.Forms.TextBox();
			this.labelWidth = new System.Windows.Forms.Label();
			this.pixelFormatComboBox = new System.Windows.Forms.ComboBox();
			this.panelDataRow2 = new System.Windows.Forms.Panel();
			this.validationIdxPathTextBox = new System.Windows.Forms.TextBox();
			this.clearValidationDataButton = new System.Windows.Forms.Button();
			this.selectValidationIdxButton = new System.Windows.Forms.Button();
			this.labelValidationData = new System.Windows.Forms.Label();
			this.startButton = new System.Windows.Forms.Button();
			this.stopButton = new System.Windows.Forms.Button();
			this.pauseButton = new System.Windows.Forms.Button();
			this.exportButton = new System.Windows.Forms.Button();
			this.importButton = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.seedTextBox = new System.Windows.Forms.TextBox();
			this.labelSeed = new System.Windows.Forms.Label();
			this.panelModel.SuspendLayout();
			this.panelModelRow3.SuspendLayout();
			this.panelModelRow2.SuspendLayout();
			this.panelModelRow1.SuspendLayout();
			this.panelParameters.SuspendLayout();
			this.panelParametersRow3.SuspendLayout();
			this.panelParametersRow2.SuspendLayout();
			this.panelParametersRow1.SuspendLayout();
			this.panelDataRow1.SuspendLayout();
			this.panelData.SuspendLayout();
			this.panelDataRow3.SuspendLayout();
			this.panelDataRow2.SuspendLayout();
			this.SuspendLayout();
			// 
			// panelModel
			// 
			this.panelModel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.panelModel.Controls.Add(this.panelModelRow3);
			this.panelModel.Controls.Add(this.panelModelRow2);
			this.panelModel.Controls.Add(this.panelModelRow1);
			this.panelModel.Controls.Add(this.labelModel);
			this.panelModel.Dock = System.Windows.Forms.DockStyle.Left;
			this.panelModel.Location = new System.Drawing.Point(313, 0);
			this.panelModel.Name = "panelModel";
			this.panelModel.Size = new System.Drawing.Size(271, 102);
			this.panelModel.TabIndex = 1;
			// 
			// panelModelRow3
			// 
			this.panelModelRow3.Controls.Add(this.hiddenUnitsTextBox);
			this.panelModelRow3.Controls.Add(this.labelHiddenUnits);
			this.panelModelRow3.Controls.Add(this.visibleUnitsLabel);
			this.panelModelRow3.Controls.Add(this.labelVisibleUnits);
			this.panelModelRow3.Dock = System.Windows.Forms.DockStyle.Top;
			this.panelModelRow3.Location = new System.Drawing.Point(0, 52);
			this.panelModelRow3.Name = "panelModelRow3";
			this.panelModelRow3.Padding = new System.Windows.Forms.Padding(3);
			this.panelModelRow3.Size = new System.Drawing.Size(269, 26);
			this.panelModelRow3.TabIndex = 2;
			// 
			// hiddenUnitsTextBox
			// 
			this.hiddenUnitsTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.hiddenUnitsTextBox.Location = new System.Drawing.Point(202, 3);
			this.hiddenUnitsTextBox.Name = "hiddenUnitsTextBox";
			this.hiddenUnitsTextBox.Size = new System.Drawing.Size(63, 20);
			this.hiddenUnitsTextBox.TabIndex = 4;
			this.hiddenUnitsTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.hiddenUnitsTextBox_KeyPress);
			this.hiddenUnitsTextBox.Leave += new System.EventHandler(this.hiddenUnitsTextBox_Leave);
			// 
			// labelHiddenUnits
			// 
			this.labelHiddenUnits.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelHiddenUnits.Location = new System.Drawing.Point(131, 3);
			this.labelHiddenUnits.Name = "labelHiddenUnits";
			this.labelHiddenUnits.Padding = new System.Windows.Forms.Padding(3, 0, 0, 0);
			this.labelHiddenUnits.Size = new System.Drawing.Size(71, 20);
			this.labelHiddenUnits.TabIndex = 0;
			this.labelHiddenUnits.Text = "Hidden Units";
			this.labelHiddenUnits.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// visibleUnitsLabel
			// 
			this.visibleUnitsLabel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.visibleUnitsLabel.Dock = System.Windows.Forms.DockStyle.Left;
			this.visibleUnitsLabel.Location = new System.Drawing.Point(68, 3);
			this.visibleUnitsLabel.Name = "visibleUnitsLabel";
			this.visibleUnitsLabel.Padding = new System.Windows.Forms.Padding(3, 0, 0, 0);
			this.visibleUnitsLabel.Size = new System.Drawing.Size(63, 20);
			this.visibleUnitsLabel.TabIndex = 3;
			this.visibleUnitsLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// labelVisibleUnits
			// 
			this.labelVisibleUnits.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelVisibleUnits.Location = new System.Drawing.Point(3, 3);
			this.labelVisibleUnits.Name = "labelVisibleUnits";
			this.labelVisibleUnits.Size = new System.Drawing.Size(65, 20);
			this.labelVisibleUnits.TabIndex = 0;
			this.labelVisibleUnits.Text = "Visible Units";
			this.labelVisibleUnits.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// panelModelRow2
			// 
			this.panelModelRow2.Controls.Add(this.hiddenTypeComboBox);
			this.panelModelRow2.Controls.Add(this.labelHiddenType);
			this.panelModelRow2.Controls.Add(this.visibleTypeComboBox);
			this.panelModelRow2.Controls.Add(this.labelVisibleType);
			this.panelModelRow2.Dock = System.Windows.Forms.DockStyle.Top;
			this.panelModelRow2.Location = new System.Drawing.Point(0, 26);
			this.panelModelRow2.Name = "panelModelRow2";
			this.panelModelRow2.Padding = new System.Windows.Forms.Padding(3);
			this.panelModelRow2.Size = new System.Drawing.Size(269, 26);
			this.panelModelRow2.TabIndex = 1;
			// 
			// hiddenTypeComboBox
			// 
			this.hiddenTypeComboBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.hiddenTypeComboBox.FormattingEnabled = true;
			this.hiddenTypeComboBox.Location = new System.Drawing.Point(201, 3);
			this.hiddenTypeComboBox.Name = "hiddenTypeComboBox";
			this.hiddenTypeComboBox.Size = new System.Drawing.Size(65, 21);
			this.hiddenTypeComboBox.TabIndex = 2;
			this.hiddenTypeComboBox.SelectionChangeCommitted += new System.EventHandler(this.hiddenTypeComboBox_SelectionChangeCommitted);
			// 
			// labelHiddenType
			// 
			this.labelHiddenType.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelHiddenType.Location = new System.Drawing.Point(133, 3);
			this.labelHiddenType.Name = "labelHiddenType";
			this.labelHiddenType.Size = new System.Drawing.Size(68, 20);
			this.labelHiddenType.TabIndex = 0;
			this.labelHiddenType.Text = "Hidden Type";
			this.labelHiddenType.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// visibleTypeComboBox
			// 
			this.visibleTypeComboBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.visibleTypeComboBox.FormattingEnabled = true;
			this.visibleTypeComboBox.Location = new System.Drawing.Point(68, 3);
			this.visibleTypeComboBox.Name = "visibleTypeComboBox";
			this.visibleTypeComboBox.Size = new System.Drawing.Size(65, 21);
			this.visibleTypeComboBox.TabIndex = 1;
			this.visibleTypeComboBox.SelectionChangeCommitted += new System.EventHandler(this.visibleTypeComboBox_SelectionChangeCommitted);
			// 
			// labelVisibleType
			// 
			this.labelVisibleType.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelVisibleType.Location = new System.Drawing.Point(3, 3);
			this.labelVisibleType.Name = "labelVisibleType";
			this.labelVisibleType.Size = new System.Drawing.Size(65, 20);
			this.labelVisibleType.TabIndex = 0;
			this.labelVisibleType.Text = "Visible Type";
			this.labelVisibleType.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// panelModelRow1
			// 
			this.panelModelRow1.Controls.Add(this.modelTypeComboBox);
			this.panelModelRow1.Controls.Add(this.labelModelType);
			this.panelModelRow1.Dock = System.Windows.Forms.DockStyle.Top;
			this.panelModelRow1.Location = new System.Drawing.Point(0, 0);
			this.panelModelRow1.Name = "panelModelRow1";
			this.panelModelRow1.Padding = new System.Windows.Forms.Padding(3);
			this.panelModelRow1.Size = new System.Drawing.Size(269, 26);
			this.panelModelRow1.TabIndex = 0;
			// 
			// modelTypeComboBox
			// 
			this.modelTypeComboBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.modelTypeComboBox.FormattingEnabled = true;
			this.modelTypeComboBox.Location = new System.Drawing.Point(68, 3);
			this.modelTypeComboBox.Name = "modelTypeComboBox";
			this.modelTypeComboBox.Size = new System.Drawing.Size(198, 21);
			this.modelTypeComboBox.TabIndex = 0;
			this.modelTypeComboBox.SelectionChangeCommitted += new System.EventHandler(this.modelTypeComboBox_SelectionChangeCommitted);
			// 
			// labelModelType
			// 
			this.labelModelType.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelModelType.Location = new System.Drawing.Point(3, 3);
			this.labelModelType.Name = "labelModelType";
			this.labelModelType.Size = new System.Drawing.Size(65, 20);
			this.labelModelType.TabIndex = 0;
			this.labelModelType.Text = "Model Type";
			this.labelModelType.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// labelModel
			// 
			this.labelModel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.labelModel.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.labelModel.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.labelModel.Location = new System.Drawing.Point(0, 78);
			this.labelModel.Name = "labelModel";
			this.labelModel.Size = new System.Drawing.Size(269, 22);
			this.labelModel.TabIndex = 0;
			this.labelModel.Text = "Model";
			this.labelModel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// panelParameters
			// 
			this.panelParameters.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.panelParameters.Controls.Add(this.panelParametersRow3);
			this.panelParameters.Controls.Add(this.panelParametersRow2);
			this.panelParameters.Controls.Add(this.panelParametersRow1);
			this.panelParameters.Controls.Add(this.labelParameters);
			this.panelParameters.Dock = System.Windows.Forms.DockStyle.Left;
			this.panelParameters.Location = new System.Drawing.Point(584, 0);
			this.panelParameters.Name = "panelParameters";
			this.panelParameters.Size = new System.Drawing.Size(410, 102);
			this.panelParameters.TabIndex = 2;
			// 
			// panelParametersRow3
			// 
			this.panelParametersRow3.Controls.Add(this.adadeltaDecayTextBox);
			this.panelParametersRow3.Controls.Add(this.labelAdadeltaDecay);
			this.panelParametersRow3.Controls.Add(this.epochsTextBox);
			this.panelParametersRow3.Controls.Add(this.labelEpochs);
			this.panelParametersRow3.Controls.Add(this.minibatchSizeTextBox);
			this.panelParametersRow3.Controls.Add(this.labelMinibatchSize);
			this.panelParametersRow3.Dock = System.Windows.Forms.DockStyle.Top;
			this.panelParametersRow3.Location = new System.Drawing.Point(0, 52);
			this.panelParametersRow3.Name = "panelParametersRow3";
			this.panelParametersRow3.Padding = new System.Windows.Forms.Padding(3);
			this.panelParametersRow3.Size = new System.Drawing.Size(408, 26);
			this.panelParametersRow3.TabIndex = 2;
			// 
			// adadeltaDecayTextBox
			// 
			this.adadeltaDecayTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.adadeltaDecayTextBox.Location = new System.Drawing.Point(285, 3);
			this.adadeltaDecayTextBox.Name = "adadeltaDecayTextBox";
			this.adadeltaDecayTextBox.Size = new System.Drawing.Size(30, 20);
			this.adadeltaDecayTextBox.TabIndex = 9;
			this.adadeltaDecayTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.adadeltaDecayTextBox_KeyPress);
			this.adadeltaDecayTextBox.Leave += new System.EventHandler(this.adadeltaDecayTextBox_Leave);
			// 
			// labelAdadeltaDecay
			// 
			this.labelAdadeltaDecay.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelAdadeltaDecay.Location = new System.Drawing.Point(202, 3);
			this.labelAdadeltaDecay.Name = "labelAdadeltaDecay";
			this.labelAdadeltaDecay.Size = new System.Drawing.Size(83, 20);
			this.labelAdadeltaDecay.TabIndex = 8;
			this.labelAdadeltaDecay.Text = "Adadelta Decay";
			this.labelAdadeltaDecay.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// epochsTextBox
			// 
			this.epochsTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.epochsTextBox.Location = new System.Drawing.Point(162, 3);
			this.epochsTextBox.Name = "epochsTextBox";
			this.epochsTextBox.Size = new System.Drawing.Size(40, 20);
			this.epochsTextBox.TabIndex = 7;
			this.epochsTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.epochsTextBox_KeyPress);
			this.epochsTextBox.Leave += new System.EventHandler(this.epochsTextBox_Leave);
			// 
			// labelEpochs
			// 
			this.labelEpochs.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelEpochs.Location = new System.Drawing.Point(119, 3);
			this.labelEpochs.Name = "labelEpochs";
			this.labelEpochs.Size = new System.Drawing.Size(43, 20);
			this.labelEpochs.TabIndex = 0;
			this.labelEpochs.Text = "Epochs";
			this.labelEpochs.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// minibatchSizeTextBox
			// 
			this.minibatchSizeTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.minibatchSizeTextBox.Location = new System.Drawing.Point(79, 3);
			this.minibatchSizeTextBox.Name = "minibatchSizeTextBox";
			this.minibatchSizeTextBox.Size = new System.Drawing.Size(40, 20);
			this.minibatchSizeTextBox.TabIndex = 6;
			this.minibatchSizeTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.minibatchSizeTextBox_KeyPress);
			this.minibatchSizeTextBox.Leave += new System.EventHandler(this.minibatchSizeTextBox_Leave);
			// 
			// labelMinibatchSize
			// 
			this.labelMinibatchSize.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelMinibatchSize.Location = new System.Drawing.Point(3, 3);
			this.labelMinibatchSize.Name = "labelMinibatchSize";
			this.labelMinibatchSize.Size = new System.Drawing.Size(76, 20);
			this.labelMinibatchSize.TabIndex = 0;
			this.labelMinibatchSize.Text = "Minibatch Size";
			this.labelMinibatchSize.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// panelParametersRow2
			// 
			this.panelParametersRow2.Controls.Add(this.hiddenDropoutTextBox);
			this.panelParametersRow2.Controls.Add(this.labelHiddenDropout);
			this.panelParametersRow2.Controls.Add(this.l2TextBox);
			this.panelParametersRow2.Controls.Add(this.labelL2);
			this.panelParametersRow2.Controls.Add(this.momentumTextBox);
			this.panelParametersRow2.Controls.Add(this.labelMomentum);
			this.panelParametersRow2.Dock = System.Windows.Forms.DockStyle.Top;
			this.panelParametersRow2.Location = new System.Drawing.Point(0, 26);
			this.panelParametersRow2.Name = "panelParametersRow2";
			this.panelParametersRow2.Padding = new System.Windows.Forms.Padding(3);
			this.panelParametersRow2.Size = new System.Drawing.Size(408, 26);
			this.panelParametersRow2.TabIndex = 1;
			// 
			// hiddenDropoutTextBox
			// 
			this.hiddenDropoutTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.hiddenDropoutTextBox.Location = new System.Drawing.Point(285, 3);
			this.hiddenDropoutTextBox.Name = "hiddenDropoutTextBox";
			this.hiddenDropoutTextBox.Size = new System.Drawing.Size(30, 20);
			this.hiddenDropoutTextBox.TabIndex = 5;
			this.hiddenDropoutTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.hiddenDropoutTextBox_KeyPress);
			this.hiddenDropoutTextBox.Leave += new System.EventHandler(this.hiddenDropoutTextBox_Leave);
			// 
			// labelHiddenDropout
			// 
			this.labelHiddenDropout.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelHiddenDropout.Location = new System.Drawing.Point(202, 3);
			this.labelHiddenDropout.Name = "labelHiddenDropout";
			this.labelHiddenDropout.Size = new System.Drawing.Size(83, 20);
			this.labelHiddenDropout.TabIndex = 0;
			this.labelHiddenDropout.Text = "Hidden Dropout";
			this.labelHiddenDropout.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// l2TextBox
			// 
			this.l2TextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.l2TextBox.Location = new System.Drawing.Point(152, 3);
			this.l2TextBox.Name = "l2TextBox";
			this.l2TextBox.Size = new System.Drawing.Size(50, 20);
			this.l2TextBox.TabIndex = 4;
			this.l2TextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.l2TextBox_KeyPress);
			this.l2TextBox.Leave += new System.EventHandler(this.l2TextBox_Leave);
			// 
			// labelL2
			// 
			this.labelL2.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelL2.Location = new System.Drawing.Point(129, 3);
			this.labelL2.Name = "labelL2";
			this.labelL2.Size = new System.Drawing.Size(23, 20);
			this.labelL2.TabIndex = 0;
			this.labelL2.Text = "L2";
			this.labelL2.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// momentumTextBox
			// 
			this.momentumTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.momentumTextBox.Location = new System.Drawing.Point(79, 3);
			this.momentumTextBox.Name = "momentumTextBox";
			this.momentumTextBox.Size = new System.Drawing.Size(50, 20);
			this.momentumTextBox.TabIndex = 3;
			this.momentumTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.momentumTextBox_KeyPress);
			this.momentumTextBox.Leave += new System.EventHandler(this.momentumTextBox_Leave);
			// 
			// labelMomentum
			// 
			this.labelMomentum.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelMomentum.Location = new System.Drawing.Point(3, 3);
			this.labelMomentum.Name = "labelMomentum";
			this.labelMomentum.Size = new System.Drawing.Size(76, 20);
			this.labelMomentum.TabIndex = 0;
			this.labelMomentum.Text = "Momentum";
			this.labelMomentum.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// panelParametersRow1
			// 
			this.panelParametersRow1.Controls.Add(this.seedTextBox);
			this.panelParametersRow1.Controls.Add(this.labelSeed);
			this.panelParametersRow1.Controls.Add(this.visibleDropoutTextBox);
			this.panelParametersRow1.Controls.Add(this.labelVisibleDropout);
			this.panelParametersRow1.Controls.Add(this.l1TextBox);
			this.panelParametersRow1.Controls.Add(this.labelL1);
			this.panelParametersRow1.Controls.Add(this.learningRateTextBox);
			this.panelParametersRow1.Controls.Add(this.labelLearningRate);
			this.panelParametersRow1.Dock = System.Windows.Forms.DockStyle.Top;
			this.panelParametersRow1.Location = new System.Drawing.Point(0, 0);
			this.panelParametersRow1.Name = "panelParametersRow1";
			this.panelParametersRow1.Padding = new System.Windows.Forms.Padding(3);
			this.panelParametersRow1.Size = new System.Drawing.Size(408, 26);
			this.panelParametersRow1.TabIndex = 0;
			// 
			// visibleDropoutTextBox
			// 
			this.visibleDropoutTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.visibleDropoutTextBox.Location = new System.Drawing.Point(285, 3);
			this.visibleDropoutTextBox.Name = "visibleDropoutTextBox";
			this.visibleDropoutTextBox.Size = new System.Drawing.Size(30, 20);
			this.visibleDropoutTextBox.TabIndex = 2;
			this.visibleDropoutTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.visibleDropoutTextBox_KeyPress);
			this.visibleDropoutTextBox.Leave += new System.EventHandler(this.visibleDropoutTextBox_Leave);
			// 
			// labelVisibleDropout
			// 
			this.labelVisibleDropout.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelVisibleDropout.Location = new System.Drawing.Point(202, 3);
			this.labelVisibleDropout.Name = "labelVisibleDropout";
			this.labelVisibleDropout.Size = new System.Drawing.Size(83, 20);
			this.labelVisibleDropout.TabIndex = 0;
			this.labelVisibleDropout.Text = "Visible Dropout";
			this.labelVisibleDropout.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// l1TextBox
			// 
			this.l1TextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.l1TextBox.Location = new System.Drawing.Point(152, 3);
			this.l1TextBox.Name = "l1TextBox";
			this.l1TextBox.Size = new System.Drawing.Size(50, 20);
			this.l1TextBox.TabIndex = 1;
			this.l1TextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.l1TextBox_KeyPress);
			this.l1TextBox.Leave += new System.EventHandler(this.l1TextBox_Leave);
			// 
			// labelL1
			// 
			this.labelL1.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelL1.Location = new System.Drawing.Point(129, 3);
			this.labelL1.Name = "labelL1";
			this.labelL1.Size = new System.Drawing.Size(23, 20);
			this.labelL1.TabIndex = 0;
			this.labelL1.Text = "L1";
			this.labelL1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// learningRateTextBox
			// 
			this.learningRateTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.learningRateTextBox.Location = new System.Drawing.Point(79, 3);
			this.learningRateTextBox.Name = "learningRateTextBox";
			this.learningRateTextBox.Size = new System.Drawing.Size(50, 20);
			this.learningRateTextBox.TabIndex = 0;
			this.learningRateTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.learningRateTextBox_KeyPress);
			this.learningRateTextBox.Leave += new System.EventHandler(this.learningRateTextBox_Leave);
			// 
			// labelLearningRate
			// 
			this.labelLearningRate.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelLearningRate.Location = new System.Drawing.Point(3, 3);
			this.labelLearningRate.Name = "labelLearningRate";
			this.labelLearningRate.Size = new System.Drawing.Size(76, 20);
			this.labelLearningRate.TabIndex = 0;
			this.labelLearningRate.Text = "Learning Rate";
			this.labelLearningRate.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// labelParameters
			// 
			this.labelParameters.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.labelParameters.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.labelParameters.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.labelParameters.Location = new System.Drawing.Point(0, 78);
			this.labelParameters.Name = "labelParameters";
			this.labelParameters.Size = new System.Drawing.Size(408, 22);
			this.labelParameters.TabIndex = 0;
			this.labelParameters.Text = "Parameters";
			this.labelParameters.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// loadScheduleButton
			// 
			this.loadScheduleButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
			this.loadScheduleButton.Dock = System.Windows.Forms.DockStyle.Left;
			this.loadScheduleButton.Enabled = false;
			this.loadScheduleButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
			this.loadScheduleButton.Image = ((System.Drawing.Image)(resources.GetObject("loadScheduleButton.Image")));
			this.loadScheduleButton.Location = new System.Drawing.Point(1054, 0);
			this.loadScheduleButton.Name = "loadScheduleButton";
			this.loadScheduleButton.Size = new System.Drawing.Size(60, 102);
			this.loadScheduleButton.TabIndex = 3;
			this.loadScheduleButton.Text = "Schedule";
			this.loadScheduleButton.TextAlign = System.Drawing.ContentAlignment.BottomCenter;
			this.loadScheduleButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageAboveText;
			this.loadScheduleButton.UseMnemonic = false;
			this.loadScheduleButton.UseVisualStyleBackColor = true;
			this.loadScheduleButton.Click += new System.EventHandler(this.loadParametersButton_Click);
			// 
			// panelDataRow1
			// 
			this.panelDataRow1.Controls.Add(this.traingingIdxPathTextBox);
			this.panelDataRow1.Controls.Add(this.selectTrainingIdxButton);
			this.panelDataRow1.Controls.Add(this.labelInputData);
			this.panelDataRow1.Dock = System.Windows.Forms.DockStyle.Top;
			this.panelDataRow1.Location = new System.Drawing.Point(0, 0);
			this.panelDataRow1.Name = "panelDataRow1";
			this.panelDataRow1.Padding = new System.Windows.Forms.Padding(3);
			this.panelDataRow1.Size = new System.Drawing.Size(311, 26);
			this.panelDataRow1.TabIndex = 0;
			// 
			// traingingIdxPathTextBox
			// 
			this.traingingIdxPathTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.traingingIdxPathTextBox.Location = new System.Drawing.Point(76, 3);
			this.traingingIdxPathTextBox.Name = "traingingIdxPathTextBox";
			this.traingingIdxPathTextBox.ReadOnly = true;
			this.traingingIdxPathTextBox.Size = new System.Drawing.Size(206, 20);
			this.traingingIdxPathTextBox.TabIndex = 0;
			// 
			// selectTrainingIdxButton
			// 
			this.selectTrainingIdxButton.AutoSize = true;
			this.selectTrainingIdxButton.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.selectTrainingIdxButton.Dock = System.Windows.Forms.DockStyle.Right;
			this.selectTrainingIdxButton.Location = new System.Drawing.Point(282, 3);
			this.selectTrainingIdxButton.Name = "selectTrainingIdxButton";
			this.selectTrainingIdxButton.Size = new System.Drawing.Size(26, 20);
			this.selectTrainingIdxButton.TabIndex = 1;
			this.selectTrainingIdxButton.Text = "...";
			this.selectTrainingIdxButton.UseVisualStyleBackColor = true;
			this.selectTrainingIdxButton.Click += new System.EventHandler(this.selectIdxButton_Click);
			// 
			// labelInputData
			// 
			this.labelInputData.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelInputData.Location = new System.Drawing.Point(3, 3);
			this.labelInputData.Name = "labelInputData";
			this.labelInputData.Size = new System.Drawing.Size(73, 20);
			this.labelInputData.TabIndex = 0;
			this.labelInputData.Text = "Training Set";
			this.labelInputData.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// labelData
			// 
			this.labelData.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.labelData.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.labelData.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.labelData.Location = new System.Drawing.Point(0, 78);
			this.labelData.Name = "labelData";
			this.labelData.Size = new System.Drawing.Size(311, 22);
			this.labelData.TabIndex = 0;
			this.labelData.Text = "Data";
			this.labelData.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// labelPixelFormat
			// 
			this.labelPixelFormat.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelPixelFormat.Location = new System.Drawing.Point(3, 3);
			this.labelPixelFormat.Name = "labelPixelFormat";
			this.labelPixelFormat.Size = new System.Drawing.Size(73, 20);
			this.labelPixelFormat.TabIndex = 0;
			this.labelPixelFormat.Text = "Pixel Format";
			this.labelPixelFormat.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// panelData
			// 
			this.panelData.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.panelData.Controls.Add(this.panelDataRow3);
			this.panelData.Controls.Add(this.panelDataRow2);
			this.panelData.Controls.Add(this.labelData);
			this.panelData.Controls.Add(this.panelDataRow1);
			this.panelData.Dock = System.Windows.Forms.DockStyle.Left;
			this.panelData.Location = new System.Drawing.Point(0, 0);
			this.panelData.Name = "panelData";
			this.panelData.Size = new System.Drawing.Size(313, 102);
			this.panelData.TabIndex = 0;
			// 
			// panelDataRow3
			// 
			this.panelDataRow3.Controls.Add(this.heightTextBox);
			this.panelDataRow3.Controls.Add(this.labelHeight);
			this.panelDataRow3.Controls.Add(this.widthTextBox);
			this.panelDataRow3.Controls.Add(this.labelWidth);
			this.panelDataRow3.Controls.Add(this.pixelFormatComboBox);
			this.panelDataRow3.Controls.Add(this.labelPixelFormat);
			this.panelDataRow3.Dock = System.Windows.Forms.DockStyle.Top;
			this.panelDataRow3.Location = new System.Drawing.Point(0, 52);
			this.panelDataRow3.Name = "panelDataRow3";
			this.panelDataRow3.Padding = new System.Windows.Forms.Padding(3);
			this.panelDataRow3.Size = new System.Drawing.Size(311, 26);
			this.panelDataRow3.TabIndex = 2;
			// 
			// heightTextBox
			// 
			this.heightTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.heightTextBox.Location = new System.Drawing.Point(266, 3);
			this.heightTextBox.Name = "heightTextBox";
			this.heightTextBox.Size = new System.Drawing.Size(41, 20);
			this.heightTextBox.TabIndex = 7;
			this.heightTextBox.TextChanged += new System.EventHandler(this.heightTextBox_Leave);
			this.heightTextBox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.heightTextBox_KeyDown);
			// 
			// labelHeight
			// 
			this.labelHeight.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelHeight.Location = new System.Drawing.Point(228, 3);
			this.labelHeight.Name = "labelHeight";
			this.labelHeight.Size = new System.Drawing.Size(38, 20);
			this.labelHeight.TabIndex = 0;
			this.labelHeight.Text = "Height";
			this.labelHeight.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// widthTextBox
			// 
			this.widthTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.widthTextBox.Location = new System.Drawing.Point(187, 3);
			this.widthTextBox.Name = "widthTextBox";
			this.widthTextBox.Size = new System.Drawing.Size(41, 20);
			this.widthTextBox.TabIndex = 6;
			this.widthTextBox.TextChanged += new System.EventHandler(this.widthTextBox_Leave);
			this.widthTextBox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.widthTextBox_KeyDown);
			// 
			// labelWidth
			// 
			this.labelWidth.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelWidth.Location = new System.Drawing.Point(152, 3);
			this.labelWidth.Name = "labelWidth";
			this.labelWidth.Size = new System.Drawing.Size(35, 20);
			this.labelWidth.TabIndex = 0;
			this.labelWidth.Text = "Width";
			this.labelWidth.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// pixelFormatComboBox
			// 
			this.pixelFormatComboBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.pixelFormatComboBox.FormattingEnabled = true;
			this.pixelFormatComboBox.Items.AddRange(new object[] {
            VisualRBM.PixelFormat.Lightness,
            VisualRBM.PixelFormat.RGB});
			this.pixelFormatComboBox.Location = new System.Drawing.Point(76, 3);
			this.pixelFormatComboBox.Name = "pixelFormatComboBox";
			this.pixelFormatComboBox.Size = new System.Drawing.Size(76, 21);
			this.pixelFormatComboBox.TabIndex = 5;
			this.pixelFormatComboBox.SelectionChangeCommitted += new System.EventHandler(this.pixelFormatComboBox_SelectionChangeCommitted);
			// 
			// panelDataRow2
			// 
			this.panelDataRow2.Controls.Add(this.validationIdxPathTextBox);
			this.panelDataRow2.Controls.Add(this.clearValidationDataButton);
			this.panelDataRow2.Controls.Add(this.selectValidationIdxButton);
			this.panelDataRow2.Controls.Add(this.labelValidationData);
			this.panelDataRow2.Dock = System.Windows.Forms.DockStyle.Top;
			this.panelDataRow2.Location = new System.Drawing.Point(0, 26);
			this.panelDataRow2.Name = "panelDataRow2";
			this.panelDataRow2.Padding = new System.Windows.Forms.Padding(3);
			this.panelDataRow2.Size = new System.Drawing.Size(311, 26);
			this.panelDataRow2.TabIndex = 1;
			// 
			// validationIdxPathTextBox
			// 
			this.validationIdxPathTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.validationIdxPathTextBox.Location = new System.Drawing.Point(76, 3);
			this.validationIdxPathTextBox.Name = "validationIdxPathTextBox";
			this.validationIdxPathTextBox.ReadOnly = true;
			this.validationIdxPathTextBox.Size = new System.Drawing.Size(188, 20);
			this.validationIdxPathTextBox.TabIndex = 2;
			// 
			// clearValidationDataButton
			// 
			this.clearValidationDataButton.AutoSize = true;
			this.clearValidationDataButton.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.clearValidationDataButton.Dock = System.Windows.Forms.DockStyle.Right;
			this.clearValidationDataButton.Enabled = false;
			this.clearValidationDataButton.Image = ((System.Drawing.Image)(resources.GetObject("clearValidationDataButton.Image")));
			this.clearValidationDataButton.Location = new System.Drawing.Point(264, 3);
			this.clearValidationDataButton.Name = "clearValidationDataButton";
			this.clearValidationDataButton.Size = new System.Drawing.Size(18, 20);
			this.clearValidationDataButton.TabIndex = 3;
			this.clearValidationDataButton.UseVisualStyleBackColor = true;
			this.clearValidationDataButton.Click += new System.EventHandler(this.clearValidationDataButton_Click);
			// 
			// selectValidationIdxButton
			// 
			this.selectValidationIdxButton.AutoSize = true;
			this.selectValidationIdxButton.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.selectValidationIdxButton.Dock = System.Windows.Forms.DockStyle.Right;
			this.selectValidationIdxButton.Enabled = false;
			this.selectValidationIdxButton.Location = new System.Drawing.Point(282, 3);
			this.selectValidationIdxButton.Name = "selectValidationIdxButton";
			this.selectValidationIdxButton.Size = new System.Drawing.Size(26, 20);
			this.selectValidationIdxButton.TabIndex = 4;
			this.selectValidationIdxButton.Text = "...";
			this.selectValidationIdxButton.UseVisualStyleBackColor = true;
			this.selectValidationIdxButton.Click += new System.EventHandler(this.selectValidationIdxButton_Click);
			// 
			// labelValidationData
			// 
			this.labelValidationData.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelValidationData.Location = new System.Drawing.Point(3, 3);
			this.labelValidationData.Name = "labelValidationData";
			this.labelValidationData.Size = new System.Drawing.Size(73, 20);
			this.labelValidationData.TabIndex = 0;
			this.labelValidationData.Text = "Validation Set";
			this.labelValidationData.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// startButton
			// 
			this.startButton.Dock = System.Windows.Forms.DockStyle.Left;
			this.startButton.Enabled = false;
			this.startButton.Image = ((System.Drawing.Image)(resources.GetObject("startButton.Image")));
			this.startButton.Location = new System.Drawing.Point(1114, 0);
			this.startButton.Name = "startButton";
			this.startButton.Size = new System.Drawing.Size(60, 102);
			this.startButton.TabIndex = 5;
			this.startButton.Text = "Start";
			this.startButton.TextAlign = System.Drawing.ContentAlignment.BottomCenter;
			this.startButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageAboveText;
			this.startButton.UseVisualStyleBackColor = true;
			this.startButton.Click += new System.EventHandler(this.startButton_Click);
			// 
			// stopButton
			// 
			this.stopButton.Dock = System.Windows.Forms.DockStyle.Left;
			this.stopButton.Enabled = false;
			this.stopButton.Image = ((System.Drawing.Image)(resources.GetObject("stopButton.Image")));
			this.stopButton.Location = new System.Drawing.Point(1234, 0);
			this.stopButton.Name = "stopButton";
			this.stopButton.Size = new System.Drawing.Size(60, 102);
			this.stopButton.TabIndex = 7;
			this.stopButton.Text = "Stop";
			this.stopButton.TextAlign = System.Drawing.ContentAlignment.BottomCenter;
			this.stopButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageAboveText;
			this.stopButton.UseVisualStyleBackColor = true;
			this.stopButton.Click += new System.EventHandler(this.stopButton_Click);
			// 
			// pauseButton
			// 
			this.pauseButton.Dock = System.Windows.Forms.DockStyle.Left;
			this.pauseButton.Enabled = false;
			this.pauseButton.Image = ((System.Drawing.Image)(resources.GetObject("pauseButton.Image")));
			this.pauseButton.Location = new System.Drawing.Point(1174, 0);
			this.pauseButton.Name = "pauseButton";
			this.pauseButton.Size = new System.Drawing.Size(60, 102);
			this.pauseButton.TabIndex = 6;
			this.pauseButton.Text = "Pause";
			this.pauseButton.TextAlign = System.Drawing.ContentAlignment.BottomCenter;
			this.pauseButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageAboveText;
			this.pauseButton.UseVisualStyleBackColor = true;
			this.pauseButton.Click += new System.EventHandler(this.pauseButton_Click);
			// 
			// exportButton
			// 
			this.exportButton.Dock = System.Windows.Forms.DockStyle.Left;
			this.exportButton.Enabled = false;
			this.exportButton.Image = ((System.Drawing.Image)(resources.GetObject("exportButton.Image")));
			this.exportButton.Location = new System.Drawing.Point(1294, 0);
			this.exportButton.Name = "exportButton";
			this.exportButton.Size = new System.Drawing.Size(60, 102);
			this.exportButton.TabIndex = 8;
			this.exportButton.Text = "Export";
			this.exportButton.TextAlign = System.Drawing.ContentAlignment.BottomCenter;
			this.exportButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageAboveText;
			this.exportButton.UseVisualStyleBackColor = true;
			this.exportButton.Click += new System.EventHandler(this.exportButton_Click);
			// 
			// importButton
			// 
			this.importButton.Dock = System.Windows.Forms.DockStyle.Left;
			this.importButton.Enabled = false;
			this.importButton.Image = ((System.Drawing.Image)(resources.GetObject("importButton.Image")));
			this.importButton.Location = new System.Drawing.Point(994, 0);
			this.importButton.Name = "importButton";
			this.importButton.Size = new System.Drawing.Size(60, 102);
			this.importButton.TabIndex = 4;
			this.importButton.Text = "Import";
			this.importButton.TextAlign = System.Drawing.ContentAlignment.BottomCenter;
			this.importButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageAboveText;
			this.importButton.UseVisualStyleBackColor = true;
			this.importButton.Click += new System.EventHandler(this.importButton_Click);
			// 
			// label1
			// 
			this.label1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.label1.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label1.Location = new System.Drawing.Point(0, 78);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(62, 22);
			this.label1.TabIndex = 0;
			this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// SeedTextBox
			// 
			this.seedTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.seedTextBox.Location = new System.Drawing.Point(347, 3);
			this.seedTextBox.Name = "SeedTextBox";
			this.seedTextBox.Size = new System.Drawing.Size(59, 20);
			this.seedTextBox.TabIndex = 4;
			this.seedTextBox.Leave += new System.EventHandler(this.seedTextBox_Leave);
			this.seedTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.seedTextBox_KeyPress);
			// 
			// labelSeed
			// 
			this.labelSeed.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelSeed.Location = new System.Drawing.Point(315, 3);
			this.labelSeed.Name = "labelSeed";
			this.labelSeed.Size = new System.Drawing.Size(32, 20);
			this.labelSeed.TabIndex = 3;
			this.labelSeed.Text = "Seed";
			this.labelSeed.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// SettingsBar
			// 
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
			this.Controls.Add(this.exportButton);
			this.Controls.Add(this.stopButton);
			this.Controls.Add(this.pauseButton);
			this.Controls.Add(this.startButton);
			this.Controls.Add(this.loadScheduleButton);
			this.Controls.Add(this.importButton);
			this.Controls.Add(this.panelParameters);
			this.Controls.Add(this.panelModel);
			this.Controls.Add(this.panelData);
			this.Margin = new System.Windows.Forms.Padding(0);
			this.MinimumSize = new System.Drawing.Size(1354, 102);
			this.Name = "SettingsBar";
			this.Size = new System.Drawing.Size(1354, 102);
			this.panelModel.ResumeLayout(false);
			this.panelModelRow3.ResumeLayout(false);
			this.panelModelRow3.PerformLayout();
			this.panelModelRow2.ResumeLayout(false);
			this.panelModelRow1.ResumeLayout(false);
			this.panelParameters.ResumeLayout(false);
			this.panelParametersRow3.ResumeLayout(false);
			this.panelParametersRow3.PerformLayout();
			this.panelParametersRow2.ResumeLayout(false);
			this.panelParametersRow2.PerformLayout();
			this.panelParametersRow1.ResumeLayout(false);
			this.panelParametersRow1.PerformLayout();
			this.panelDataRow1.ResumeLayout(false);
			this.panelDataRow1.PerformLayout();
			this.panelData.ResumeLayout(false);
			this.panelDataRow3.ResumeLayout(false);
			this.panelDataRow3.PerformLayout();
			this.panelDataRow2.ResumeLayout(false);
			this.panelDataRow2.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Panel panelModel;
		private System.Windows.Forms.Panel panelModelRow1;
		private System.Windows.Forms.Label labelModel;
		private System.Windows.Forms.ComboBox modelTypeComboBox;
		private System.Windows.Forms.Label labelModelType;
		private System.Windows.Forms.Panel panelParameters;
		private System.Windows.Forms.Label labelParameters;
		private System.Windows.Forms.Panel panelModelRow3;
		private System.Windows.Forms.Label labelVisibleUnits;
		private System.Windows.Forms.Panel panelModelRow2;
		private System.Windows.Forms.ComboBox visibleTypeComboBox;
		private System.Windows.Forms.Label labelVisibleType;
		private System.Windows.Forms.ComboBox hiddenTypeComboBox;
		private System.Windows.Forms.Label labelHiddenType;
		private System.Windows.Forms.TextBox hiddenUnitsTextBox;
		private System.Windows.Forms.Label labelHiddenUnits;
		private System.Windows.Forms.Label visibleUnitsLabel;
		private System.Windows.Forms.Panel panelDataRow1;
		private System.Windows.Forms.TextBox traingingIdxPathTextBox;
		private System.Windows.Forms.Button selectTrainingIdxButton;
		private System.Windows.Forms.Label labelInputData;
		private System.Windows.Forms.Label labelData;
		protected System.Windows.Forms.Label labelPixelFormat;
		private System.Windows.Forms.Panel panelData;
		private System.Windows.Forms.Panel panelDataRow2;
		private System.Windows.Forms.Label labelValidationData;
		private System.Windows.Forms.Button selectValidationIdxButton;
		private System.Windows.Forms.TextBox validationIdxPathTextBox;
		private System.Windows.Forms.Button clearValidationDataButton;
		private System.Windows.Forms.Panel panelDataRow3;
		protected System.Windows.Forms.TextBox heightTextBox;
		protected System.Windows.Forms.Label labelHeight;
		protected System.Windows.Forms.TextBox widthTextBox;
		protected System.Windows.Forms.Label labelWidth;
		protected System.Windows.Forms.ComboBox pixelFormatComboBox;
		private System.Windows.Forms.Button startButton;
		private System.Windows.Forms.Button stopButton;
		private System.Windows.Forms.Button pauseButton;
		private System.Windows.Forms.Button exportButton;
		private System.Windows.Forms.Button importButton;
		private System.Windows.Forms.Button loadScheduleButton;
		private System.Windows.Forms.Panel panelParametersRow3;
		private System.Windows.Forms.Panel panelParametersRow2;
		private System.Windows.Forms.Panel panelParametersRow1;
		private System.Windows.Forms.TextBox learningRateTextBox;
		private System.Windows.Forms.Label labelLearningRate;
		private System.Windows.Forms.TextBox minibatchSizeTextBox;
		private System.Windows.Forms.Label labelMinibatchSize;
		private System.Windows.Forms.TextBox momentumTextBox;
		private System.Windows.Forms.Label labelMomentum;
		private System.Windows.Forms.TextBox epochsTextBox;
		private System.Windows.Forms.Label labelEpochs;
		private System.Windows.Forms.TextBox l2TextBox;
		private System.Windows.Forms.Label labelL2;
		private System.Windows.Forms.TextBox l1TextBox;
		private System.Windows.Forms.Label labelL1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox hiddenDropoutTextBox;
		private System.Windows.Forms.Label labelHiddenDropout;
		private System.Windows.Forms.TextBox visibleDropoutTextBox;
		private System.Windows.Forms.Label labelVisibleDropout;
		private System.Windows.Forms.TextBox adadeltaDecayTextBox;
		private System.Windows.Forms.Label labelAdadeltaDecay;
		private System.Windows.Forms.TextBox seedTextBox;
		private System.Windows.Forms.Label labelSeed;

	}
}
