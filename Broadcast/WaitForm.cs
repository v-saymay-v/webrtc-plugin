using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace Broadcast
{
    public partial class WaitForm : Form
    {
        public bool bCloseToExit = true;

        public WaitForm()
        {
            InitializeComponent();
        }

        private void cancelButton_Click(object sender, EventArgs e)
        {
#if DEBUG
            System.Diagnostics.Trace.WriteLine("Exit bye WaitForm.cancelButton_Click");
#endif
            System.Windows.Forms.Application.Exit();
        }

        private void WaitForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (bCloseToExit)
            {
#if DEBUG
                System.Diagnostics.Trace.WriteLine("Exit bye WaitForm.WaitForm_FormClosed");
#endif
                System.Windows.Forms.Application.Exit();
            }
        }
    }
}
