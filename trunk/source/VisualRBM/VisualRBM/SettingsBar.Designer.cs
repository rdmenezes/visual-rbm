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
			this.modelPanel = new System.Windows.Forms.Panel();
			this.panel7 = new System.Windows.Forms.Panel();
			this.modelTypeComboBox = new System.Windows.Forms.ComboBox();
			this.labelModelType = new System.Windows.Forms.Label();
			this.panel6 = new System.Windows.Forms.Panel();
			this.visibleTypeComboBox = new System.Windows.Forms.ComboBox();
			this.labelVisibleType = new System.Windows.Forms.Label();
			this.hiddenTypeComboBox = new System.Windows.Forms.ComboBox();
			this.labelHiddenType = new System.Windows.Forms.Label();
			this.panel14 = new System.Windows.Forms.Panel();
			this.hiddenUnitsTextBox = new System.Windows.Forms.TextBox();
			this.labelHiddenUnits = new System.Windows.Forms.Label();
			this.visibleUnitsLabel = new System.Windows.Forms.Label();
			this.labelVisibleUnits = new System.Windows.Forms.Label();
			this.label10 = new System.Windows.Forms.Label();
			this.parametersPanel = new System.Windows.Forms.Panel();
			this.panel5 = new System.Windows.Forms.Panel();
			this.epochsTextBox = new System.Windows.Forms.TextBox();
			this.labelEpochs = new System.Windows.Forms.Label();
			this.minibatchSizeTextBox = new System.Windows.Forms.TextBox();
			this.labelMinibatchSize = new System.Windows.Forms.Label();
			this.panel15 = new System.Windows.Forms.Panel();
			this.l2TextBox = new System.Windows.Forms.TextBox();
			this.labelL2 = new System.Windows.Forms.Label();
			this.momentumTextBox = new System.Windows.Forms.TextBox();
			this.labelMomentum = new System.Windows.Forms.Label();
			this.panel3 = new System.Windows.Forms.Panel();
			this.l1TextBox = new System.Windows.Forms.TextBox();
			this.labelL1 = new System.Windows.Forms.Label();
			this.learningRateTextBox = new System.Windows.Forms.TextBox();
			this.labelLearningRate = new System.Windows.Forms.Label();
			this.label14 = new System.Windows.Forms.Label();
			this.resetParametersButton = new System.Windows.Forms.Button();
			this.saveParametersButton = new System.Windows.Forms.Button();
			this.loadParametersButton = new System.Windows.Forms.Button();
			this.panel1 = new System.Windows.Forms.Panel();
			this.traingingIdxPathTextBox = new System.Windows.Forms.TextBox();
			this.selectTrainingIdxButton = new System.Windows.Forms.Button();
			this.labelInputData = new System.Windows.Forms.Label();
			this.label9 = new System.Windows.Forms.Label();
			this.labelPixelFormat = new System.Windows.Forms.Label();
			this.dataPanel = new System.Windows.Forms.Panel();
			this.panel8 = new System.Windows.Forms.Panel();
			this.heightTextBox = new System.Windows.Forms.TextBox();
			this.labelHeight = new System.Windows.Forms.Label();
			this.widthTextBox = new System.Windows.Forms.TextBox();
			this.labelWidth = new System.Windows.Forms.Label();
			this.pixelFormatComboBox = new System.Windows.Forms.ComboBox();
			this.panel2 = new System.Windows.Forms.Panel();
			this.validationIdxPathTextBox = new System.Windows.Forms.TextBox();
			this.clearValidationDataButton = new System.Windows.Forms.Button();
			this.selectValidationIdxButton = new System.Windows.Forms.Button();
			this.labelValidationData = new System.Windows.Forms.Label();
			this.startButton = new System.Windows.Forms.Button();
			this.stopButton = new System.Windows.Forms.Button();
			this.pauseButton = new System.Windows.Forms.Button();
			this.exportButton = new System.Windows.Forms.Button();
			this.importButton = new System.Windows.Forms.Button();
			this.panel4 = new System.Windows.Forms.Panel();
			this.label1 = new System.Windows.Forms.Label();
			this.labelVisibleDropout = new System.Windows.Forms.Label();
			this.labelHiddenDropout = new System.Windows.Forms.Label();
			this.visibleDropoutTextBox = new System.Windows.Forms.TextBox();
			this.hiddenDropoutTextBox = new System.Windows.Forms.TextBox();
			this.modelPanel.SuspendLayout();
			this.panel7.SuspendLayout();
			this.panel6.SuspendLayout();
			this.panel14.SuspendLayout();
			this.parametersPanel.SuspendLayout();
			this.panel5.SuspendLayout();
			this.panel15.SuspendLayout();
			this.panel3.SuspendLayout();
			this.panel1.SuspendLayout();
			this.dataPanel.SuspendLayout();
			this.panel8.SuspendLayout();
			this.panel2.SuspendLayout();
			this.panel4.SuspendLayout();
			this.SuspendLayout();
			// 
			// modelPanel
			// 
			this.modelPanel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.modelPanel.Controls.Add(this.panel14);
			this.modelPanel.Controls.Add(this.panel6);
			this.modelPanel.Controls.Add(this.panel7);
			this.modelPanel.Controls.Add(this.label10);
			this.modelPanel.Dock = System.Windows.Forms.DockStyle.Left;
			this.modelPanel.Location = new System.Drawing.Point(313, 0);
			this.modelPanel.Name = "modelPanel";
			this.modelPanel.Size = new System.Drawing.Size(271, 102);
			this.modelPanel.TabIndex = 25;
			// 
			// panel7
			// 
			this.panel7.Controls.Add(this.modelTypeComboBox);
			this.panel7.Controls.Add(this.labelModelType);
			this.panel7.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel7.Location = new System.Drawing.Point(0, 0);
			this.panel7.Name = "panel7";
			this.panel7.Padding = new System.Windows.Forms.Padding(3);
			this.panel7.Size = new System.Drawing.Size(233, 26);
			this.panel7.TabIndex = 20;
			// 
			// modelTypeComboBox
			// 
			this.modelTypeComboBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.modelTypeComboBox.FormattingEnabled = true;
			this.modelTypeComboBox.Location = new System.Drawing.Point(68, 3);
			this.modelTypeComboBox.Name = "modelTypeComboBox";
			this.modelTypeComboBox.Size = new System.Drawing.Size(198, 21);
			this.modelTypeComboBox.TabIndex = 7;
			this.modelTypeComboBox.SelectionChangeCommitted += new System.EventHandler(this.modelTypeComboBox_SelectionChangeCommitted);
			// 
			// labelModelType
			// 
			this.labelModelType.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelModelType.Location = new System.Drawing.Point(3, 3);
			this.labelModelType.Name = "labelModelType";
			this.labelModelType.Size = new System.Drawing.Size(65, 20);
			this.labelModelType.TabIndex = 6;
			this.labelModelType.Text = "Model Type";
			this.labelModelType.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// panel6
			// 
			this.panel6.Controls.Add(this.hiddenTypeComboBox);
			this.panel6.Controls.Add(this.labelHiddenType);
			this.panel6.Controls.Add(this.visibleTypeComboBox);
			this.panel6.Controls.Add(this.labelVisibleType);
			this.panel6.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel6.Location = new System.Drawing.Point(0, 26);
			this.panel6.Name = "panel6";
			this.panel6.Padding = new System.Windows.Forms.Padding(3);
			this.panel6.Size = new System.Drawing.Size(233, 26);
			this.panel6.TabIndex = 29;
			// 
			// visibleTypeComboBox
			// 
			this.visibleTypeComboBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.visibleTypeComboBox.FormattingEnabled = true;
			this.visibleTypeComboBox.Location = new System.Drawing.Point(68, 3);
			this.visibleTypeComboBox.Name = "visibleTypeComboBox";
			this.visibleTypeComboBox.Size = new System.Drawing.Size(65, 21);
			this.visibleTypeComboBox.TabIndex = 5;
			this.visibleTypeComboBox.SelectionChangeCommitted += new System.EventHandler(this.visibleTypeComboBox_SelectionChangeCommitted);
			// 
			// labelVisibleType
			// 
			this.labelVisibleType.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelVisibleType.Location = new System.Drawing.Point(3, 3);
			this.labelVisibleType.Name = "labelVisibleType";
			this.labelVisibleType.Size = new System.Drawing.Size(65, 20);
			this.labelVisibleType.TabIndex = 4;
			this.labelVisibleType.Text = "Visible Type";
			this.labelVisibleType.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// hiddenTypeComboBox
			// 
			this.hiddenTypeComboBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.hiddenTypeComboBox.FormattingEnabled = true;
			this.hiddenTypeComboBox.Location = new System.Drawing.Point(68, 3);
			this.hiddenTypeComboBox.Name = "hiddenTypeComboBox";
			this.hiddenTypeComboBox.Size = new System.Drawing.Size(65, 21);
			this.hiddenTypeComboBox.TabIndex = 5;
			this.hiddenTypeComboBox.SelectionChangeCommitted += new System.EventHandler(this.hiddenTypeComboBox_SelectionChangeCommitted);
			// 
			// labelHiddenType
			// 
			this.labelHiddenType.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelHiddenType.Location = new System.Drawing.Point(3, 3);
			this.labelHiddenType.Name = "labelHiddenType";
			this.labelHiddenType.Size = new System.Drawing.Size(68, 20);
			this.labelHiddenType.TabIndex = 4;
			this.labelHiddenType.Text = "Hidden Type";
			this.labelHiddenType.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// panel14
			// 
			this.panel14.Controls.Add(this.hiddenUnitsTextBox);
			this.panel14.Controls.Add(this.labelHiddenUnits);
			this.panel14.Controls.Add(this.visibleUnitsLabel);
			this.panel14.Controls.Add(this.labelVisibleUnits);
			this.panel14.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel14.Location = new System.Drawing.Point(0, 52);
			this.panel14.Name = "panel14";
			this.panel14.Padding = new System.Windows.Forms.Padding(3);
			this.panel14.Size = new System.Drawing.Size(233, 26);
			this.panel14.TabIndex = 30;
			// 
			// hiddenUnitsTextBox
			// 
			this.hiddenUnitsTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.hiddenUnitsTextBox.Location = new System.Drawing.Point(184, 3);
			this.hiddenUnitsTextBox.Name = "hiddenUnitsTextBox";
			this.hiddenUnitsTextBox.Size = new System.Drawing.Size(63, 20);
			this.hiddenUnitsTextBox.TabIndex = 15;
			this.hiddenUnitsTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.hiddenUnitsTextBox_KeyPress);
			this.hiddenUnitsTextBox.Leave += new System.EventHandler(this.hiddenUnitsTextBox_Leave);
			// 
			// labelHiddenUnits
			// 
			this.labelHiddenUnits.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelHiddenUnits.Location = new System.Drawing.Point(113, 3);
			this.labelHiddenUnits.Name = "labelHiddenUnits";
			this.labelHiddenUnits.Padding = new System.Windows.Forms.Padding(3, 0, 0, 0);
			this.labelHiddenUnits.Size = new System.Drawing.Size(71, 20);
			this.labelHiddenUnits.TabIndex = 14;
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
			this.visibleUnitsLabel.TabIndex = 13;
			this.visibleUnitsLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// labelVisibleUnits
			// 
			this.labelVisibleUnits.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelVisibleUnits.Location = new System.Drawing.Point(3, 3);
			this.labelVisibleUnits.Name = "labelVisibleUnits";
			this.labelVisibleUnits.Size = new System.Drawing.Size(65, 20);
			this.labelVisibleUnits.TabIndex = 8;
			this.labelVisibleUnits.Text = "Visible Units";
			this.labelVisibleUnits.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// label10
			// 
			this.label10.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.label10.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.label10.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label10.Location = new System.Drawing.Point(0, 78);
			this.label10.Name = "label10";
			this.label10.Size = new System.Drawing.Size(233, 22);
			this.label10.TabIndex = 21;
			this.label10.Text = "Model";
			this.label10.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// parametersPanel
			// 
			this.parametersPanel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.parametersPanel.Controls.Add(this.panel5);
			this.parametersPanel.Controls.Add(this.panel15);
			this.parametersPanel.Controls.Add(this.panel3);
			this.parametersPanel.Controls.Add(this.label14);
			this.parametersPanel.Dock = System.Windows.Forms.DockStyle.Left;
			this.parametersPanel.Location = new System.Drawing.Point(548, 0);
			this.parametersPanel.Name = "parametersPanel";
			this.parametersPanel.Size = new System.Drawing.Size(319, 102);
			this.parametersPanel.TabIndex = 26;
			// 
			// panel5
			// 
			this.panel5.Controls.Add(this.epochsTextBox);
			this.panel5.Controls.Add(this.labelEpochs);
			this.panel5.Controls.Add(this.minibatchSizeTextBox);
			this.panel5.Controls.Add(this.labelMinibatchSize);
			this.panel5.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel5.Location = new System.Drawing.Point(0, 52);
			this.panel5.Name = "panel5";
			this.panel5.Padding = new System.Windows.Forms.Padding(3);
			this.panel5.Size = new System.Drawing.Size(317, 26);
			this.panel5.TabIndex = 29;
			// 
			// epochsTextBox
			// 
			this.epochsTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.epochsTextBox.Location = new System.Drawing.Point(162, 3);
			this.epochsTextBox.Name = "epochsTextBox";
			this.epochsTextBox.Size = new System.Drawing.Size(40, 20);
			this.epochsTextBox.TabIndex = 11;
			this.epochsTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.epochsTextBox_KeyPress);
			this.epochsTextBox.Leave += new System.EventHandler(this.epochsTextBox_Leave);
			// 
			// labelEpochs
			// 
			this.labelEpochs.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelEpochs.Location = new System.Drawing.Point(119, 3);
			this.labelEpochs.Name = "labelEpochs";
			this.labelEpochs.Size = new System.Drawing.Size(43, 20);
			this.labelEpochs.TabIndex = 10;
			this.labelEpochs.Text = "Epochs";
			this.labelEpochs.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// minibatchSizeTextBox
			// 
			this.minibatchSizeTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.minibatchSizeTextBox.Location = new System.Drawing.Point(79, 3);
			this.minibatchSizeTextBox.Name = "minibatchSizeTextBox";
			this.minibatchSizeTextBox.Size = new System.Drawing.Size(40, 20);
			this.minibatchSizeTextBox.TabIndex = 9;
			this.minibatchSizeTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.minibatchSizeTextBox_KeyPress);
			this.minibatchSizeTextBox.Leave += new System.EventHandler(this.minibatchSizeTextBox_Leave);
			// 
			// labelMinibatchSize
			// 
			this.labelMinibatchSize.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelMinibatchSize.Location = new System.Drawing.Point(3, 3);
			this.labelMinibatchSize.Name = "labelMinibatchSize";
			this.labelMinibatchSize.Size = new System.Drawing.Size(76, 20);
			this.labelMinibatchSize.TabIndex = 8;
			this.labelMinibatchSize.Text = "Minibatch Size";
			this.labelMinibatchSize.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// panel15
			// 
			this.panel15.Controls.Add(this.hiddenDropoutTextBox);
			this.panel15.Controls.Add(this.labelHiddenDropout);
			this.panel15.Controls.Add(this.l2TextBox);
			this.panel15.Controls.Add(this.labelL2);
			this.panel15.Controls.Add(this.momentumTextBox);
			this.panel15.Controls.Add(this.labelMomentum);
			this.panel15.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel15.Location = new System.Drawing.Point(0, 26);
			this.panel15.Name = "panel15";
			this.panel15.Padding = new System.Windows.Forms.Padding(3);
			this.panel15.Size = new System.Drawing.Size(317, 26);
			this.panel15.TabIndex = 26;
			// 
			// l2TextBox
			// 
			this.l2TextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.l2TextBox.Location = new System.Drawing.Point(152, 3);
			this.l2TextBox.Name = "l2TextBox";
			this.l2TextBox.Size = new System.Drawing.Size(50, 20);
			this.l2TextBox.TabIndex = 13;
			this.l2TextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.l2TextBox_KeyPress);
			this.l2TextBox.Leave += new System.EventHandler(this.l2TextBox_Leave);
			// 
			// labelL2
			// 
			this.labelL2.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelL2.Location = new System.Drawing.Point(129, 3);
			this.labelL2.Name = "labelL2";
			this.labelL2.Size = new System.Drawing.Size(23, 20);
			this.labelL2.TabIndex = 12;
			this.labelL2.Text = "L2";
			this.labelL2.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// momentumTextBox
			// 
			this.momentumTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.momentumTextBox.Location = new System.Drawing.Point(79, 3);
			this.momentumTextBox.Name = "momentumTextBox";
			this.momentumTextBox.Size = new System.Drawing.Size(50, 20);
			this.momentumTextBox.TabIndex = 11;
			this.momentumTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.momentumTextBox_KeyPress);
			this.momentumTextBox.Leave += new System.EventHandler(this.momentumTextBox_Leave);
			// 
			// labelMomentum
			// 
			this.labelMomentum.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelMomentum.Location = new System.Drawing.Point(3, 3);
			this.labelMomentum.Name = "labelMomentum";
			this.labelMomentum.Size = new System.Drawing.Size(76, 20);
			this.labelMomentum.TabIndex = 10;
			this.labelMomentum.Text = "Momentum";
			this.labelMomentum.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// panel3
			// 
			this.panel3.Controls.Add(this.visibleDropoutTextBox);
			this.panel3.Controls.Add(this.labelVisibleDropout);
			this.panel3.Controls.Add(this.l1TextBox);
			this.panel3.Controls.Add(this.labelL1);
			this.panel3.Controls.Add(this.learningRateTextBox);
			this.panel3.Controls.Add(this.labelLearningRate);
			this.panel3.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel3.Location = new System.Drawing.Point(0, 0);
			this.panel3.Name = "panel3";
			this.panel3.Padding = new System.Windows.Forms.Padding(3);
			this.panel3.Size = new System.Drawing.Size(317, 26);
			this.panel3.TabIndex = 25;
			// 
			// l1TextBox
			// 
			this.l1TextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.l1TextBox.Location = new System.Drawing.Point(152, 3);
			this.l1TextBox.Name = "l1TextBox";
			this.l1TextBox.Size = new System.Drawing.Size(50, 20);
			this.l1TextBox.TabIndex = 3;
			this.l1TextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.l1TextBox_KeyPress);
			this.l1TextBox.Leave += new System.EventHandler(this.l1TextBox_Leave);
			// 
			// labelL1
			// 
			this.labelL1.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelL1.Location = new System.Drawing.Point(129, 3);
			this.labelL1.Name = "labelL1";
			this.labelL1.Size = new System.Drawing.Size(23, 20);
			this.labelL1.TabIndex = 2;
			this.labelL1.Text = "L1";
			this.labelL1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// learningRateTextBox
			// 
			this.learningRateTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.learningRateTextBox.Location = new System.Drawing.Point(79, 3);
			this.learningRateTextBox.Name = "learningRateTextBox";
			this.learningRateTextBox.Size = new System.Drawing.Size(50, 20);
			this.learningRateTextBox.TabIndex = 1;
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
			// label14
			// 
			this.label14.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.label14.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.label14.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label14.Location = new System.Drawing.Point(0, 78);
			this.label14.Name = "label14";
			this.label14.Size = new System.Drawing.Size(317, 22);
			this.label14.TabIndex = 29;
			this.label14.Text = "Parameters";
			this.label14.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// resetParametersButton
			// 
			this.resetParametersButton.Dock = System.Windows.Forms.DockStyle.Top;
			this.resetParametersButton.Image = ((System.Drawing.Image)(resources.GetObject("resetParametersButton.Image")));
			this.resetParametersButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.resetParametersButton.Location = new System.Drawing.Point(0, 51);
			this.resetParametersButton.Name = "resetParametersButton";
			this.resetParametersButton.Size = new System.Drawing.Size(62, 25);
			this.resetParametersButton.TabIndex = 33;
			this.resetParametersButton.Text = "Reset";
			this.resetParametersButton.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
			this.resetParametersButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.resetParametersButton.UseVisualStyleBackColor = true;
			this.resetParametersButton.Visible = false;
			this.resetParametersButton.Click += new System.EventHandler(this.resetParametersButton_Click);
			// 
			// saveParametersButton
			// 
			this.saveParametersButton.Dock = System.Windows.Forms.DockStyle.Top;
			this.saveParametersButton.Image = ((System.Drawing.Image)(resources.GetObject("saveParametersButton.Image")));
			this.saveParametersButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.saveParametersButton.Location = new System.Drawing.Point(0, 26);
			this.saveParametersButton.Name = "saveParametersButton";
			this.saveParametersButton.Size = new System.Drawing.Size(62, 25);
			this.saveParametersButton.TabIndex = 32;
			this.saveParametersButton.Text = "Save";
			this.saveParametersButton.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
			this.saveParametersButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.saveParametersButton.UseVisualStyleBackColor = true;
			this.saveParametersButton.Visible = false;
			this.saveParametersButton.Click += new System.EventHandler(this.saveParametersButton_Click);
			// 
			// loadParametersButton
			// 
			this.loadParametersButton.Dock = System.Windows.Forms.DockStyle.Top;
			this.loadParametersButton.Enabled = false;
			this.loadParametersButton.Image = ((System.Drawing.Image)(resources.GetObject("loadParametersButton.Image")));
			this.loadParametersButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.loadParametersButton.Location = new System.Drawing.Point(0, 1);
			this.loadParametersButton.Name = "loadParametersButton";
			this.loadParametersButton.Size = new System.Drawing.Size(62, 25);
			this.loadParametersButton.TabIndex = 31;
			this.loadParametersButton.Text = "Load";
			this.loadParametersButton.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
			this.loadParametersButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.loadParametersButton.UseVisualStyleBackColor = true;
			this.loadParametersButton.Click += new System.EventHandler(this.loadParametersButton_Click);
			// 
			// panel1
			// 
			this.panel1.Controls.Add(this.traingingIdxPathTextBox);
			this.panel1.Controls.Add(this.selectTrainingIdxButton);
			this.panel1.Controls.Add(this.labelInputData);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Name = "panel1";
			this.panel1.Padding = new System.Windows.Forms.Padding(3);
			this.panel1.Size = new System.Drawing.Size(311, 26);
			this.panel1.TabIndex = 15;
			// 
			// traingingIdxPathTextBox
			// 
			this.traingingIdxPathTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.traingingIdxPathTextBox.Location = new System.Drawing.Point(76, 3);
			this.traingingIdxPathTextBox.Name = "traingingIdxPathTextBox";
			this.traingingIdxPathTextBox.ReadOnly = true;
			this.traingingIdxPathTextBox.Size = new System.Drawing.Size(206, 20);
			this.traingingIdxPathTextBox.TabIndex = 3;
			// 
			// selectTrainingIdxButton
			// 
			this.selectTrainingIdxButton.AutoSize = true;
			this.selectTrainingIdxButton.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.selectTrainingIdxButton.Dock = System.Windows.Forms.DockStyle.Right;
			this.selectTrainingIdxButton.Location = new System.Drawing.Point(282, 3);
			this.selectTrainingIdxButton.Name = "selectTrainingIdxButton";
			this.selectTrainingIdxButton.Size = new System.Drawing.Size(26, 20);
			this.selectTrainingIdxButton.TabIndex = 2;
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
			// label9
			// 
			this.label9.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.label9.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.label9.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label9.Location = new System.Drawing.Point(0, 78);
			this.label9.Name = "label9";
			this.label9.Size = new System.Drawing.Size(311, 22);
			this.label9.TabIndex = 17;
			this.label9.Text = "Data";
			this.label9.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// labelPixelFormat
			// 
			this.labelPixelFormat.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelPixelFormat.Location = new System.Drawing.Point(3, 3);
			this.labelPixelFormat.Name = "labelPixelFormat";
			this.labelPixelFormat.Size = new System.Drawing.Size(73, 20);
			this.labelPixelFormat.TabIndex = 25;
			this.labelPixelFormat.Text = "Pixel Format";
			this.labelPixelFormat.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// dataPanel
			// 
			this.dataPanel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.dataPanel.Controls.Add(this.panel8);
			this.dataPanel.Controls.Add(this.panel2);
			this.dataPanel.Controls.Add(this.label9);
			this.dataPanel.Controls.Add(this.panel1);
			this.dataPanel.Dock = System.Windows.Forms.DockStyle.Left;
			this.dataPanel.Location = new System.Drawing.Point(0, 0);
			this.dataPanel.Name = "dataPanel";
			this.dataPanel.Size = new System.Drawing.Size(313, 102);
			this.dataPanel.TabIndex = 21;
			// 
			// panel8
			// 
			this.panel8.Controls.Add(this.heightTextBox);
			this.panel8.Controls.Add(this.labelHeight);
			this.panel8.Controls.Add(this.widthTextBox);
			this.panel8.Controls.Add(this.labelWidth);
			this.panel8.Controls.Add(this.pixelFormatComboBox);
			this.panel8.Controls.Add(this.labelPixelFormat);
			this.panel8.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel8.Location = new System.Drawing.Point(0, 52);
			this.panel8.Name = "panel8";
			this.panel8.Padding = new System.Windows.Forms.Padding(3);
			this.panel8.Size = new System.Drawing.Size(311, 26);
			this.panel8.TabIndex = 31;
			// 
			// heightTextBox
			// 
			this.heightTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.heightTextBox.Location = new System.Drawing.Point(266, 3);
			this.heightTextBox.Name = "heightTextBox";
			this.heightTextBox.Size = new System.Drawing.Size(41, 20);
			this.heightTextBox.TabIndex = 35;
			this.heightTextBox.TextChanged += new System.EventHandler(this.heightTextBox_Leave);
			this.heightTextBox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.heightTextBox_KeyDown);
			// 
			// labelHeight
			// 
			this.labelHeight.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelHeight.Location = new System.Drawing.Point(228, 3);
			this.labelHeight.Name = "labelHeight";
			this.labelHeight.Size = new System.Drawing.Size(38, 20);
			this.labelHeight.TabIndex = 34;
			this.labelHeight.Text = "Height";
			this.labelHeight.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// widthTextBox
			// 
			this.widthTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.widthTextBox.Location = new System.Drawing.Point(187, 3);
			this.widthTextBox.Name = "widthTextBox";
			this.widthTextBox.Size = new System.Drawing.Size(41, 20);
			this.widthTextBox.TabIndex = 31;
			this.widthTextBox.TextChanged += new System.EventHandler(this.widthTextBox_Leave);
			this.widthTextBox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.widthTextBox_KeyDown);
			// 
			// labelWidth
			// 
			this.labelWidth.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelWidth.Location = new System.Drawing.Point(152, 3);
			this.labelWidth.Name = "labelWidth";
			this.labelWidth.Size = new System.Drawing.Size(35, 20);
			this.labelWidth.TabIndex = 32;
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
			this.pixelFormatComboBox.TabIndex = 30;
			this.pixelFormatComboBox.SelectionChangeCommitted += new System.EventHandler(this.pixelFormatComboBox_SelectionChangeCommitted);
			// 
			// panel2
			// 
			this.panel2.Controls.Add(this.validationIdxPathTextBox);
			this.panel2.Controls.Add(this.clearValidationDataButton);
			this.panel2.Controls.Add(this.selectValidationIdxButton);
			this.panel2.Controls.Add(this.labelValidationData);
			this.panel2.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel2.Location = new System.Drawing.Point(0, 26);
			this.panel2.Name = "panel2";
			this.panel2.Padding = new System.Windows.Forms.Padding(3);
			this.panel2.Size = new System.Drawing.Size(311, 26);
			this.panel2.TabIndex = 30;
			// 
			// validationIdxPathTextBox
			// 
			this.validationIdxPathTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.validationIdxPathTextBox.Location = new System.Drawing.Point(76, 3);
			this.validationIdxPathTextBox.Name = "validationIdxPathTextBox";
			this.validationIdxPathTextBox.ReadOnly = true;
			this.validationIdxPathTextBox.Size = new System.Drawing.Size(188, 20);
			this.validationIdxPathTextBox.TabIndex = 4;
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
			this.selectValidationIdxButton.TabIndex = 2;
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
			this.startButton.Location = new System.Drawing.Point(983, 0);
			this.startButton.Name = "startButton";
			this.startButton.Size = new System.Drawing.Size(52, 102);
			this.startButton.TabIndex = 27;
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
			this.stopButton.Location = new System.Drawing.Point(1087, 0);
			this.stopButton.Name = "stopButton";
			this.stopButton.Size = new System.Drawing.Size(52, 102);
			this.stopButton.TabIndex = 28;
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
			this.pauseButton.Location = new System.Drawing.Point(1035, 0);
			this.pauseButton.Name = "pauseButton";
			this.pauseButton.Size = new System.Drawing.Size(52, 102);
			this.pauseButton.TabIndex = 29;
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
			this.exportButton.Location = new System.Drawing.Point(1139, 0);
			this.exportButton.Name = "exportButton";
			this.exportButton.Size = new System.Drawing.Size(52, 102);
			this.exportButton.TabIndex = 30;
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
			this.importButton.Location = new System.Drawing.Point(931, 0);
			this.importButton.Name = "importButton";
			this.importButton.Size = new System.Drawing.Size(52, 102);
			this.importButton.TabIndex = 31;
			this.importButton.Text = "Import";
			this.importButton.TextAlign = System.Drawing.ContentAlignment.BottomCenter;
			this.importButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageAboveText;
			this.importButton.UseVisualStyleBackColor = true;
			this.importButton.Click += new System.EventHandler(this.importButton_Click);
			// 
			// panel4
			// 
			this.panel4.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.panel4.Controls.Add(this.label1);
			this.panel4.Controls.Add(this.resetParametersButton);
			this.panel4.Controls.Add(this.saveParametersButton);
			this.panel4.Controls.Add(this.loadParametersButton);
			this.panel4.Dock = System.Windows.Forms.DockStyle.Left;
			this.panel4.Location = new System.Drawing.Point(867, 0);
			this.panel4.Name = "panel4";
			this.panel4.Padding = new System.Windows.Forms.Padding(0, 1, 0, 0);
			this.panel4.Size = new System.Drawing.Size(64, 102);
			this.panel4.TabIndex = 32;
			// 
			// label1
			// 
			this.label1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.label1.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label1.Location = new System.Drawing.Point(0, 78);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(62, 22);
			this.label1.TabIndex = 34;
			this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// labelVisibleDropout
			// 
			this.labelVisibleDropout.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelVisibleDropout.Location = new System.Drawing.Point(202, 3);
			this.labelVisibleDropout.Name = "labelVisibleDropout";
			this.labelVisibleDropout.Size = new System.Drawing.Size(82, 20);
			this.labelVisibleDropout.TabIndex = 4;
			this.labelVisibleDropout.Text = "Visible Dropout";
			this.labelVisibleDropout.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// labelHiddenDropout
			// 
			this.labelHiddenDropout.Dock = System.Windows.Forms.DockStyle.Left;
			this.labelHiddenDropout.Location = new System.Drawing.Point(202, 3);
			this.labelHiddenDropout.Name = "labelHiddenDropout";
			this.labelHiddenDropout.Size = new System.Drawing.Size(82, 20);
			this.labelHiddenDropout.TabIndex = 14;
			this.labelHiddenDropout.Text = "Hidden Dropout";
			this.labelHiddenDropout.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// visibleDropoutTextBox
			// 
			this.visibleDropoutTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.visibleDropoutTextBox.Location = new System.Drawing.Point(284, 3);
			this.visibleDropoutTextBox.Name = "visibleDropoutTextBox";
			this.visibleDropoutTextBox.Size = new System.Drawing.Size(30, 20);
			this.visibleDropoutTextBox.TabIndex = 12;
			this.visibleDropoutTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.visibleDropoutTextBox_KeyPress);
			this.visibleDropoutTextBox.Leave += new System.EventHandler(this.visibleDropoutTextBox_Leave);
			// 
			// hiddenDropoutTextBox
			// 
			this.hiddenDropoutTextBox.Dock = System.Windows.Forms.DockStyle.Left;
			this.hiddenDropoutTextBox.Location = new System.Drawing.Point(284, 3);
			this.hiddenDropoutTextBox.Name = "hiddenDropoutTextBox";
			this.hiddenDropoutTextBox.Size = new System.Drawing.Size(30, 20);
			this.hiddenDropoutTextBox.TabIndex = 15;
			this.hiddenDropoutTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.hiddenDropoutTextBox_KeyPress);
			this.hiddenDropoutTextBox.Leave += new System.EventHandler(this.hiddenDropoutTextBox_Leave);
			// 
			// SettingsBar
			// 
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
			this.Controls.Add(this.exportButton);
			this.Controls.Add(this.stopButton);
			this.Controls.Add(this.pauseButton);
			this.Controls.Add(this.startButton);
			this.Controls.Add(this.importButton);
			this.Controls.Add(this.panel4);
			this.Controls.Add(this.parametersPanel);
			this.Controls.Add(this.modelPanel);
			this.Controls.Add(this.dataPanel);
			this.Margin = new System.Windows.Forms.Padding(0);
			this.MinimumSize = new System.Drawing.Size(1228, 102);
			this.Name = "SettingsBar";
			this.Size = new System.Drawing.Size(1228, 102);
			this.modelPanel.ResumeLayout(false);
			this.panel7.ResumeLayout(false);
			this.panel6.ResumeLayout(false);
			this.panel14.ResumeLayout(false);
			this.panel14.PerformLayout();
			this.parametersPanel.ResumeLayout(false);
			this.panel5.ResumeLayout(false);
			this.panel5.PerformLayout();
			this.panel15.ResumeLayout(false);
			this.panel15.PerformLayout();
			this.panel3.ResumeLayout(false);
			this.panel3.PerformLayout();
			this.panel1.ResumeLayout(false);
			this.panel1.PerformLayout();
			this.dataPanel.ResumeLayout(false);
			this.panel8.ResumeLayout(false);
			this.panel8.PerformLayout();
			this.panel2.ResumeLayout(false);
			this.panel2.PerformLayout();
			this.panel4.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Panel modelPanel;
		private System.Windows.Forms.Panel panel7;
		private System.Windows.Forms.Label label10;
		private System.Windows.Forms.ComboBox modelTypeComboBox;
		private System.Windows.Forms.Label labelModelType;
		private System.Windows.Forms.Panel parametersPanel;
		private System.Windows.Forms.Label label14;
		private System.Windows.Forms.Panel panel14;
		private System.Windows.Forms.Label labelVisibleUnits;
		private System.Windows.Forms.Panel panel6;
		private System.Windows.Forms.ComboBox visibleTypeComboBox;
		private System.Windows.Forms.Label labelVisibleType;
		private System.Windows.Forms.ComboBox hiddenTypeComboBox;
		private System.Windows.Forms.Label labelHiddenType;
		private System.Windows.Forms.TextBox hiddenUnitsTextBox;
		private System.Windows.Forms.Label labelHiddenUnits;
		private System.Windows.Forms.Label visibleUnitsLabel;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.TextBox traingingIdxPathTextBox;
		private System.Windows.Forms.Button selectTrainingIdxButton;
		private System.Windows.Forms.Label labelInputData;
		private System.Windows.Forms.Label label9;
		protected System.Windows.Forms.Label labelPixelFormat;
		private System.Windows.Forms.Panel dataPanel;
		private System.Windows.Forms.Panel panel2;
		private System.Windows.Forms.Label labelValidationData;
		private System.Windows.Forms.Button selectValidationIdxButton;
		private System.Windows.Forms.TextBox validationIdxPathTextBox;
		private System.Windows.Forms.Button clearValidationDataButton;
		private System.Windows.Forms.Panel panel8;
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
		private System.Windows.Forms.Button saveParametersButton;
		private System.Windows.Forms.Button loadParametersButton;
		private System.Windows.Forms.Panel panel5;
		private System.Windows.Forms.Panel panel15;
		private System.Windows.Forms.Panel panel3;
		private System.Windows.Forms.TextBox learningRateTextBox;
		private System.Windows.Forms.Label labelLearningRate;
		private System.Windows.Forms.Button resetParametersButton;
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
		private System.Windows.Forms.Panel panel4;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox hiddenDropoutTextBox;
		private System.Windows.Forms.Label labelHiddenDropout;
		private System.Windows.Forms.TextBox visibleDropoutTextBox;
		private System.Windows.Forms.Label labelVisibleDropout;

	}
}
