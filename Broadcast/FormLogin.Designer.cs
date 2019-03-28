using System.Windows;

namespace Broadcast
{
    partial class FormLogin
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
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.serverName = new System.Windows.Forms.TextBox();
            this.hotbizName = new System.Windows.Forms.TextBox();
            this.hotbizUserId = new System.Windows.Forms.TextBox();
            this.hotbizPass = new System.Windows.Forms.TextBox();
            this.loginButton = new System.Windows.Forms.Button();
            this.cancelButton = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = global::Broadcast.Properties.Resources.logo_login;
            this.pictureBox1.InitialImage = global::Broadcast.Properties.Resources.logo_login;
            this.pictureBox1.Location = new System.Drawing.Point(320, 28);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(196, 53);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(293, 104);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(79, 12);
            this.label1.TabIndex = 1;
            this.label1.Text = "HotBizサーバー";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(293, 130);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(51, 12);
            this.label2.TabIndex = 2;
            this.label2.Text = "HotBiz名";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(293, 156);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(90, 12);
            this.label3.TabIndex = 3;
            this.label3.Text = "HotBizユーザーID";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(293, 182);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(86, 12);
            this.label4.TabIndex = 4;
            this.label4.Text = "HotBizパスワード";
            // 
            // serverName
            // 
            this.serverName.Location = new System.Drawing.Point(448, 104);
            this.serverName.Name = "serverName";
            this.serverName.Size = new System.Drawing.Size(100, 19);
            this.serverName.TabIndex = 5;
            // 
            // hotbizName
            // 
            this.hotbizName.Location = new System.Drawing.Point(448, 130);
            this.hotbizName.Name = "hotbizName";
            this.hotbizName.Size = new System.Drawing.Size(100, 19);
            this.hotbizName.TabIndex = 6;
            // 
            // hotbizUserId
            // 
            this.hotbizUserId.Location = new System.Drawing.Point(448, 156);
            this.hotbizUserId.Name = "hotbizUserId";
            this.hotbizUserId.Size = new System.Drawing.Size(100, 19);
            this.hotbizUserId.TabIndex = 7;
            // 
            // hotbizPass
            // 
            this.hotbizPass.Location = new System.Drawing.Point(448, 182);
            this.hotbizPass.Name = "hotbizPass";
            this.hotbizPass.PasswordChar = '*';
            this.hotbizPass.Size = new System.Drawing.Size(100, 19);
            this.hotbizPass.TabIndex = 8;
            // 
            // loginButton
            // 
            this.loginButton.Location = new System.Drawing.Point(308, 248);
            this.loginButton.Name = "loginButton";
            this.loginButton.Size = new System.Drawing.Size(75, 23);
            this.loginButton.TabIndex = 9;
            this.loginButton.Text = "ログイン";
            this.loginButton.UseVisualStyleBackColor = true;
            this.loginButton.Click += new System.EventHandler(this.Login_Click);
            // 
            // cancelButton
            // 
            this.cancelButton.Location = new System.Drawing.Point(426, 248);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 10;
            this.cancelButton.Text = "キャンセル";
            this.cancelButton.UseVisualStyleBackColor = true;
            this.cancelButton.Click += new System.EventHandler(this.Cancel_Click);
            // 
            // FormLogin
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 320);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.loginButton);
            this.Controls.Add(this.hotbizPass);
            this.Controls.Add(this.hotbizUserId);
            this.Controls.Add(this.hotbizName);
            this.Controls.Add(this.serverName);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.pictureBox1);
            this.Name = "FormLogin";
            this.Text = "FormLogin";
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

            //System.Diagnostics.Trace.WriteLine("SelectScreen.Designer InitializeComponent end");
        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox serverName;
        private System.Windows.Forms.TextBox hotbizName;
        private System.Windows.Forms.TextBox hotbizUserId;
        private System.Windows.Forms.TextBox hotbizPass;
        private System.Windows.Forms.Button loginButton;
        private System.Windows.Forms.Button cancelButton;
    }
}