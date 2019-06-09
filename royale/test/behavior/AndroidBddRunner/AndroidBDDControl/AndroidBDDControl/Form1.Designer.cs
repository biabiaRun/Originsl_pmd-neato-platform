namespace AndroidBDDControl
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose (bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose (disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.btnCheckAdb = new System.Windows.Forms.Button();
            this.btnStartApp = new System.Windows.Forms.Button();
            this.btnResult = new System.Windows.Forms.Button();
            this.btnEnableTCP = new System.Windows.Forms.Button();
            this.textBoxLog = new System.Windows.Forms.TextBox();
            this.btnConnect = new System.Windows.Forms.Button();
            this.textBoxIP = new System.Windows.Forms.TextBox();
            this.labelIP = new System.Windows.Forms.Label();
            this.SuspendLayout();
            //
            // btnCheckAdb
            //
            this.btnCheckAdb.Location = new System.Drawing.Point (12, 55);
            this.btnCheckAdb.Name = "btnCheckAdb";
            this.btnCheckAdb.Size = new System.Drawing.Size (116, 23);
            this.btnCheckAdb.TabIndex = 1;
            this.btnCheckAdb.Text = "Check ADB";
            this.btnCheckAdb.UseVisualStyleBackColor = true;
            this.btnCheckAdb.Click += new System.EventHandler (this.btnCheckAdb_Click);
            //
            // btnStartApp
            //
            this.btnStartApp.Location = new System.Drawing.Point (12, 142);
            this.btnStartApp.Name = "btnStartApp";
            this.btnStartApp.Size = new System.Drawing.Size (116, 23);
            this.btnStartApp.TabIndex = 2;
            this.btnStartApp.Text = "Start Test";
            this.btnStartApp.UseVisualStyleBackColor = true;
            this.btnStartApp.Click += new System.EventHandler (this.btnStartApp_Click);
            //
            // btnResult
            //
            this.btnResult.Location = new System.Drawing.Point (12, 171);
            this.btnResult.Name = "btnResult";
            this.btnResult.Size = new System.Drawing.Size (116, 23);
            this.btnResult.TabIndex = 2;
            this.btnResult.Text = "Fetch Result";
            this.btnResult.UseVisualStyleBackColor = true;
            this.btnResult.Click += new System.EventHandler (this.btnResult_Click);
            //
            // btnEnableTCP
            //
            this.btnEnableTCP.Location = new System.Drawing.Point (12, 84);
            this.btnEnableTCP.Name = "btnEnableTCP";
            this.btnEnableTCP.Size = new System.Drawing.Size (116, 23);
            this.btnEnableTCP.TabIndex = 2;
            this.btnEnableTCP.Text = "Enable TCP/IP";
            this.btnEnableTCP.UseVisualStyleBackColor = true;
            this.btnEnableTCP.Click += new System.EventHandler (this.btnEnableTCP_Click);
            //
            // textBoxLog
            //
            this.textBoxLog.Anchor = ( (System.Windows.Forms.AnchorStyles) ( ( ( (System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                                       | System.Windows.Forms.AnchorStyles.Left)
                                       | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxLog.Location = new System.Drawing.Point (134, 12);
            this.textBoxLog.Multiline = true;
            this.textBoxLog.Name = "textBoxLog";
            this.textBoxLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBoxLog.Size = new System.Drawing.Size (705, 518);
            this.textBoxLog.TabIndex = 3;
            //
            // btnConnect
            //
            this.btnConnect.Location = new System.Drawing.Point (12, 113);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size (116, 23);
            this.btnConnect.TabIndex = 2;
            this.btnConnect.Text = "Connect TCP/IP";
            this.btnConnect.UseVisualStyleBackColor = true;
            this.btnConnect.Click += new System.EventHandler (this.btnConnect_Click);
            //
            // textBoxIP
            //
            this.textBoxIP.Location = new System.Drawing.Point (12, 29);
            this.textBoxIP.Name = "textBoxIP";
            this.textBoxIP.Size = new System.Drawing.Size (116, 20);
            this.textBoxIP.TabIndex = 4;
            this.textBoxIP.Text = "192.168.0.100";
            //
            // labelIP
            //
            this.labelIP.AutoSize = true;
            this.labelIP.Location = new System.Drawing.Point (13, 13);
            this.labelIP.Name = "labelIP";
            this.labelIP.Size = new System.Drawing.Size (93, 13);
            this.labelIP.TabIndex = 5;
            this.labelIP.Text = "Android Device IP";
            //
            // Form1
            //
            this.AutoScaleDimensions = new System.Drawing.SizeF (6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size (851, 542);
            this.Controls.Add (this.labelIP);
            this.Controls.Add (this.textBoxIP);
            this.Controls.Add (this.textBoxLog);
            this.Controls.Add (this.btnConnect);
            this.Controls.Add (this.btnEnableTCP);
            this.Controls.Add (this.btnResult);
            this.Controls.Add (this.btnStartApp);
            this.Controls.Add (this.btnCheckAdb);
            this.Name = "Form1";
            this.Text = "Android BDD Control";
            this.ResumeLayout (false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Button btnCheckAdb;
        private System.Windows.Forms.Button btnStartApp;
        private System.Windows.Forms.Button btnResult;
        private System.Windows.Forms.Button btnEnableTCP;
        private System.Windows.Forms.TextBox textBoxLog;
        private System.Windows.Forms.Button btnConnect;
        private System.Windows.Forms.TextBox textBoxIP;
        private System.Windows.Forms.Label labelIP;
    }
}

