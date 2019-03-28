using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Runtime.Serialization.Json;
using System.Text;
using System.Windows.Forms;
using System.Xml.Serialization;
using WebSocket4Net;
using Newtonsoft.Json.Linq;
using System.Windows;

namespace Broadcast
{
    public class RegisterArgs : EventArgs
    {
        private readonly Params _params;

        public RegisterArgs(Params prms)
        {
            _params = prms;
        }
        public Params RegisterParams { get { return _params; } }
    }

    public class MessageArgs : EventArgs
    {
        private readonly string message;
        private readonly Data data;

        public MessageArgs(string msg, Data dt)
        {
            message = msg;
            data = dt;
        }
        public string MessageParams { get { return message; } }
        public Data MessageData { get { return data; } }
    }

    public class Signaling
    {
        private WebSocket websocket;
        public List<string> pendingCmd;
        static public Settings settings;

        private bool isLoggedIn = false;
        private bool isResetting = false;

        //-----------------------------------
        //イベントハンドラのデリゲートを定義
        //public delegate void MyEventHandler(EventArgs e);
        //public event MyEventHandler MyProgressEvent;

        public delegate void MyLoginHandler(EventArgs e);
        public event MyLoginHandler MyLoginEvent;

        public delegate void MyCloseHandler(EventArgs e);
        public event MyCloseHandler MyCloseEvent;

        public delegate void MyRegisterHandler(RegisterArgs e);
        public event MyRegisterHandler MyRegisterEvent;

        public delegate void WebsockeMessageHandler(MessageArgs e);
        public event WebsockeMessageHandler GotWebsocketMessage;

        public delegate void WebsocketErrorHandler(SuperSocket.ClientEngine.ErrorEventArgs error);
        public event WebsocketErrorHandler GotWebsocketError;

        public delegate void MyLoginError(string error);
        public event MyLoginError MyErrorEvent;

        public delegate void MyLoginInvalid(string error);
        public event MyLoginInvalid MyInvalidEvent;

        public Control mainThreadForm;

