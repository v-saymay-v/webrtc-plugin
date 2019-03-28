using System;
using System.Collections.Generic;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Forms;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace Broadcast
{
    public partial class SelectScreen : Form
    {
        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

        [DllImport("User32.Dll")]
        private static extern IntPtr GetDesktopWindow();

        private const string dllName = "rtc.dll";

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void EnumWindowsDelegate(IntPtr lparam, IntPtr hWnd, int width, int height, StringBuilder title, StringBuilder image);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void GetUserMediaSuccessCallBack(StringBuilder remoteid);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void GetUserMediaErrorCallBack(StringBuilder error);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void SendSignalinMessage(StringBuilder remoteId, StringBuilder message);

        [DllImport(dllName, EntryPoint = "GetWindowsNum", CallingConvention = CallingConvention.Cdecl)]
        private extern static int GetWindowsNum();

        [DllImport(dllName, EntryPoint = "GetWindowList", CallingConvention = CallingConvention.Cdecl)]
        private extern static void GetWindowList(IntPtr lparam, EnumWindowsDelegate lpFuncGotWindow);

        [DllImport(dllName, EntryPoint = "CreateThumbNail", CallingConvention = CallingConvention.Cdecl)]
        private extern static IntPtr CreateThumbNail(IntPtr hWnd);

        [DllImport(dllName, EntryPoint = "DllGetUserMedia", CallingConvention = CallingConvention.Cdecl)]
        private extern static void DllGetUserMedia(IntPtr hWnd, StringBuilder roomId, StringBuilder clientId, StringBuilder iceServers, StringBuilder remoteUsers,
            GetUserMediaSuccessCallBack func1, GetUserMediaErrorCallBack func2, SendSignalinMessage func3);

        [DllImport(dllName, EntryPoint = "DllGotRemoteSdp", CallingConvention = CallingConvention.Cdecl)]
        private extern static void DllGotRemoteSdp(StringBuilder sdp, StringBuilder type, StringBuilder roomid, StringBuilder clientid, StringBuilder remoteid);

        [DllImport(dllName, EntryPoint = "DllGotRemoteCandidate", CallingConvention = CallingConvention.Cdecl)]
        private extern static void DllGotRemoteCandidate(int label, StringBuilder id, StringBuilder candidate, StringBuilder roomid, StringBuilder clientid, StringBuilder remoteid);

        [DllImport(dllName, EntryPoint = "DllGotEoic", CallingConvention = CallingConvention.Cdecl)]
        private extern static void DllGotEoic(StringBuilder roomid, StringBuilder clientid, StringBuilder remoteid);

        [DllImport(dllName, EntryPoint = "DllExitApplication", CallingConvention = CallingConvention.Cdecl)]
        private extern static void DllExitApplication();

        private RegisterArgs registerArgs;
        //static private Login login;
        private Signaling websocket;
        private IntPtr watchingWnd;
        private System.Timers.Timer windowWatchTimer = null;
        private System.Timers.Timer waitTimer = null;
        private WaitForm waitForm = null;
        private JObject loginData = null;
        private bool isNeedLogin = false;
        private bool isSharingNow = false;

        [StructLayout(LayoutKind.Sequential)]
        public struct RECT
        {
            public int Left;        // x position of upper-left corner
            public int Top;         // y position of upper-left corner
            public int Right;       // x position of lower-right corner
            public int Bottom;      // y position of lower-right corner
        }

        public SelectScreen(JObject data)
        {
            InitializeComponent();
            GetBitmapForWindows();

            loginData = data;
            websocket = new Signaling(this, data);
            websocket.GotWebsocketError += new Signaling.WebsocketErrorHandler(CallBackWebsocketError);
            websocket.MyErrorEvent += new Signaling.MyLoginError(CallBackEventError);
            websocket.MyInvalidEvent += new Signaling.MyLoginInvalid(CallBackEventInvalid);
            websocket.MyLoginEvent += new Signaling.MyLoginHandler(CallBackEventLogin);
            websocket.MyCloseEvent += new Signaling.MyCloseHandler(CallBackSocketClosed);
        }

        ~SelectScreen()
        {
            if (windowWatchTimer != null)
                windowWatchTimer.Stop();
            if (waitTimer != null)
                waitTimer.Stop();
        }

        private void SelectScreen_Shown(Object sender, EventArgs e)
        {
            waitTimer = new System.Timers.Timer(1000);
            waitTimer.Elapsed += (Object source, System.Timers.ElapsedEventArgs eea) =>
            {
                Program.Wait();
            };
            waitTimer.Start();

            if (!websocket.CheckLogin())
            {
                if (loginData == null)
                {
                    FormLogin form = new FormLogin(websocket);
                    form.ShowDialog();
                    form.Dispose();
                }
                else
                {
                    //FormLogin form = new FormLogin(websocket);
                    waitForm = new WaitForm();
                    waitForm.ShowDialog();
                    waitForm.Dispose();
                }
            }
            else
            {
                websocket.LoginToRoom();
            }
        }

        private void CallBackSocketClosed(EventArgs e)
        {
            if (isNeedLogin)
            {
                FormLogin form = new FormLogin(websocket);
                form.ShowDialog();
                form.Dispose();
                isNeedLogin = false;
            }
        }

        private void CallBackEventLogin(EventArgs e)
        {
            websocket.MyRegisterEvent += new Signaling.MyRegisterHandler(CallBackEventRegister);
            websocket.RegisterRoom();
        }

        private void DispError(string title, string body)
        {
            System.Windows.MessageBox.Show(body,
                title,
                MessageBoxButton.OK,
                MessageBoxImage.Error);
        }

        private void CallBackWebsocketError(SuperSocket.ClientEngine.ErrorEventArgs error)
        {
            DispError("エラー", "エラーが発生しました：" + error.ToString());
            websocket.ResetWebSocket();
        }

        private void CallBackEventError(string error)
        {
            DispError("エラー", "ログインに失敗しました：" + error);
            websocket.ResetWebSocket();
        }

        private void CallBackEventInvalid(string error)
        {
            DispError("エラー", "ログインに失敗しました：" + error);
            isNeedLogin = true;
            websocket.ResetWebSocket();
        }

        private void mCancelButton_Click(object sender, EventArgs e)
        {
            try
            {
                DllExitApplication();
            }
            catch (Exception exp)
            {
                System.Windows.MessageBox.Show(exp.ToString(),
                    "エラー",
                    MessageBoxButton.OK,
                    MessageBoxImage.Error);
            }
            finally
            {
#if DEBUG
                System.Diagnostics.Trace.WriteLine("Exit bye SelectScreen.mCancelButton_Click");
#endif
                System.Windows.Forms.Application.Exit();
            }
        }

        private void mShareButton_Click(object sender, EventArgs e)
        {
            if (isSharingNow)
            {
                try
                {
                    DllExitApplication();
                }
                catch (Exception exp)
                {
                    System.Windows.MessageBox.Show(exp.ToString(),
                        "エラー",
                        MessageBoxButton.OK,
                        MessageBoxImage.Error);
                }
                finally
                {
#if DEBUG
                    System.Diagnostics.Trace.WriteLine("Send bye message");
#endif
                    if (windowWatchTimer != null)
                        windowWatchTimer.Stop();
                    string msg = "{\"type\":\"bye\", \"remoteid\":\"" + Signaling.settings.BizAccessUserId + "_share" + "\"}";
                    websocket.SendMessage(Signaling.settings.BizAccessRoomId, Signaling.settings.BizAccessSubId, Signaling.settings.BizAccessUserId + "_share", "0", msg);
                    this.mShareButton.Text = "共有";
                    isSharingNow = false;
                }
            }
            else if (mWindowList.SelectedItems.Count > 1)
            {
                DispError("エラー", "複数選択されています。");
            }
            else if (registerArgs == null)
            {
                DispError("エラー", "ログインされていません。");
            }
            else
            {
                IntPtr hWnd = (IntPtr)mWindowList.SelectedItems[0].Tag;
                RECT origRect = new RECT();
                GetWindowRect(hWnd, out origRect);

                watchingWnd = hWnd;
                windowWatchTimer = new System.Timers.Timer(250);

                // タイマーの処理
                windowWatchTimer.Elapsed += (Object source, System.Timers.ElapsedEventArgs eea) =>
                {
                    RECT r = new RECT();
                    GetWindowRect(hWnd, out r);
                    if (origRect.Right - origRect.Left != r.Right - r.Left || origRect.Bottom - origRect.Top != r.Bottom - r.Top)
                    {
                        string msg = "{\"type\":\"windowSize\",\"width\":\"" + (r.Right - r.Left).ToString() + "\",\"height\":\"" + (r.Bottom - r.Top).ToString() + "\"}";
                        websocket.SendMessage(Signaling.settings.BizAccessRoomId, Signaling.settings.BizAccessSubId, Signaling.settings.BizAccessUserId + "_share", "0", msg);
                        origRect = r;
                    }
                };

                // タイマーを開始する
                windowWatchTimer.Start();

                Remote[] users = registerArgs.RegisterParams.remotes;
                string strUser = "";
                foreach (Remote rmt in users)
                {
                    if (rmt.id != Signaling.settings.BizAccessUserId + "_share")
                    {
                        if (strUser.Length > 0)
                        {
                            strUser += ",";
                        }
                        strUser += rmt.id;
                    }
                }
                StringBuilder remoteUsers = new StringBuilder();
                remoteUsers.Append(strUser);

                IceServer[] ices = registerArgs.RegisterParams.pc_config.iceServers;
                string strIce = "";
                foreach (IceServer ice in ices)
                {
                    if (strIce.Length > 0)
                    {
                        strIce += "&";
                    }
                    strIce += ice.urls;
                    if (ice.username != null && ice.credential != null)
                    {
                        strIce += "," + ice.username + "," + ice.credential;
                    }
                }
                StringBuilder iceServers = new StringBuilder();
                iceServers.Append(strIce);

                GetUserMediaErrorCallBack pn = GotError;
                GetUserMediaSuccessCallBack sc = GotSuccess;
                SendSignalinMessage sm = GotSignalingMessage;
                StringBuilder roomId = new StringBuilder();
                roomId.Append(Signaling.settings.BizAccessRoomId);
                StringBuilder clientId = new StringBuilder();
                clientId.Append(Signaling.settings.BizAccessUserId+"_share");
#if DEBUG
                string MyPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetEntryAssembly().Location);
                //NativeDllDir.Set(MyPath + @"\..\..\..\Debug", MyPath + @"\..\..\..\x64\Debug");
                NativeDllDir.Set(MyPath, MyPath);
#else
                string MyPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetEntryAssembly().Location);
                NativeDllDir.Set(MyPath + @"\.\x86", MyPath + @"\.\x64");
#endif
                websocket.GotWebsocketMessage += GotWebsocketMessage;
                DllGetUserMedia(hWnd, roomId, clientId, iceServers, remoteUsers, sc, pn, sm);
            }
        }

        private void GotWindow(IntPtr lparam, IntPtr hWnd, int width, int height, StringBuilder title, StringBuilder image)
        {
            IntPtr hBitmap = CreateThumbNail(hWnd);
            Bitmap bmp = Image.FromHbitmap(hBitmap);
            windowThumbs.Images.Add(bmp);
            mapHwndBitmap[hWnd] = bmp;

            int cnt = mWindowList.Items.Count;
            mWindowList.Items.Add(title.ToString(), cnt);
            mWindowList.Items[cnt].Tag = hWnd;
        }

        private void GetBitmapForWindows()
        {
            mapHwndBitmap = new Dictionary<IntPtr, Bitmap>();

            windowThumbs.ImageSize = new System.Drawing.Size(160, 160);
            mWindowList.LargeImageList = windowThumbs;

            columnName = new ColumnHeader();
            columnName.Text = "タイトル";
            columnName.Width = 100;
            ColumnHeader[] colHeaderRegValue = { columnName };
            mWindowList.Columns.AddRange(colHeaderRegValue);

            IntPtr hWnd = GetDesktopWindow();
            mWindowList.Items.Add("デスクトップ", 0);
            mWindowList.Items[0].Tag = hWnd;

#if DEBUG
            string MyPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetEntryAssembly().Location);
            //NativeDllDir.Set(MyPath + @"\..\..\..\Debug", MyPath + @"\..\..\..\x64\Debug");
            NativeDllDir.Set(MyPath, MyPath);
