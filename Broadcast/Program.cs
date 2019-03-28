using System;
using System.Diagnostics;
using System.IO;
using System.Threading.Tasks;
using System.Windows.Forms;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace Broadcast
{
    static class Program
    {
        /// <summary>
        /// アプリケーションのメイン エントリ ポイントです。
        /// </summary>
        [STAThread]
        static void Main()
        {
            //Task.Run(() => {
            //    Wait();
            //});
            JObject data = null;
            string[] cmds = System.Environment.GetCommandLineArgs();
            if (cmds.Length > 1)
            {
                string json = cmds[1].Replace("{", "{\"").Replace(":", "\":\"").Replace(",", "\",\"").Replace("}", "\"}");
                try
                {
                    data = JObject.Parse(json);
                } catch (Exception exp)
                {
                    MessageBox.Show(exp.ToString());
                }
            }
            else
            {
                data = Wait();
            }
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new SelectScreen(data));
        }

        public static JObject Wait()
        {
            JObject data = Read();
            if (data != null)
            {
                //System.Diagnostics.Trace.WriteLine("create select screen: " + data.ToString());
                //selectScreen.LoginFromWeb(data);
                if (data.ContainsKey("command"))
                {
                    if (data["command"].Value<string>() == "ping")
                    {
                        Write("pong");
                    }
                    else
                    {
                        Write("Invalid command");
                    }
                }
                else
                {
                    Write("get Data");
                }
            }
            return data;
            /*
            while ((data = Read()) != null)
            {
                var processed = ProcessMessage(data);
                Write(processed);
                if (processed == "exit")
                {
                    return;
                }
            }
            */
        }

        /*
        public static string ProcessMessage(JObject data)
        {
            var message = data["message"].Value<string>();
            switch (message)
            {
                case "test":
                    return "testing!";
                case "exit":
                    return "exit";
                default:
                    return "echo: " + message;
            }
        }
        */

        public static JObject Read()
        {
            var stdin = Console.OpenStandardInput();
            var length = 0;

            var lengthBytes = new byte[4];
            stdin.Read(lengthBytes, 0, 4);
            length = BitConverter.ToInt32(lengthBytes, 0);

            var buffer = new char[length];
            using (var reader = new StreamReader(stdin))
            {
                while (reader.Peek() >= 0)
                {
                    reader.Read(buffer, 0, buffer.Length);
                }
            }
            //System.Diagnostics.Trace.WriteLine("get data in buffer: " + new string(buffer));

            try
            {
                JObject data = (JObject)JsonConvert.DeserializeObject<JObject>(new string(buffer));
                return data/*["data"]*/;
            } catch (NullReferenceException)
            {
                return null;
            }
        }

        public static void Write(JToken data)
        {
            var json = new JObject();
            json["data"] = data;

            var bytes = System.Text.Encoding.UTF8.GetBytes(json.ToString(Formatting.None));

            var stdout = Console.OpenStandardOutput();
            stdout.WriteByte((byte)((bytes.Length >> 0) & 0xFF));
            stdout.WriteByte((byte)((bytes.Length >> 8) & 0xFF));
            stdout.WriteByte((byte)((bytes.Length >> 16) & 0xFF));
            stdout.WriteByte((byte)((bytes.Length >> 24) & 0xFF));
            stdout.Write(bytes, 0, bytes.Length);
            stdout.Flush();
        }
    }
}