        public Signaling(Control cnt, JObject data)
        {
            mainThreadForm = cnt;
            pendingCmd = new List<String>();

            websocket = new WebSocket(Login.bizAccessWssUrl);
            websocket.Opened += Websocket_Opened;
            websocket.Closed += Websocket_Closed;
            websocket.MessageReceived += Websocket_MessageReceived_Login;
            websocket.DataReceived += Websocket_DataReceived;
            websocket.Error += Websocket_Error;

            websocket.AutoSendPingInterval = 30;
            websocket.EnableAutoSendPing = true;

            websocket.Open();

            XmlSerializer serializer = new XmlSerializer(typeof(Settings));
            FileStream fs = null;
            try
            {
                string str;
                fs = new FileStream(Directory.GetCurrentDirectory() + "\\" + "settings.xml", FileMode.Open, FileAccess.Read);
                settings = (Settings)serializer.Deserialize(fs);
                if (data != null)
                {
                    str = data["BizAccessSession"].Value<string>();
                    if (str != null && str != "" && settings.BizAccessSession != str)
                    {
                        settings.BizAccessSession = str;
                    }
                    str = data["BizAccessRoomId"].Value<string>();
                    if (str != null && str != "" && settings.BizAccessRoomId != str)
                    {
                        settings.BizAccessRoomId = str;
                    }
                    str = data["BizAccessSubId"].Value<string>();
                    if (str != null && str != "" && settings.BizAccessRoomId != str)
                    {
                        settings.BizAccessSubId = str;
                    }
                    str = data["BizAccessUserId"].Value<string>();
                    if (str != null && str != "" && settings.BizAccessUserId != str)
                    {
                        settings.BizAccessUserId = str;
                    }
                    str = data["BizAccessUserPhoto"].Value<string>();
                    if (str != null && str != "" && settings.BizAccessUserPhoto != str)
                    {
                        settings.BizAccessUserPhoto = str;
                    }
                    str = data["BizAccessUserName"].Value<string>();
                    if (str != null && str != "" && settings.BizAccessUserName != str)
                    {
                        settings.BizAccessUserName = str;
                    }
                    str = data["BizAccessPhoneNo"].Value<string>();
                    if (str != null && str != "" && settings.BizAccessPhoneNo != str)
                    {
                        settings.BizAccessPhoneNo = str;
                    }
                    str = data["HotBizSession"].Value<string>();
                    if (str != null && str != "" && settings.HotBizSession != str)
                    {
                        settings.HotBizSession = str;
                    }
                    str = data["HotBizUserId"].Value<string>();
                    if (str != null && str != "" && settings.HotBizUserId != str)
                    {
                        settings.HotBizUserId = str;
                    }
                }
                str = settings.BizAccessEndPoint;
                if (str == null || str == "")
                {
                    settings.BizAccessEndPoint = Guid.NewGuid().ToString();
                }
                fs.Close();
            }
            catch (FileNotFoundException)
            {
                fs = new FileStream(Directory.GetCurrentDirectory() + "\\" + "settings.xml", FileMode.Create, FileAccess.ReadWrite);
                settings = new Settings();
                settings.BizAccessEndPoint = Guid.NewGuid().ToString();
                if (data != null)
                {
                    settings.BizAccessSession = data["BizAccessSession"].Value<string>();
                    settings.BizAccessRoomId = data["BizAccessRoomId"].Value<string>();
                    settings.BizAccessSubId = data["BizAccessSubId"].Value<string>();
                    settings.BizAccessUserId = data["BizAccessUserId"].Value<string>();
                    settings.BizAccessUserPhoto = data["BizAccessUserPhoto"].Value<string>();
                    settings.BizAccessUserName = data["BizAccessUserName"].Value<string>();
                    settings.BizAccessPhoneNo = data["BizAccessPhoneNo"].Value<string>();
                    settings.HotBizSession = data["HotBizSession"].Value<string>();
                    settings.HotBizUserId = data["HotBizUserId"].Value<string>();
                }
                try
                {
                    serializer.Serialize(fs, settings);
                    fs.Close();
                }
                catch (Exception exp)
                {
                    System.Windows.MessageBox.Show(exp.ToString(),
                        "ファイルシステムエラー",
                        MessageBoxButton.OK,
                        MessageBoxImage.Error);
                }
            }
            catch (Exception exp)
            {
                System.Windows.MessageBox.Show(exp.ToString(),
                    "ファイルシステムエラー",
                    MessageBoxButton.OK,
                    MessageBoxImage.Error);
            }
            /*
            string message = "BizAccessSession:" + settings.BizAccessSession + "\r\n" +
                "BizAccessRoomId:" + settings.BizAccessRoomId + "\r\n" +
                "BizAccessUserId:" + settings.BizAccessUserId + "\r\n" +
                "BizAccessUserPhoto:" + settings.BizAccessUserPhoto + "\r\n" +
                "BizAccessUserName:" + settings.BizAccessUserName + "\r\n" +
                "BizAccessPhoneNo:" + settings.BizAccessPhoneNo + "\r\n" +
                "HotBizSession:" + settings.HotBizSession + "\r\n" +
                "HotBizUserId:" + settings.HotBizUserId + "\r\n";
            System.Windows.MessageBox.Show(message,
                "デバッグメッセージ",
                MessageBoxButton.OK,
                MessageBoxImage.Information);
            */
        }

        ~Signaling()
        {
            if (websocket.State == WebSocketState.Open)
            {
                websocket.Close();
            }
        }

        public void ResetWebSocket()
        {
            if (websocket.State == WebSocketState.Open)
            {
                isResetting = true;
                websocket.Close();
            } else if (websocket.State == WebSocketState.Closing || websocket.State == WebSocketState.Closed)
            {
                try
                {
                    websocket.Open();
                }
                catch (Exception)
                {
                }
            }
        }

        private void Websocket_Opened(object sender, EventArgs e)
        {
            if (pendingCmd.Count() > 0)
            {
                for (int i = 0; i < pendingCmd.Count(); ++i)
                {
                    websocket.Send(pendingCmd[i]);
                }
                pendingCmd.Clear();
            }
#if DEBUG
            System.Diagnostics.Trace.WriteLine("Websocket_Opened");
#endif
        }

        private void Websocket_Closed(object sender, EventArgs e)
        {
#if DEBUG
            System.Diagnostics.Trace.WriteLine("closed");
#endif
            if (isResetting)
            {
                try
                {
                    websocket.Open();
                } catch(Exception)
                {
                }
                MyCloseEvent(new EventArgs());
                isResetting = false;
            }
        }