#else
            string MyPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetEntryAssembly().Location);
            NativeDllDir.Set(MyPath, MyPath);
#endif
            IntPtr hBitmap = new IntPtr();
            try
            {
                hBitmap = CreateThumbNail(hWnd);
            }
            catch (DllNotFoundException exp)
            {
                System.Windows.MessageBox.Show(exp.ToString(),
                    "DLLロードエラー",
                    MessageBoxButton.OK,
                    MessageBoxImage.Error);
                return;
            }
            try
            {
                Bitmap bmp = Image.FromHbitmap(hBitmap);
                windowThumbs.Images.Add(bmp);
                mapHwndBitmap[hWnd] = bmp;
            } catch (BadImageFormatException exp)
            {
                System.Windows.MessageBox.Show(exp.ToString(),
                    "イメージエラー",
                    MessageBoxButton.OK,
                    MessageBoxImage.Error);
                return;
            }

            EnumWindowsDelegate pn = GotWindow;
            GetWindowList(IntPtr.Zero, pn);
        }

        private void GotWebsocketMessage(MessageArgs args)
        {
            Data data = args.MessageData;
            StringBuilder roomid = new StringBuilder();
            roomid.Append(data.roomid);
            StringBuilder clientid = new StringBuilder();
            clientid.Append(data.clientid);
            StringBuilder remoteid = new StringBuilder();
            remoteid.Append(data.remoteid);

            //string mess = args.MessageParams;
            switch (data.msg.type)
            {
                case "offer":
                case "answer":
                    {
                        StringBuilder sdp = new StringBuilder();
                        sdp.Append(data.msg.sdp.Replace("\\r\\n", "\r\n"));
                        StringBuilder type = new StringBuilder();
                        type.Append(data.msg.type);
                        DllGotRemoteSdp(sdp, type, roomid, clientid, remoteid);
                        break;
                    }
                case "candidate":
                    {
                        StringBuilder id = new StringBuilder();
                        id.Append(data.msg.id);
                        StringBuilder candidate = new StringBuilder();
                        candidate.Append(data.msg.candidate);
                        DllGotRemoteCandidate(data.msg.label, id, candidate, roomid, clientid, remoteid);
                        break;
                    }
                case "eoic":
                    DllGotEoic(roomid, clientid, remoteid);
                    break;
                default:
                    break;
            }
        }

        private void GotSuccess(StringBuilder remoteid)
        {
            IntPtr hWnd = (IntPtr)mWindowList.SelectedItems[0].Tag;
            RECT r = new RECT();
            GetWindowRect(hWnd, out r);

            string msg = "{\"type\":\"windowSize\",\"width\":\"" + (r.Right - r.Left).ToString() + "\",\"height\":\"" + (r.Bottom - r.Top).ToString() + "\"}";
            websocket.SendMessage(Signaling.settings.BizAccessRoomId, Signaling.settings.BizAccessSubId, Signaling.settings.BizAccessUserId+"_share", remoteid.ToString(), msg);
        }

        private void GotError(StringBuilder error)
        {
            System.Windows.MessageBox.Show("接続に失敗しました：" + error.ToString(),
                "エラー",
                MessageBoxButton.OK,
                MessageBoxImage.Error);
        }

        private void GotSignalingMessage(StringBuilder remoteId, StringBuilder message)
        {
            if (websocket != null)
                websocket.SendMessage(Signaling.settings.BizAccessRoomId, Signaling.settings.BizAccessSubId, Signaling.settings.BizAccessUserId+"_share", remoteId.ToString(), message.ToString());
            JObject data = (JObject)JsonConvert.DeserializeObject<JObject>(message.ToString());
            if (data["type"].Value<string>() == "eoic")
            {
                this.mShareButton.Text = "停止";
                isSharingNow = true;
            }
        }

        private void CallBackEventRegister(RegisterArgs e)
        {
            registerArgs = e;
            if (waitForm != null)
            {
                waitForm.bCloseToExit = false;
                waitForm.Close();
            }
        }
    }

    public static class NativeDllDir
    {
        [System.Runtime.InteropServices.DllImport("kernel32", SetLastError = true)]
        private static extern bool SetDllDirectory(string lpPathName);

        /// <summary>
        /// DllImport用に、x86用のDLLのあるディレクトリとx64用のDLLのあるディレクトリを設定します。
        /// </summary>
        /// <param name="x86DllDir">x86環境用のDLLを配置したディレクトリを指定します。指定しなければカレントディレクトリとなります。</param>
        /// <param name="x64DllDir">x64環境用のDLLを配置したディレクトリを指定します。指定しなければカレントディレクトリとなります。</param>
        /// <returns>設定に成功したらtrue。</returns>
        /// <exception cref="PlatformNotSupportedException">x86でもx64でもない場合の例外です。</exception>
        public static bool Set(string x86DllDir = null, string x64DllDir = null)
        {
            // 既に設定されているものをリセット
            SetDllDirectory(null);

            if (Environment.Is64BitProcess)
            {
                return SetDllDirectory(string.IsNullOrEmpty(x64DllDir) ? "." : x64DllDir);
            }
            else
            {
                return SetDllDirectory(string.IsNullOrEmpty(x86DllDir) ? "." : x86DllDir);
            }
        }
    }
}
