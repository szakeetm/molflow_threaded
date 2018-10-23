namespace MolflowElements
{
    partial class CreateShape
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
            this.shapePanel = new System.Windows.Forms.GroupBox();
            this.placeHolder = new System.Windows.Forms.Panel();
            this.racetrackCheckbox = new System.Windows.Forms.CheckBox();
            this.ellipseCheckbox = new System.Windows.Forms.CheckBox();
            this.rectangleCheckbox = new System.Windows.Forms.CheckBox();
            this.positionPanel = new System.Windows.Forms.GroupBox();
            this.normalStatusLabel = new System.Windows.Forms.Label();
            this.normalVertexButton = new System.Windows.Forms.Button();
            this.facetNormalButton = new System.Windows.Forms.Button();
            this.axisStatusLabel = new System.Windows.Forms.Label();
            this.axisVertexButton = new System.Windows.Forms.Button();
            this.axisFacetUButton = new System.Windows.Forms.Button();
            this.centerStatusLabel = new System.Windows.Forms.Label();
            this.centerVertexButton = new System.Windows.Forms.Button();
            this.facetCenterButton = new System.Windows.Forms.Button();
            this.normalZtext = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.normalYtext = new System.Windows.Forms.TextBox();
            this.label10 = new System.Windows.Forms.Label();
            this.normalXtext = new System.Windows.Forms.TextBox();
            this.label11 = new System.Windows.Forms.Label();
            this.label12 = new System.Windows.Forms.Label();
            this.axisZtext = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.axisYtext = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.axisXtext = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.centerZtext = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.centerYtext = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.centerXtext = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.sizePanel = new System.Windows.Forms.GroupBox();
            this.fullCircleButton = new System.Windows.Forms.Button();
            this.nbstepsText = new System.Windows.Forms.TextBox();
            this.label22 = new System.Windows.Forms.Label();
            this.label20 = new System.Windows.Forms.Label();
            this.racetrackToplengthText = new System.Windows.Forms.TextBox();
            this.label21 = new System.Windows.Forms.Label();
            this.label18 = new System.Windows.Forms.Label();
            this.axis2LengthText = new System.Windows.Forms.TextBox();
            this.label19 = new System.Windows.Forms.Label();
            this.label17 = new System.Windows.Forms.Label();
            this.axis1LengthText = new System.Windows.Forms.TextBox();
            this.label16 = new System.Windows.Forms.Label();
            this.createButton = new System.Windows.Forms.Button();
            this.shapePanel.SuspendLayout();
            this.positionPanel.SuspendLayout();
            this.sizePanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // shapePanel
            // 
            this.shapePanel.Controls.Add(this.placeHolder);
            this.shapePanel.Controls.Add(this.racetrackCheckbox);
            this.shapePanel.Controls.Add(this.ellipseCheckbox);
            this.shapePanel.Controls.Add(this.rectangleCheckbox);
            this.shapePanel.Location = new System.Drawing.Point(8, 3);
            this.shapePanel.Name = "shapePanel";
            this.shapePanel.Size = new System.Drawing.Size(694, 247);
            this.shapePanel.TabIndex = 0;
            this.shapePanel.TabStop = false;
            this.shapePanel.Text = "Shape";
            // 
            // placeHolder
            // 
            this.placeHolder.Location = new System.Drawing.Point(94, 43);
            this.placeHolder.Name = "placeHolder";
            this.placeHolder.Size = new System.Drawing.Size(500, 190);
            this.placeHolder.TabIndex = 3;
            this.placeHolder.Paint += new System.Windows.Forms.PaintEventHandler(this.placeHolder_Paint);
            // 
            // racetrackCheckbox
            // 
            this.racetrackCheckbox.AutoSize = true;
            this.racetrackCheckbox.Location = new System.Drawing.Point(424, 19);
            this.racetrackCheckbox.Name = "racetrackCheckbox";
            this.racetrackCheckbox.Size = new System.Drawing.Size(76, 17);
            this.racetrackCheckbox.TabIndex = 2;
            this.racetrackCheckbox.Text = "Racetrack";
            this.racetrackCheckbox.UseVisualStyleBackColor = true;
            this.racetrackCheckbox.CheckedChanged += new System.EventHandler(this.racetrackCheckbox_CheckedChanged);
            // 
            // ellipseCheckbox
            // 
            this.ellipseCheckbox.AutoSize = true;
            this.ellipseCheckbox.Location = new System.Drawing.Point(308, 19);
            this.ellipseCheckbox.Name = "ellipseCheckbox";
            this.ellipseCheckbox.Size = new System.Drawing.Size(93, 17);
            this.ellipseCheckbox.TabIndex = 1;
            this.ellipseCheckbox.Text = "Circle / Ellipse";
            this.ellipseCheckbox.UseVisualStyleBackColor = true;
            this.ellipseCheckbox.CheckedChanged += new System.EventHandler(this.ellipseCheckbox_CheckedChanged);
            // 
            // rectangleCheckbox
            // 
            this.rectangleCheckbox.AutoSize = true;
            this.rectangleCheckbox.Location = new System.Drawing.Point(182, 19);
            this.rectangleCheckbox.Name = "rectangleCheckbox";
            this.rectangleCheckbox.Size = new System.Drawing.Size(120, 17);
            this.rectangleCheckbox.TabIndex = 0;
            this.rectangleCheckbox.Text = "Square / Rectangle";
            this.rectangleCheckbox.UseVisualStyleBackColor = true;
            this.rectangleCheckbox.CheckedChanged += new System.EventHandler(this.rectangleCheckbox_CheckedChanged);
            // 
            // positionPanel
            // 
            this.positionPanel.Controls.Add(this.normalStatusLabel);
            this.positionPanel.Controls.Add(this.normalVertexButton);
            this.positionPanel.Controls.Add(this.facetNormalButton);
            this.positionPanel.Controls.Add(this.axisStatusLabel);
            this.positionPanel.Controls.Add(this.centerVertexButton);
            this.positionPanel.Controls.Add(this.axisVertexButton);
            this.positionPanel.Controls.Add(this.axisFacetUButton);
            this.positionPanel.Controls.Add(this.centerStatusLabel);
            this.positionPanel.Controls.Add(this.facetCenterButton);
            this.positionPanel.Controls.Add(this.normalZtext);
            this.positionPanel.Controls.Add(this.label9);
            this.positionPanel.Controls.Add(this.normalYtext);
            this.positionPanel.Controls.Add(this.label10);
            this.positionPanel.Controls.Add(this.normalXtext);
            this.positionPanel.Controls.Add(this.label11);
            this.positionPanel.Controls.Add(this.label12);
            this.positionPanel.Controls.Add(this.axisZtext);
            this.positionPanel.Controls.Add(this.label5);
            this.positionPanel.Controls.Add(this.axisYtext);
            this.positionPanel.Controls.Add(this.label6);
            this.positionPanel.Controls.Add(this.axisXtext);
            this.positionPanel.Controls.Add(this.label7);
            this.positionPanel.Controls.Add(this.label8);
            this.positionPanel.Controls.Add(this.centerZtext);
            this.positionPanel.Controls.Add(this.label4);
            this.positionPanel.Controls.Add(this.centerYtext);
            this.positionPanel.Controls.Add(this.label3);
            this.positionPanel.Controls.Add(this.centerXtext);
            this.positionPanel.Controls.Add(this.label2);
            this.positionPanel.Controls.Add(this.label1);
            this.positionPanel.Location = new System.Drawing.Point(8, 256);
            this.positionPanel.Name = "positionPanel";
            this.positionPanel.Size = new System.Drawing.Size(694, 94);
            this.positionPanel.TabIndex = 1;
            this.positionPanel.TabStop = false;
            this.positionPanel.Text = "Position";
            // 
            // normalStatusLabel
            // 
            this.normalStatusLabel.AutoSize = true;
            this.normalStatusLabel.Location = new System.Drawing.Point(594, 68);
            this.normalStatusLabel.Name = "normalStatusLabel";
            this.normalStatusLabel.Size = new System.Drawing.Size(35, 13);
            this.normalStatusLabel.TabIndex = 29;
            this.normalStatusLabel.Text = "status";
            // 
            // normalVertexButton
            // 
            this.normalVertexButton.Location = new System.Drawing.Point(506, 65);
            this.normalVertexButton.Name = "normalVertexButton";
            this.normalVertexButton.Size = new System.Drawing.Size(85, 20);
            this.normalVertexButton.TabIndex = 28;
            this.normalVertexButton.Text = "Center to vertex";
            this.normalVertexButton.UseVisualStyleBackColor = true;
            // 
            // facetNormalButton
            // 
            this.facetNormalButton.Location = new System.Drawing.Point(425, 65);
            this.facetNormalButton.Name = "facetNormalButton";
            this.facetNormalButton.Size = new System.Drawing.Size(75, 20);
            this.facetNormalButton.TabIndex = 27;
            this.facetNormalButton.Text = "Facet N";
            this.facetNormalButton.UseVisualStyleBackColor = true;
            // 
            // axisStatusLabel
            // 
            this.axisStatusLabel.AutoSize = true;
            this.axisStatusLabel.Location = new System.Drawing.Point(594, 42);
            this.axisStatusLabel.Name = "axisStatusLabel";
            this.axisStatusLabel.Size = new System.Drawing.Size(35, 13);
            this.axisStatusLabel.TabIndex = 26;
            this.axisStatusLabel.Text = "status";
            // 
            // axisVertexButton
            // 
            this.axisVertexButton.Location = new System.Drawing.Point(506, 39);
            this.axisVertexButton.Name = "axisVertexButton";
            this.axisVertexButton.Size = new System.Drawing.Size(85, 20);
            this.axisVertexButton.TabIndex = 25;
            this.axisVertexButton.Text = "Center to vertex";
            this.axisVertexButton.UseVisualStyleBackColor = true;
            this.axisVertexButton.Click += new System.EventHandler(this.axisVertexButton_Click);
            // 
            // axisFacetUButton
            // 
            this.axisFacetUButton.Location = new System.Drawing.Point(425, 39);
            this.axisFacetUButton.Name = "axisFacetUButton";
            this.axisFacetUButton.Size = new System.Drawing.Size(75, 20);
            this.axisFacetUButton.TabIndex = 24;
            this.axisFacetUButton.Text = "Facet U";
            this.axisFacetUButton.UseVisualStyleBackColor = true;
            // 
            // centerStatusLabel
            // 
            this.centerStatusLabel.AutoSize = true;
            this.centerStatusLabel.Location = new System.Drawing.Point(594, 16);
            this.centerStatusLabel.Name = "centerStatusLabel";
            this.centerStatusLabel.Size = new System.Drawing.Size(35, 13);
            this.centerStatusLabel.TabIndex = 23;
            this.centerStatusLabel.Text = "status";
            // 
            // centerVertexButton
            // 
            this.centerVertexButton.Location = new System.Drawing.Point(506, 13);
            this.centerVertexButton.Name = "centerVertexButton";
            this.centerVertexButton.Size = new System.Drawing.Size(85, 20);
            this.centerVertexButton.TabIndex = 22;
            this.centerVertexButton.Text = "Vertex";
            this.centerVertexButton.UseVisualStyleBackColor = true;
            // 
            // facetCenterButton
            // 
            this.facetCenterButton.Location = new System.Drawing.Point(425, 13);
            this.facetCenterButton.Name = "facetCenterButton";
            this.facetCenterButton.Size = new System.Drawing.Size(75, 20);
            this.facetCenterButton.TabIndex = 21;
            this.facetCenterButton.Text = "Facet center";
            this.facetCenterButton.UseVisualStyleBackColor = true;
            // 
            // normalZtext
            // 
            this.normalZtext.Location = new System.Drawing.Point(339, 65);
            this.normalZtext.Name = "normalZtext";
            this.normalZtext.Size = new System.Drawing.Size(80, 20);
            this.normalZtext.TabIndex = 20;
            this.normalZtext.Text = "1";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(322, 68);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(17, 13);
            this.label9.TabIndex = 19;
            this.label9.Text = "Z:";
            // 
            // normalYtext
            // 
            this.normalYtext.Location = new System.Drawing.Point(232, 65);
            this.normalYtext.Name = "normalYtext";
            this.normalYtext.Size = new System.Drawing.Size(80, 20);
            this.normalYtext.TabIndex = 18;
            this.normalYtext.Text = "0";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(215, 68);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(17, 13);
            this.label10.TabIndex = 17;
            this.label10.Text = "Y:";
            // 
            // normalXtext
            // 
            this.normalXtext.Location = new System.Drawing.Point(128, 65);
            this.normalXtext.Name = "normalXtext";
            this.normalXtext.Size = new System.Drawing.Size(80, 20);
            this.normalXtext.TabIndex = 16;
            this.normalXtext.Text = "0";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(111, 68);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(17, 13);
            this.label11.TabIndex = 15;
            this.label11.Text = "X:";
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(6, 68);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(86, 13);
            this.label12.TabIndex = 14;
            this.label12.Text = "Normal direction:";
            // 
            // axisZtext
            // 
            this.axisZtext.Location = new System.Drawing.Point(339, 40);
            this.axisZtext.Name = "axisZtext";
            this.axisZtext.Size = new System.Drawing.Size(80, 20);
            this.axisZtext.TabIndex = 13;
            this.axisZtext.Text = "0";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(322, 42);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(17, 13);
            this.label5.TabIndex = 12;
            this.label5.Text = "Z:";
            // 
            // axisYtext
            // 
            this.axisYtext.Location = new System.Drawing.Point(232, 39);
            this.axisYtext.Name = "axisYtext";
            this.axisYtext.Size = new System.Drawing.Size(80, 20);
            this.axisYtext.TabIndex = 11;
            this.axisYtext.Text = "0";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(215, 42);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(17, 13);
            this.label6.TabIndex = 10;
            this.label6.Text = "Y:";
            // 
            // axisXtext
            // 
            this.axisXtext.Location = new System.Drawing.Point(128, 39);
            this.axisXtext.Name = "axisXtext";
            this.axisXtext.Size = new System.Drawing.Size(80, 20);
            this.axisXtext.TabIndex = 9;
            this.axisXtext.Text = "1";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(111, 42);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(17, 13);
            this.label7.TabIndex = 8;
            this.label7.Text = "X:";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(6, 42);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(78, 13);
            this.label8.TabIndex = 7;
            this.label8.Text = "Axis1 direction:";
            // 
            // centerZtext
            // 
            this.centerZtext.Location = new System.Drawing.Point(339, 13);
            this.centerZtext.Name = "centerZtext";
            this.centerZtext.Size = new System.Drawing.Size(80, 20);
            this.centerZtext.TabIndex = 6;
            this.centerZtext.Text = "0";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(322, 16);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(17, 13);
            this.label4.TabIndex = 5;
            this.label4.Text = "Z:";
            // 
            // centerYtext
            // 
            this.centerYtext.Location = new System.Drawing.Point(232, 13);
            this.centerYtext.Name = "centerYtext";
            this.centerYtext.Size = new System.Drawing.Size(80, 20);
            this.centerYtext.TabIndex = 4;
            this.centerYtext.Text = "0";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(215, 16);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(17, 13);
            this.label3.TabIndex = 3;
            this.label3.Text = "Y:";
            // 
            // centerXtext
            // 
            this.centerXtext.Location = new System.Drawing.Point(128, 13);
            this.centerXtext.Name = "centerXtext";
            this.centerXtext.Size = new System.Drawing.Size(80, 20);
            this.centerXtext.TabIndex = 2;
            this.centerXtext.Text = "0";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(111, 16);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(17, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "X:";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 16);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(41, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Center:";
            // 
            // sizePanel
            // 
            this.sizePanel.Controls.Add(this.fullCircleButton);
            this.sizePanel.Controls.Add(this.nbstepsText);
            this.sizePanel.Controls.Add(this.label22);
            this.sizePanel.Controls.Add(this.label20);
            this.sizePanel.Controls.Add(this.racetrackToplengthText);
            this.sizePanel.Controls.Add(this.label21);
            this.sizePanel.Controls.Add(this.label18);
            this.sizePanel.Controls.Add(this.axis2LengthText);
            this.sizePanel.Controls.Add(this.label19);
            this.sizePanel.Controls.Add(this.label17);
            this.sizePanel.Controls.Add(this.axis1LengthText);
            this.sizePanel.Controls.Add(this.label16);
            this.sizePanel.Location = new System.Drawing.Point(8, 356);
            this.sizePanel.Name = "sizePanel";
            this.sizePanel.Size = new System.Drawing.Size(694, 66);
            this.sizePanel.TabIndex = 1;
            this.sizePanel.TabStop = false;
            this.sizePanel.Text = "Size";
            // 
            // fullCircleButton
            // 
            this.fullCircleButton.Location = new System.Drawing.Point(449, 13);
            this.fullCircleButton.Name = "fullCircleButton";
            this.fullCircleButton.Size = new System.Drawing.Size(142, 20);
            this.fullCircleButton.TabIndex = 30;
            this.fullCircleButton.Text = "Full circle sides";
            this.fullCircleButton.UseVisualStyleBackColor = true;
            // 
            // nbstepsText
            // 
            this.nbstepsText.Location = new System.Drawing.Point(339, 39);
            this.nbstepsText.Name = "nbstepsText";
            this.nbstepsText.Size = new System.Drawing.Size(80, 20);
            this.nbstepsText.TabIndex = 39;
            this.nbstepsText.Text = "10";
            // 
            // label22
            // 
            this.label22.AutoSize = true;
            this.label22.Location = new System.Drawing.Point(229, 42);
            this.label22.Name = "label22";
            this.label22.Size = new System.Drawing.Size(66, 13);
            this.label22.TabIndex = 38;
            this.label22.Text = "Steps in arc:";
            // 
            // label20
            // 
            this.label20.AutoSize = true;
            this.label20.Location = new System.Drawing.Point(422, 16);
            this.label20.Name = "label20";
            this.label20.Size = new System.Drawing.Size(21, 13);
            this.label20.TabIndex = 35;
            this.label20.Text = "cm";
            // 
            // racetrackToplengthText
            // 
            this.racetrackToplengthText.Location = new System.Drawing.Point(339, 13);
            this.racetrackToplengthText.Name = "racetrackToplengthText";
            this.racetrackToplengthText.Size = new System.Drawing.Size(80, 20);
            this.racetrackToplengthText.TabIndex = 37;
            // 
            // label21
            // 
            this.label21.AutoSize = true;
            this.label21.Location = new System.Drawing.Point(229, 16);
            this.label21.Name = "label21";
            this.label21.Size = new System.Drawing.Size(110, 13);
            this.label21.TabIndex = 36;
            this.label21.Text = "Racetrack top length:";
            // 
            // label18
            // 
            this.label18.AutoSize = true;
            this.label18.Location = new System.Drawing.Point(161, 42);
            this.label18.Name = "label18";
            this.label18.Size = new System.Drawing.Size(21, 13);
            this.label18.TabIndex = 32;
            this.label18.Text = "cm";
            // 
            // axis2LengthText
            // 
            this.axis2LengthText.Location = new System.Drawing.Point(79, 39);
            this.axis2LengthText.Name = "axis2LengthText";
            this.axis2LengthText.Size = new System.Drawing.Size(80, 20);
            this.axis2LengthText.TabIndex = 34;
            this.axis2LengthText.Text = "1";
            // 
            // label19
            // 
            this.label19.AutoSize = true;
            this.label19.Location = new System.Drawing.Point(6, 42);
            this.label19.Name = "label19";
            this.label19.Size = new System.Drawing.Size(67, 13);
            this.label19.TabIndex = 33;
            this.label19.Text = "Axis2 length:";
            // 
            // label17
            // 
            this.label17.AutoSize = true;
            this.label17.Location = new System.Drawing.Point(161, 16);
            this.label17.Name = "label17";
            this.label17.Size = new System.Drawing.Size(21, 13);
            this.label17.TabIndex = 30;
            this.label17.Text = "cm";
            // 
            // axis1LengthText
            // 
            this.axis1LengthText.Location = new System.Drawing.Point(79, 13);
            this.axis1LengthText.Name = "axis1LengthText";
            this.axis1LengthText.Size = new System.Drawing.Size(80, 20);
            this.axis1LengthText.TabIndex = 31;
            this.axis1LengthText.Text = "1";
            // 
            // label16
            // 
            this.label16.AutoSize = true;
            this.label16.Location = new System.Drawing.Point(6, 16);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(67, 13);
            this.label16.TabIndex = 30;
            this.label16.Text = "Axis1 length:";
            // 
            // createButton
            // 
            this.createButton.Location = new System.Drawing.Point(327, 431);
            this.createButton.Name = "createButton";
            this.createButton.Size = new System.Drawing.Size(100, 20);
            this.createButton.TabIndex = 38;
            this.createButton.Text = "Create facet";
            this.createButton.UseVisualStyleBackColor = true;
            // 
            // CreateShape
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(698, 463);
            this.Controls.Add(this.createButton);
            this.Controls.Add(this.sizePanel);
            this.Controls.Add(this.positionPanel);
            this.Controls.Add(this.shapePanel);
            this.Name = "CreateShape";
            this.Text = "Create shape";
            this.shapePanel.ResumeLayout(false);
            this.shapePanel.PerformLayout();
            this.positionPanel.ResumeLayout(false);
            this.positionPanel.PerformLayout();
            this.sizePanel.ResumeLayout(false);
            this.sizePanel.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox shapePanel;
        private System.Windows.Forms.CheckBox racetrackCheckbox;
        private System.Windows.Forms.CheckBox ellipseCheckbox;
        private System.Windows.Forms.CheckBox rectangleCheckbox;
        private System.Windows.Forms.GroupBox positionPanel;
        private System.Windows.Forms.Label normalStatusLabel;
        private System.Windows.Forms.Button normalVertexButton;
        private System.Windows.Forms.Button facetNormalButton;
        private System.Windows.Forms.Label axisStatusLabel;
        private System.Windows.Forms.Button axisVertexButton;
        private System.Windows.Forms.Button axisFacetUButton;
        private System.Windows.Forms.Label centerStatusLabel;
        private System.Windows.Forms.Button centerVertexButton;
        private System.Windows.Forms.Button facetCenterButton;
        private System.Windows.Forms.TextBox normalZtext;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.TextBox normalYtext;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.TextBox normalXtext;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.TextBox axisZtext;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox axisYtext;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox axisXtext;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TextBox centerZtext;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox centerYtext;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox centerXtext;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox sizePanel;
        private System.Windows.Forms.Button fullCircleButton;
        private System.Windows.Forms.TextBox nbstepsText;
        private System.Windows.Forms.Label label22;
        private System.Windows.Forms.Label label20;
        private System.Windows.Forms.TextBox racetrackToplengthText;
        private System.Windows.Forms.Label label21;
        private System.Windows.Forms.Label label18;
        private System.Windows.Forms.TextBox axis2LengthText;
        private System.Windows.Forms.Label label19;
        private System.Windows.Forms.Label label17;
        private System.Windows.Forms.TextBox axis1LengthText;
        private System.Windows.Forms.Label label16;
        private System.Windows.Forms.Button createButton;
        private System.Windows.Forms.Panel placeHolder;
    }
}