namespace MolflowElements
{
    partial class MoveFacet
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
            this.offsetCheckbox = new System.Windows.Forms.CheckBox();
            this.directionCheckBox = new System.Windows.Forms.CheckBox();
            this.distanceText = new System.Windows.Forms.TextBox();
            this.dxLabel = new System.Windows.Forms.Label();
            this.xText = new System.Windows.Forms.TextBox();
            this.cmLabelX = new System.Windows.Forms.Label();
            this.cmLabelY = new System.Windows.Forms.Label();
            this.yText = new System.Windows.Forms.TextBox();
            this.dyLabel = new System.Windows.Forms.Label();
            this.cmLabelZ = new System.Windows.Forms.Label();
            this.zText = new System.Windows.Forms.TextBox();
            this.dzLabel = new System.Windows.Forms.Label();
            this.directionPanel = new System.Windows.Forms.GroupBox();
            this.dirPanel = new System.Windows.Forms.GroupBox();
            this.dirVertexButton = new System.Windows.Forms.Button();
            this.dirFacetCenterButton = new System.Windows.Forms.Button();
            this.directionStatusLabel = new System.Windows.Forms.Label();
            this.basePanel = new System.Windows.Forms.GroupBox();
            this.baseVertexButton = new System.Windows.Forms.Button();
            this.baseFacetCenterButton = new System.Windows.Forms.Button();
            this.baseStatusLabel = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.facetNormalButton = new System.Windows.Forms.Button();
            this.copyButton = new System.Windows.Forms.Button();
            this.moveButton = new System.Windows.Forms.Button();
            this.directionPanel.SuspendLayout();
            this.dirPanel.SuspendLayout();
            this.basePanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // offsetCheckbox
            // 
            this.offsetCheckbox.AutoSize = true;
            this.offsetCheckbox.Location = new System.Drawing.Point(5, 6);
            this.offsetCheckbox.Name = "offsetCheckbox";
            this.offsetCheckbox.Size = new System.Drawing.Size(96, 17);
            this.offsetCheckbox.TabIndex = 0;
            this.offsetCheckbox.Text = "Absolute offset";
            this.offsetCheckbox.UseVisualStyleBackColor = true;
            // 
            // directionCheckBox
            // 
            this.directionCheckBox.AutoSize = true;
            this.directionCheckBox.Location = new System.Drawing.Point(100, 6);
            this.directionCheckBox.Name = "directionCheckBox";
            this.directionCheckBox.Size = new System.Drawing.Size(132, 17);
            this.directionCheckBox.TabIndex = 1;
            this.directionCheckBox.Text = "Direction and distance";
            this.directionCheckBox.UseVisualStyleBackColor = true;
            // 
            // distanceText
            // 
            this.distanceText.Location = new System.Drawing.Point(63, 23);
            this.distanceText.Name = "distanceText";
            this.distanceText.Size = new System.Drawing.Size(98, 20);
            this.distanceText.TabIndex = 2;
            this.distanceText.TextChanged += new System.EventHandler(this.offsetText_TextChanged);
            // 
            // dxLabel
            // 
            this.dxLabel.AutoSize = true;
            this.dxLabel.Location = new System.Drawing.Point(8, 32);
            this.dxLabel.Name = "dxLabel";
            this.dxLabel.Size = new System.Drawing.Size(20, 13);
            this.dxLabel.TabIndex = 4;
            this.dxLabel.Text = "dX";
            // 
            // xText
            // 
            this.xText.Location = new System.Drawing.Point(49, 29);
            this.xText.Name = "xText";
            this.xText.Size = new System.Drawing.Size(116, 20);
            this.xText.TabIndex = 5;
            this.xText.Text = "0";
            // 
            // cmLabelX
            // 
            this.cmLabelX.AutoSize = true;
            this.cmLabelX.Location = new System.Drawing.Point(171, 32);
            this.cmLabelX.Name = "cmLabelX";
            this.cmLabelX.Size = new System.Drawing.Size(21, 13);
            this.cmLabelX.TabIndex = 6;
            this.cmLabelX.Text = "cm";
            // 
            // cmLabelY
            // 
            this.cmLabelY.AutoSize = true;
            this.cmLabelY.Location = new System.Drawing.Point(171, 58);
            this.cmLabelY.Name = "cmLabelY";
            this.cmLabelY.Size = new System.Drawing.Size(21, 13);
            this.cmLabelY.TabIndex = 9;
            this.cmLabelY.Text = "cm";
            // 
            // yText
            // 
            this.yText.Location = new System.Drawing.Point(49, 55);
            this.yText.Name = "yText";
            this.yText.Size = new System.Drawing.Size(116, 20);
            this.yText.TabIndex = 8;
            this.yText.Text = "0";
            // 
            // dyLabel
            // 
            this.dyLabel.AutoSize = true;
            this.dyLabel.Location = new System.Drawing.Point(8, 58);
            this.dyLabel.Name = "dyLabel";
            this.dyLabel.Size = new System.Drawing.Size(20, 13);
            this.dyLabel.TabIndex = 7;
            this.dyLabel.Text = "dY";
            // 
            // cmLabelZ
            // 
            this.cmLabelZ.AutoSize = true;
            this.cmLabelZ.Location = new System.Drawing.Point(171, 84);
            this.cmLabelZ.Name = "cmLabelZ";
            this.cmLabelZ.Size = new System.Drawing.Size(21, 13);
            this.cmLabelZ.TabIndex = 12;
            this.cmLabelZ.Text = "cm";
            // 
            // zText
            // 
            this.zText.Location = new System.Drawing.Point(49, 81);
            this.zText.Name = "zText";
            this.zText.Size = new System.Drawing.Size(116, 20);
            this.zText.TabIndex = 11;
            this.zText.Text = "0";
            // 
            // dzLabel
            // 
            this.dzLabel.AutoSize = true;
            this.dzLabel.Location = new System.Drawing.Point(8, 84);
            this.dzLabel.Name = "dzLabel";
            this.dzLabel.Size = new System.Drawing.Size(20, 13);
            this.dzLabel.TabIndex = 10;
            this.dzLabel.Text = "dZ";
            // 
            // directionPanel
            // 
            this.directionPanel.Controls.Add(this.dirPanel);
            this.directionPanel.Controls.Add(this.basePanel);
            this.directionPanel.Controls.Add(this.label4);
            this.directionPanel.Controls.Add(this.label1);
            this.directionPanel.Controls.Add(this.facetNormalButton);
            this.directionPanel.Controls.Add(this.distanceText);
            this.directionPanel.Location = new System.Drawing.Point(4, 111);
            this.directionPanel.Name = "directionPanel";
            this.directionPanel.Size = new System.Drawing.Size(216, 187);
            this.directionPanel.TabIndex = 13;
            this.directionPanel.TabStop = false;
            this.directionPanel.Text = "In direction";
            // 
            // dirPanel
            // 
            this.dirPanel.Controls.Add(this.dirVertexButton);
            this.dirPanel.Controls.Add(this.dirFacetCenterButton);
            this.dirPanel.Controls.Add(this.directionStatusLabel);
            this.dirPanel.Location = new System.Drawing.Point(110, 92);
            this.dirPanel.Name = "dirPanel";
            this.dirPanel.Size = new System.Drawing.Size(100, 89);
            this.dirPanel.TabIndex = 16;
            this.dirPanel.TabStop = false;
            this.dirPanel.Text = "Direction";
            // 
            // dirVertexButton
            // 
            this.dirVertexButton.Location = new System.Drawing.Point(6, 31);
            this.dirVertexButton.Name = "dirVertexButton";
            this.dirVertexButton.Size = new System.Drawing.Size(84, 20);
            this.dirVertexButton.TabIndex = 6;
            this.dirVertexButton.Text = "Selected Vertex";
            this.dirVertexButton.UseVisualStyleBackColor = true;
            this.dirVertexButton.Click += new System.EventHandler(this.button4_Click);
            // 
            // dirFacetCenterButton
            // 
            this.dirFacetCenterButton.Location = new System.Drawing.Point(6, 60);
            this.dirFacetCenterButton.Name = "dirFacetCenterButton";
            this.dirFacetCenterButton.Size = new System.Drawing.Size(84, 20);
            this.dirFacetCenterButton.TabIndex = 8;
            this.dirFacetCenterButton.Text = "Facet center";
            this.dirFacetCenterButton.UseVisualStyleBackColor = true;
            this.dirFacetCenterButton.Click += new System.EventHandler(this.button6_Click);
            // 
            // directionStatusLabel
            // 
            this.directionStatusLabel.AutoSize = true;
            this.directionStatusLabel.Location = new System.Drawing.Point(9, 14);
            this.directionStatusLabel.Name = "directionStatusLabel";
            this.directionStatusLabel.Size = new System.Drawing.Size(88, 13);
            this.directionStatusLabel.TabIndex = 10;
            this.directionStatusLabel.Text = "Choose base first";
            // 
            // basePanel
            // 
            this.basePanel.Controls.Add(this.baseVertexButton);
            this.basePanel.Controls.Add(this.baseFacetCenterButton);
            this.basePanel.Controls.Add(this.baseStatusLabel);
            this.basePanel.Location = new System.Drawing.Point(8, 92);
            this.basePanel.Name = "basePanel";
            this.basePanel.Size = new System.Drawing.Size(100, 89);
            this.basePanel.TabIndex = 15;
            this.basePanel.TabStop = false;
            this.basePanel.Text = "Base";
            // 
            // baseVertexButton
            // 
            this.baseVertexButton.Location = new System.Drawing.Point(6, 31);
            this.baseVertexButton.Name = "baseVertexButton";
            this.baseVertexButton.Size = new System.Drawing.Size(84, 20);
            this.baseVertexButton.TabIndex = 3;
            this.baseVertexButton.Text = "Selected Vertex";
            this.baseVertexButton.UseVisualStyleBackColor = true;
            this.baseVertexButton.Click += new System.EventHandler(this.button1_Click);
            // 
            // baseFacetCenterButton
            // 
            this.baseFacetCenterButton.Location = new System.Drawing.Point(6, 60);
            this.baseFacetCenterButton.Name = "baseFacetCenterButton";
            this.baseFacetCenterButton.Size = new System.Drawing.Size(84, 20);
            this.baseFacetCenterButton.TabIndex = 5;
            this.baseFacetCenterButton.Text = "Facet center";
            this.baseFacetCenterButton.UseVisualStyleBackColor = true;
            this.baseFacetCenterButton.Click += new System.EventHandler(this.button3_Click);
            // 
            // baseStatusLabel
            // 
            this.baseStatusLabel.AutoSize = true;
            this.baseStatusLabel.Location = new System.Drawing.Point(7, 14);
            this.baseStatusLabel.Name = "baseStatusLabel";
            this.baseStatusLabel.Size = new System.Drawing.Size(61, 13);
            this.baseStatusLabel.TabIndex = 9;
            this.baseStatusLabel.Text = "base status";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(167, 26);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(21, 13);
            this.label4.TabIndex = 14;
            this.label4.Text = "cm";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(8, 26);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(52, 13);
            this.label1.TabIndex = 14;
            this.label1.Text = "Distance:";
            // 
            // facetNormalButton
            // 
            this.facetNormalButton.Location = new System.Drawing.Point(62, 56);
            this.facetNormalButton.Name = "facetNormalButton";
            this.facetNormalButton.Size = new System.Drawing.Size(99, 20);
            this.facetNormalButton.TabIndex = 0;
            this.facetNormalButton.Text = "Facet normal";
            this.facetNormalButton.UseVisualStyleBackColor = true;
            // 
            // copyButton
            // 
            this.copyButton.Location = new System.Drawing.Point(120, 314);
            this.copyButton.Name = "copyButton";
            this.copyButton.Size = new System.Drawing.Size(84, 20);
            this.copyButton.TabIndex = 12;
            this.copyButton.Text = "Copy facets";
            this.copyButton.UseVisualStyleBackColor = true;
            // 
            // moveButton
            // 
            this.moveButton.Location = new System.Drawing.Point(18, 314);
            this.moveButton.Name = "moveButton";
            this.moveButton.Size = new System.Drawing.Size(84, 20);
            this.moveButton.TabIndex = 11;
            this.moveButton.Text = "Move facets";
            this.moveButton.UseVisualStyleBackColor = true;
            // 
            // MoveFacet
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(213, 350);
            this.Controls.Add(this.copyButton);
            this.Controls.Add(this.moveButton);
            this.Controls.Add(this.directionPanel);
            this.Controls.Add(this.cmLabelZ);
            this.Controls.Add(this.zText);
            this.Controls.Add(this.dzLabel);
            this.Controls.Add(this.cmLabelY);
            this.Controls.Add(this.yText);
            this.Controls.Add(this.dyLabel);
            this.Controls.Add(this.cmLabelX);
            this.Controls.Add(this.xText);
            this.Controls.Add(this.dxLabel);
            this.Controls.Add(this.directionCheckBox);
            this.Controls.Add(this.offsetCheckbox);
            this.Name = "MoveFacet";
            this.Text = "Move facet";
            this.directionPanel.ResumeLayout(false);
            this.directionPanel.PerformLayout();
            this.dirPanel.ResumeLayout(false);
            this.dirPanel.PerformLayout();
            this.basePanel.ResumeLayout(false);
            this.basePanel.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.CheckBox offsetCheckbox;
        private System.Windows.Forms.CheckBox directionCheckBox;
        private System.Windows.Forms.TextBox distanceText;
        private System.Windows.Forms.Label dxLabel;
        private System.Windows.Forms.TextBox xText;
        private System.Windows.Forms.Label cmLabelX;
        private System.Windows.Forms.Label cmLabelY;
        private System.Windows.Forms.TextBox yText;
        private System.Windows.Forms.Label dyLabel;
        private System.Windows.Forms.Label cmLabelZ;
        private System.Windows.Forms.TextBox zText;
        private System.Windows.Forms.Label dzLabel;
        private System.Windows.Forms.GroupBox directionPanel;
        private System.Windows.Forms.Button dirFacetCenterButton;
        private System.Windows.Forms.Button dirVertexButton;
        private System.Windows.Forms.Button baseFacetCenterButton;
        private System.Windows.Forms.Button baseVertexButton;
        private System.Windows.Forms.Button facetNormalButton;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label directionStatusLabel;
        private System.Windows.Forms.Label baseStatusLabel;
        private System.Windows.Forms.Button copyButton;
        private System.Windows.Forms.Button moveButton;
        private System.Windows.Forms.GroupBox dirPanel;
        private System.Windows.Forms.GroupBox basePanel;
    }
}