        private void Websocket_Error(object sender, SuperSocket.ClientEngine.ErrorEventArgs e)
        {
#if DEBUG
            System.Diagnostics.Trace.WriteLine($"Websocket_Error { e.ToString() } ");
#endif
            //mainThreadForm.Invoke(GotWebsocketError, e);
            GotWebsocketError(e);
        }

        private static void Websocket_DataReceived(object sender, WebSocket4Net.DataReceivedEventArgs e)
        {
#if DEBUG
            System.Diagnostics.Trace.WriteLine($"Websocket_DataReceived { e.Data }");
#endif
        }

        private void Websocket_MessageReceived_Login(object sender, MessageReceivedEventArgs e)
        {
            var websocket = (WebSocket)sender;
            var serializer = new DataContractJsonSerializer(typeof(Data));
            try
            {
                using (var ms = new MemoryStream(Encoding.UTF8.GetBytes(e.Message)))
                {
                    var data = (Data)serializer.ReadObject(ms);
                    if (data.error != null)
                    {
                        mainThreadForm.Invoke(MyErrorEvent, data.error);
                        return;
                    }
                    else if (data.invalid != null)
                    {
                        mainThreadForm.Invoke(MyInvalidEvent, data.invalid);
                        return;
                    }

                    switch (data.msg.type)
                    {
                        case "userUpdated":
                            {
                                settings.BizAccessSession = data.msg.session;
                                settings.BizAccessPhoneNo = data.msg.number;
                                settings.BizAccessUserId = data.msg.userid;
                                try
                                {
                                    XmlSerializer xmlserializer = new XmlSerializer(typeof(Settings));
                                    FileStream fs = new FileStream(Directory.GetCurrentDirectory() + "\\" + "settings.xml", FileMode.Truncate, FileAccess.Write);
                                    xmlserializer.Serialize(fs, settings);
                                    fs.Close();
                                } catch (Exception){ }
                                break;
                            }
                        case "logedin":
                            mainThreadForm.Invoke(MyLoginEvent, new EventArgs());
                            break;
                        case "joined":
                            mainThreadForm.Invoke(MyRegisterEvent, new RegisterArgs(data.msg.@params));
                            IsLoggedIn = true;
                            break;
                        default:
                            break;
                    }
                }
            }
            catch (Exception)
            {
                string str = e.Message.Replace("\\\"", "\"");
                str = str.Replace("\"{", "{");
                str = str.Replace("}\"", "}");
                using (var ms = new MemoryStream(Encoding.UTF8.GetBytes(str)))
                {
                    var data = (Data)serializer.ReadObject(ms);
                    mainThreadForm.Invoke(GotWebsocketMessage, new MessageArgs(e.Message, data));
                }

            }
#if DEBUG
            System.Diagnostics.Trace.WriteLine($"Websocket_MessageReceived { e.Message }");
#endif
        }

        public bool CheckLogin()
        {
            return (settings.BizAccessSession != null && settings.HotBizSession != null);
        }

        public async System.Threading.Tasks.Task LoginBizAccessAsync(string svr, string hb, string id, string pw)
        {
            settings.BizAccessRoomId = hb;

            string reqUrl = "https://" + svr + "/" + hb + "/hb_chat_login.cgi";

            var parameters = new Dictionary<string, string>()
            {
                { "user", id },
                { "pass", pw },
                { "login", "ログイン" },
            };
            var content = new FormUrlEncodedContent(parameters);

            using (var client = new HttpClient())
            {
                var res = await client.PostAsync(reqUrl, content);
                string response = res.ToString();
                string search = "hotbiz:///?";
                int start = response.IndexOf(search);
                if (start < 0)
                {
                    mainThreadForm.Invoke(MyInvalidEvent, "パラメーターが間違っています。");
                    return;
                }
                string str = response.Substring(start + search.Length);
                int end = str.IndexOf("\r\n");
                if (end < 0)
                {
                    mainThreadForm.Invoke(MyInvalidEvent, "パラメーターが間違っています。");
                    return;
                }
                string paramstr = str.Substring(0, end).Replace("&amp;", "&");
                string[] splits = paramstr.Split('&');
                foreach (string split in splits)
                {
                    string[] keyval = split.Split('=');
                    if (keyval[0] == "name")
                    {
                        settings.BizAccessUserName = System.Uri.UnescapeDataString(keyval[1]).Replace('+', ' ');
                    }
                    else if (keyval[0] == "session")
                    {
                        settings.HotBizSession = keyval[1];
                    }
                    else if (keyval[0] == "url")
                    {
                        settings.BizAccessUserPhoto = System.Uri.UnescapeDataString(keyval[1]);
                        Uri u = new Uri(settings.BizAccessUserPhoto);
                        string[] querys = u.Query.Substring(1).Split('&');
                        foreach (string query in querys)
                        {
                            string[] kv = query.Split('=');
                            if (kv[0] == "user")
                            {
                                settings.HotBizUserId = kv[1];
                                break;
                            }
                        }
                    }
                }
                if (settings.BizAccessUserName == "" || settings.HotBizSession == "" ||
                    settings.BizAccessUserPhoto == "" || settings.HotBizUserId == "")
                {
                    mainThreadForm.Invoke(MyInvalidEvent, "パラメーターが間違っています。");
                    return;
                }
                WebSocketLogin();
            }
        }

