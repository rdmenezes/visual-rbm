namespace VisualRBM
{
	partial class Main
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

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Main));
			this.hiddenTab = new System.Windows.Forms.TabPage();
			this.visualizeHidden = new VisualRBM.VisualizeHiddenProbabilities();
			this.featuresTab = new System.Windows.Forms.TabPage();
			this.visualizeWeights = new VisualRBM.VisualizeFeatureDetectors();
			this.visibleTab = new System.Windows.Forms.TabPage();
			this.visualizeVisible = new VisualRBM.VisualizeReconstruction();
			this.graphTab = new System.Windows.Forms.TabPage();
			this.visualizeReconstructionError = new VisualRBM.VisualizeReconstructionError();
			this.Maintabs = new System.Windows.Forms.TabControl();
			this.logTab = new System.Windows.Forms.TabPage();
			this.trainingLog = new VisualRBM.TrainingLog();
			this.settingsBar = new VisualRBM.SettingsBar();
			this.hiddenTab.SuspendLayout();
			this.featuresTab.SuspendLayout();
			this.visibleTab.SuspendLayout();
			this.graphTab.SuspendLayout();
			this.Maintabs.SuspendLayout();
			this.logTab.SuspendLayout();
			this.SuspendLayout();
			// 
			// hiddenTab
			// 
			this.hiddenTab.BackColor = System.Drawing.SystemColors.Control;
			this.hiddenTab.Controls.Add(this.visualizeHidden);
			this.hiddenTab.Location = new System.Drawing.Point(4, 22);
			this.hiddenTab.Name = "hiddenTab";
			this.hiddenTab.Padding = new System.Windows.Forms.Padding(3);
			this.hiddenTab.Size = new System.Drawing.Size(1072, 370);
			this.hiddenTab.TabIndex = 3;
			this.hiddenTab.Text = "Hidden";
			// 
			// visualizeHidden
			// 
			this.visualizeHidden.Dock = System.Windows.Forms.DockStyle.Fill;
			this.visualizeHidden.Location = new System.Drawing.Point(3, 3);
			this.visualizeHidden.Name = "visualizeHidden";
			this.visualizeHidden.Size = new System.Drawing.Size(1066, 364);
			this.visualizeHidden.TabIndex = 0;
			// 
			// featuresTab
			// 
			this.featuresTab.BackColor = System.Drawing.SystemColors.Control;
			this.featuresTab.Controls.Add(this.visualizeWeights);
			this.featuresTab.Location = new System.Drawing.Point(4, 22);
			this.featuresTab.Name = "featuresTab";
			this.featuresTab.Padding = new System.Windows.Forms.Padding(3);
			this.featuresTab.Size = new System.Drawing.Size(1072, 370);
			this.featuresTab.TabIndex = 2;
			this.featuresTab.Text = "Features";
			// 
			// visualizeWeights
			// 
			this.visualizeWeights.Dock = System.Windows.Forms.DockStyle.Fill;
			this.visualizeWeights.Location = new System.Drawing.Point(3, 3);
			this.visualizeWeights.Name = "visualizeWeights";
			this.visualizeWeights.Size = new System.Drawing.Size(1066, 364);
			this.visualizeWeights.TabIndex = 0;
			// 
			// visibleTab
			// 
			this.visibleTab.BackColor = System.Drawing.SystemColors.Control;
			this.visibleTab.Controls.Add(this.visualizeVisible);
			this.visibleTab.Location = new System.Drawing.Point(4, 22);
			this.visibleTab.Name = "visibleTab";
			this.visibleTab.Padding = new System.Windows.Forms.Padding(3);
			this.visibleTab.Size = new System.Drawing.Size(1072, 370);
			this.visibleTab.TabIndex = 1;
			this.visibleTab.Text = "Visible";
			// 
			// visualizeVisible
			// 
			this.visualizeVisible.Dock = System.Windows.Forms.DockStyle.Fill;
			this.visualizeVisible.Location = new System.Drawing.Point(3, 3);
			this.visualizeVisible.Name = "visualizeVisible";
			this.visualizeVisible.Size = new System.Drawing.Size(1066, 364);
			this.visualizeVisible.TabIndex = 0;
			// 
			// graphTab
			// 
			this.graphTab.BackColor = System.Drawing.SystemColors.Control;
			this.graphTab.Controls.Add(this.visualizeReconstructionError);
			this.graphTab.Location = new System.Drawing.Point(4, 22);
			this.graphTab.Name = "graphTab";
			this.graphTab.Padding = new System.Windows.Forms.Padding(3);
			this.graphTab.Size = new System.Drawing.Size(1072, 370);
			this.graphTab.TabIndex = 0;
			this.graphTab.Text = "Graphs";
			// 
			// visualizeReconstructionError
			// 
			this.visualizeReconstructionError.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.visualizeReconstructionError.Dock = System.Windows.Forms.DockStyle.Fill;
			this.visualizeReconstructionError.Location = new System.Drawing.Point(3, 3);
			this.visualizeReconstructionError.Name = "visualizeReconstructionError";
			this.visualizeReconstructionError.Size = new System.Drawing.Size(1066, 364);
			this.visualizeReconstructionError.TabIndex = 0;
			// 
			// Maintabs
			// 
			this.Maintabs.Controls.Add(this.graphTab);
			this.Maintabs.Controls.Add(this.visibleTab);
			this.Maintabs.Controls.Add(this.hiddenTab);
			this.Maintabs.Controls.Add(this.featuresTab);
			this.Maintabs.Controls.Add(this.logTab);
			this.Maintabs.Dock = System.Windows.Forms.DockStyle.Fill;
			this.Maintabs.Location = new System.Drawing.Point(0, 104);
			this.Maintabs.Name = "Maintabs";
			this.Maintabs.SelectedIndex = 0;
			this.Maintabs.Size = new System.Drawing.Size(1080, 396);
			this.Maintabs.TabIndex = 3;
			// 
			// logTab
			// 
			this.logTab.BackColor = System.Drawing.SystemColors.Control;
			this.logTab.Controls.Add(this.trainingLog);
			this.logTab.Location = new System.Drawing.Point(4, 22);
			this.logTab.Name = "logTab";
			this.logTab.Padding = new System.Windows.Forms.Padding(3);
			this.logTab.Size = new System.Drawing.Size(1072, 370);
			this.logTab.TabIndex = 4;
			this.logTab.Text = "Log";
			// 
			// trainingLog
			// 
			this.trainingLog.Dock = System.Windows.Forms.DockStyle.Fill;
			this.trainingLog.Location = new System.Drawing.Point(3, 3);
			this.trainingLog.Name = "trainingLog";
			this.trainingLog.Size = new System.Drawing.Size(1066, 364);
			this.trainingLog.TabIndex = 0;
			// 
			// settingsBar
			// 
			this.settingsBar.Dock = System.Windows.Forms.DockStyle.Top;
			this.settingsBar.Location = new System.Drawing.Point(0, 0);
			this.settingsBar.Margin = new System.Windows.Forms.Padding(0);
			this.settingsBar.Name = "settingsBar";
			this.settingsBar.Size = new System.Drawing.Size(1150, 100);
			this.settingsBar.TabIndex = 2;
			// 
			// Main
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(1150, 500);
			this.Controls.Add(this.Maintabs);
			this.Controls.Add(this.settingsBar);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "Main";
			this.Text = "Visual RBM";
			this.hiddenTab.ResumeLayout(false);
			this.featuresTab.ResumeLayout(false);
			this.visibleTab.ResumeLayout(false);
			this.graphTab.ResumeLayout(false);
			this.Maintabs.ResumeLayout(false);
			this.logTab.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		internal SettingsBar settingsBar;
		private System.Windows.Forms.TabPage hiddenTab;
		internal VisualizeHiddenProbabilities visualizeHidden;
		private System.Windows.Forms.TabPage featuresTab;
		internal VisualizeFeatureDetectors visualizeWeights;
		private System.Windows.Forms.TabPage visibleTab;
		internal VisualizeReconstruction visualizeVisible;
		private System.Windows.Forms.TabPage graphTab;
		internal VisualizeReconstructionError visualizeReconstructionError;
		private System.Windows.Forms.TabControl Maintabs;
		private System.Windows.Forms.TabPage logTab;
		public TrainingLog trainingLog;




	}
}

