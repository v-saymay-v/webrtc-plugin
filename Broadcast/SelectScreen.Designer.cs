using System;
using System.Drawing;
using System.Windows.Forms;
using System.Collections.Generic;

namespace Broadcast
{
    partial class SelectScreen
    {
        /// <summary>
        /// 必要なデザイナー変数です。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 使用中のリソースをすべてクリーンアップします。
        /// </summary>
        /// <param name="disposing">マネージド リソースを破棄する場合は true を指定し、その他の場合は false を指定します。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows フォーム デザイナーで生成されたコード

        /// <summary>
        /// デザイナー サポートに必要なメソッドです。このメソッドの内容を
        /// コード エディターで変更しないでください。
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SelectScreen));
            this.mShareButton = new System.Windows.Forms.Button();
            this.mCancelButton = new System.Windows.Forms.Button();
            this.mWindowList = new System.Windows.Forms.ListView();
            this.windowThumbs = new System.Windows.Forms.ImageList(this.components);
            this.SuspendLayout();
            // 
            // mShareButton
            // 
            resources.ApplyResources(this.mShareButton, "mShareButton");
            this.mShareButton.Name = "mShareButton";
            this.mShareButton.UseVisualStyleBackColor = true;
            this.mShareButton.Click += new System.EventHandler(this.mShareButton_Click);
            // 
            // mCancelButton
            // 
            resources.ApplyResources(this.mCancelButton, "mCancelButton");
            this.mCancelButton.Name = "mCancelButton";
            this.mCancelButton.UseVisualStyleBackColor = true;
            this.mCancelButton.Click += new System.EventHandler(this.mCancelButton_Click);
            // 
            // mWindowList
            // 
            resources.ApplyResources(this.mWindowList, "mWindowList");
            this.mWindowList.Name = "mWindowList";
            this.mWindowList.UseCompatibleStateImageBehavior = false;
            // 
            // windowThumbs
            // 
            this.windowThumbs.ColorDepth = System.Windows.Forms.ColorDepth.Depth8Bit;
            resources.ApplyResources(this.windowThumbs, "windowThumbs");
            this.windowThumbs.TransparentColor = System.Drawing.Color.Transparent;
            // 
            // SelectScreen
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.mWindowList);
            this.Controls.Add(this.mCancelButton);
            this.Controls.Add(this.mShareButton);
            this.Name = "SelectScreen";
            this.Shown += new System.EventHandler(this.SelectScreen_Shown);
            this.ResumeLayout(false);

        }

        #endregion

        private Button mShareButton;
        private Button mCancelButton;
        private ListView mWindowList;
        private Dictionary<IntPtr, Bitmap> mapHwndBitmap;
        private ImageList windowThumbs;
        private ColumnHeader columnName;
    }
}