        private void WebSocketLogin()
        {
            string cmd = "{" +
                "\"cmd\": \"newUser\"," +
                "\"msg\": {" +
                    "\"type\": \"hotbiz\"," +
                    "\"uid\":\"" + settings.HotBizUserId + "\"," +
                    "\"name\":\"" + settings.BizAccessUserName + "\"," +
                    "\"photo\":\"" + settings.BizAccessUserPhoto + "\"," +
                    "\"cover\": \"\"," +
                    "\"token\":\"" + settings.HotBizSession + "\"," +
                    "\"secret\": \"\"," +
                    "\"device\":\"" + settings.BizAccessEndPoint + "\"," +
                    "\"push\":\"" + settings.BizAccessEndPoint + "\"," +
                    "\"os\": \"win\"" +
                "}" +
            "}";
            if (websocket.State == WebSocketState.Open)
            {
                websocket.Send(cmd);
            } else
            {
                pendingCmd.Add(cmd);
            }
        }

        public void LoginToRoom()
        {
            string cmd = "{" +
                "\"cmd\":\"loginRoom\"," +
                "\"subid\":\"" + settings.BizAccessSubId + "\"," +
                "\"msg\": {" +
                    "\"sessionId\":\"" + settings.BizAccessSession + "\"," +
                    "\"roomid\":\"" + settings.BizAccessRoomId + "\"," +
                    "\"password\": \"\"" +
                "}" +
            "}";
            if (websocket.State == WebSocketState.Open)
            {
                websocket.Send(cmd);
            }
            else
            {
                pendingCmd.Add(cmd);
            }
        }

        public void RegisterRoom()
        {
            string cmd = "{"+
                "\"cmd\":\"register\","+
                "\"roomid\":\"" + settings.BizAccessRoomId + "\","+
                "\"subid\":\"" + settings.BizAccessSubId + "\"," +
                "\"clientid\":\"" + settings.BizAccessUserId + "_share\","+
                "\"sessionId\":\"" + settings.BizAccessSession + "\"," +
                "\"callKind\":\"incoming\""+
            "}";
            if (websocket.State == WebSocketState.Open)
            {
                websocket.Send(cmd);
            }
            else
            {
                pendingCmd.Add(cmd);
            }
        }

        public WebSocket WebSocket
        {
            get { return websocket; }
        }

        public bool IsLoggedIn
        {
            get { return isLoggedIn; }
            set { isLoggedIn = value; }
        }

        public void SendMessage(string roomId, string subId, string clientId, string remoteId, string message)
        {
            if (websocket.State == WebSocketState.Closed)
            {
                pendingCmd.Add("{\"cmd\":\"send\",\"msg\":" + message + ",\"roomid\":\"" + roomId + ",\"subid\":\"" + subId + "\",\"clientid\":\"" + clientId + "\",\"remoteid\":\"" + remoteId + "\"}");
                websocket.Open();
            } else
            {
                string cmd = "{\"cmd\":\"send\",\"msg\":" + message + ",\"roomid\":\"" + roomId + ",\"subid\":\"" + subId + "\",\"clientid\":\"" + clientId + "\",\"remoteid\":\"" + remoteId + "\"}";
                websocket.Send(cmd);
            }
        }
    }
}
