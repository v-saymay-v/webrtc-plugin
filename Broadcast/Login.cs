using System;
using System.Collections.Generic;
using System.IO;
using System.Net.Http;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using System.Text;
using System.Windows;
using System.Xml.Serialization;
using WebSocket4Net;

namespace Broadcast
{
    [DataContract]
    public class Data
    {
        [DataMember]
        public string error { get; set; }
        [DataMember]
        public string invalid { get; set; }
        [DataMember]
        public Msg msg { get; set; }
        [DataMember]
        public string roomid { get; set; }
        [DataMember]
        public string clientid { get; set; }
        [DataMember]
        public string remoteid { get; set; }
        [DataMember]
        public string cmd { get; set; }
    }

    [DataContract]
    public class Msg
    {
        [DataMember]
        public string type { get; set; }
        [DataMember]
        public string session { get; set; }
        [DataMember]
        public string number { get; set; }
        [DataMember]
        public string userid { get; set; }
        [DataMember]
        public Params @params {get; set;}
        [DataMember]
        public string sdp { get; set; }
        [DataMember]
        public string name { get; set; }
        [DataMember]
        public string candidate { get; set; }
        [DataMember]
        public int label { get; set; }
        [DataMember]
        public string id { get; set; }
    }

    [DataContract]
    public class IceServer
    {
        [DataMember]
        public string urls { get; set; }
        [DataMember]
        public string username { get; set; }
        [DataMember]
        public string credential { get; set; }
}

    [DataContract]
    public class PcConfig
    {
        [DataMember]
        public string rtcpMuxPolicy { get; set; }
        [DataMember]
        public string bundlePolicy { get; set; }
        [DataMember]
        public IceServer[] iceServers { get; set; }
    }

    [DataContract]
    public class Remote
    {
        [DataMember]
        public string id { get; set; }
        [DataMember]
        public string name { get; set; }
        [DataMember]
        public string image { get; set; }
    }

    [DataContract]
    public class Params
    {
        [DataMember]
        public string is_initiator { get; set; }
        [DataMember]
        public string[] messages { get; set; }
        [DataMember]
        public string room_state { get; set; }
        [DataMember]
        public Remote[] remotes { get; set; }
        [DataMember]
        public PcConfig pc_config { get; set; }
    }

    public class Login
    {
        static public string bizAccessHost = "www.bizaccess.jp";
        static public string bizAccessPort = "8443";
        static public string bizAccessWssUrl = "wss://" + bizAccessHost + ":" + bizAccessPort + "/ws";

        public Login()
        {
        }
    }
}
