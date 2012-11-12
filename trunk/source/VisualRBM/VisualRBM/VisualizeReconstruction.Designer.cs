namespace VisualRBM
{
	partial class VisualizeReconstruction
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
		protected void InitializeComponent()
		{
			this.panel1 = new System.Windows.Forms.Panel();
			this.zoomOutButton = new System.Windows.Forms.Button();
			this.zoomInButton = new System.Windows.Forms.Button();
			this.imageFlowPanel = new System.Windows.Forms.FlowLayoutPanel();
			this.panel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// panel1
			// 
			this.panel1.BackColor = System.Drawing.SystemColors.Control;
			this.panel1.Controls.Add(this.zoomOutButton);
			this.panel1.Controls.Add(this.zoomInButton);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(365, 55);
			this.panel1.TabIndex = 0;
			// 
			// zoomOutButton
			// 
			this.zoomOutButton.Location = new System.Drawing.Point(57, 4);
			this.zoomOutButton.Name = "zoomOutButton";
			this.zoomOutButton.Size = new System.Drawing.Size(48, 48);
			this.zoomOutButton.TabIndex = 7;
			this.zoomOutButton.Text = "Zoom out";
			this.zoomOutButton.UseVisualStyleBackColor = true;
			this.zoomOutButton.Click += new System.EventHandler(this.zoomOutButton_Click);
			// 
			// zoomInButton
			// 
			this.zoomInButton.Location = new System.Drawing.Point(3, 4);
			this.zoomInButton.Name = "zoomInButton";
			this.zoomInButton.Size = new System.Drawing.Size(48, 48);
			this.zoomInButton.TabIndex = 6;
			this.zoomInButton.Text = "Zoom in";
			this.zoomInButton.UseVisualStyleBackColor = true;
			this.zoomInButton.Click += new System.EventHandler(this.zoomInButton_Click);
			// 
			// imageFlowPanel
			// 
			this.imageFlowPanel.AutoScroll = true;
			this.imageFlowPanel.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.imageFlowPanel.BackColor = System.Drawing.SystemColors.ControlDark;
			this.imageFlowPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.imageFlowPanel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.imageFlowPanel.Location = new System.Drawing.Point(0, 55);
			this.imageFlowPanel.Name = "imageFlowPanel";
			this.imageFlowPanel.Padding = new System.Windows.Forms.Padding(2);
			this.imageFlowPanel.Size = new System.Drawing.Size(365, 165);
			this.imageFlowPanel.TabIndex = 1;
			// 
			// VisualizeReconstruction
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.imageFlowPanel);
			this.Controls.Add(this.panel1);
			this.Name = "VisualizeReconstruction";
			this.Size = new System.Drawing.Size(365, 220);
			this.panel1.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		protected System.Windows.Forms.Panel panel1;
		protected System.Windows.Forms.FlowLayoutPanel imageFlowPanel;
		protected System.Windows.Forms.Button zoomOutButton;
		protected System.Windows.Forms.Button zoomInButton;
	}
}
