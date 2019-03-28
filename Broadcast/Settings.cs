using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Broadcast
{
    public class Settings
    {
        #region メンバ変数
        private string _bizAccessSession;
        private string _bizAccessRoomId;
        private string _bizAccessSubId;
        private string _bizAccessUserId;
        private string _bizAccessUserPhoto;
        private string _bizAccessUserName;
        private string _bizAccessPhoneNo;
        private string _bizAccessEndPoint;
        private string _hotBizSession;
        private string _hotBizUserId;
        #endregion

        #region プロパティ
        public string BizAccessEndPoint
        {
            get { return _bizAccessEndPoint; }
            set { _bizAccessEndPoint = value; }
        }

        public string BizAccessSession
        {
            get { return _bizAccessSession; }
            set { _bizAccessSession = value; }
        }

        public string BizAccessRoomId
        {
            get { return _bizAccessRoomId; }
            set { _bizAccessRoomId = value; }
        }

        public string BizAccessSubId
        {
            get { return _bizAccessSubId; }
            set { _bizAccessSubId = value; }
        }

        public string BizAccessUserId
        {
            get { return _bizAccessUserId; }
            set { _bizAccessUserId = value; }
        }

        public string BizAccessUserPhoto
        {
            get { return _bizAccessUserPhoto; }
            set { _bizAccessUserPhoto = value; }
        }

        public string BizAccessUserName
        {
            get { return _bizAccessUserName; }
            set { _bizAccessUserName = value; }
        }

        public string BizAccessPhoneNo
        {
            get { return _bizAccessPhoneNo; }
            set { _bizAccessPhoneNo = value; }
        }

        public string HotBizSession
        {
            get { return _hotBizSession; }
            set { _hotBizSession = value; }
        }

        public string HotBizUserId
        {
            get { return _hotBizUserId; }
            set { _hotBizUserId = value; }
        }
        #endregion
    }
}
