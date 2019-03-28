using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Launcher
{
    public partial class Form1 : Form
    {
        private string startArg;

        public Form1(string arg)
        {
            startArg = arg;
            InitializeComponent();
        }

        private void Form1_Shown(object sender, EventArgs e)
        {
            System.Diagnostics.Process hProcess = new System.Diagnostics.Process();
            hProcess.StartInfo.FileName = @".\Broadcast.exe";
            hProcess.StartInfo.Arguments = startArg;
            hProcess.Start();
            hProcess.EnableRaisingEvents = true;
            hProcess.Exited += new System.EventHandler(Notepad_Exited);
            this.Hide();

            /*
            // 実行中のすべてのプロセスを取得する
            System.Diagnostics.Process[] hProcesses = System.Diagnostics.Process.GetProcesses();

            // 取得できたプロセスからプロセス名を取得する
            foreach (System.Diagnostics.Process hProcess in hProcesses)
            {
                if (hProcess.ProcessName == "dwm")
                {
                    hProcess.StartInfo.FileName = @".\Broadcast.exe";
                    hProcess.StartInfo.Arguments = startArg;
                    hProcess.Start();
                    this.Hide();
                    break;
                }
            }
            */
        }

        private void Notepad_Exited(object sender, System.EventArgs e)
        {
            System.Windows.Forms.Application.Exit();
        }
    }
}
