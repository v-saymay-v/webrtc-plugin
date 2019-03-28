using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Forms;

namespace Broadcast
{
    public partial class FormLogin : Form
    {
        private delegate void MyEventHandler(EventArgs e);

        private Signaling hbLogin;

        public FormLogin(Signaling login)
        {
            hbLogin = login;
            InitializeComponent();
            if (Signaling.settings.BizAccessUserPhoto != null)
            {
                Uri u = new Uri(Signaling.settings.BizAccessUserPhoto);
                this.serverName.Text = u.Host;
            }
            if (Signaling.settings.BizAccessRoomId != null)
            {
                this.hotbizName.Text = Signaling.settings.BizAccessRoomId;
            }
        }

        private void EventCloseWindow(EventArgs evt)
        {
            this.Close();
        }

        private async void Login_Click(object sender, EventArgs e)
        {
            try
            {
                string serverName = this.serverName.Text;
                string hotbizName = this.hotbizName.Text;
                string userId = this.hotbizUserId.Text;
                string password = this.hotbizPass.Text;
                await hbLogin.LoginBizAccessAsync(serverName, hotbizName, userId, password);
                Control control = (Control)this;
                control.Invoke(new MyEventHandler(EventCloseWindow), new EventArgs());
            }
            catch (Exception exp)
            {
                System.Windows.MessageBox.Show("システムエラー：" + exp.ToString(),
                    "エラー",
                    MessageBoxButton.OK,
                    MessageBoxImage.Error);
            }
        }

        private void Cancel_Click(object sender, EventArgs e)
        {
#if DEBUG
            System.Diagnostics.Trace.WriteLine("Exit bye FormLogin.Cancel_Click");
#endif
            System.Windows.Forms.Application.Exit();
        }
    }
}
