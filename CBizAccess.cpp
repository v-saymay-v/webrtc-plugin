#include "stdafx.h"

#include "webrtc\modules\desktop_capture\desktop_capture_types.h"

#include "ExMediaStreamConstraints.h"
#include "ExRTCPeerConnectionIceEvent.h"
#include "ExMediaStreamEvent.h"
#include "CBizAccess.h"

#include <regex>

MyDisplay::MyDisplay()
{
	m_hWnd = NULL;
	m_bVideoRendererStarted = FALSE;
}

MyDisplay::~MyDisplay()
{
	StopVideoSink();
	DestroyWindow(m_hWnd);
}

void MyDisplay::OnStartVideoSink()
{
	m_bVideoRendererStarted = TRUE;
}

void MyDisplay::OnStopVideoSink()
{
	m_bVideoRendererStarted = FALSE;
}

void MyDisplay::AttachMediaStream(std::shared_ptr<ExMediaStream> exMediaStream) {
	StartVideoSink(exMediaStream->GetVideoTrack());
}

LRESULT CALLBACK MyDisplay::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CREATE:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_PAINT: {
		LONG_PTR ptr = ::GetWindowLongPtr(hWnd, GWLP_USERDATA);
		MyDisplay* pDisplay = (MyDisplay*)ptr;
		if (pDisplay && pDisplay->m_bVideoRendererStarted) {
			RECT rc;
			RECTL rcl;
			ATL_DRAWINFO di;
			GetClientRect(hWnd, &rc);
			rcl.bottom = rc.bottom;
			rcl.left = rc.left;
			rcl.right = rc.right;
			rcl.top = rc.top;
			memset(&di, 0, sizeof(di));
			di.cbSize = sizeof(di);
			di.dwDrawAspect = DVASPECT_CONTENT;
			di.lindex = 0;
			di.ptd = NULL;
			di.hicTargetDev = NULL;
			di.hdcDraw = NULL;	// hdc;
			di.prcBounds = &rcl; //Rectangle in which to draw
			di.prcWBounds = NULL;	// di.prcBounds; //WindowOrg and Ext if metafile
			di.bOptimize = FALSE;
			di.bZoomed = FALSE;
			di.bRectInHimetric = FALSE;
			di.ZoomNum.cx = 1;      //ZoomX = ZoomNum.cx/ZoomNum.cy
			di.ZoomNum.cy = 1;
			di.ZoomDen.cx = rc.right - rc.left;
			di.ZoomDen.cy = rc.bottom - rc.top;
			pDisplay->PaintFrame(reinterpret_cast<intptr_t>(&di));
		}
		else {
			RECT rc;
			PAINTSTRUCT ps;
			GetClientRect(hWnd, &rc);
			HDC hdc = BeginPaint(hWnd, &ps);
			Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
			SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
			LPCTSTR pszText = _T("ATL: RTC Plugin version " kRTC_VersionString);
			TextOut(hdc,
				(rc.left + rc.right) / 2,
				(rc.top + rc.bottom) / 2,
				pszText,
				lstrlen(pszText));
			EndPaint(hWnd, &ps);
		}
		break;
	}
	default:
		return(DefWindowProc(hWnd, msg, wParam, lParam));
	}
	return (0L);
}

HWND MyDisplay::Handle() {
	if (!m_hWnd) {
		WNDCLASS wc;
		TCHAR szClassName[] = TEXT("STATIC");
		HINSTANCE hInst = GetModuleHandle(NULL);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInst;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = szClassName;
		RegisterClass(&wc);
		m_hWnd = ::CreateWindowEx(0, szClassName, TEXT("Kitty on your lap"),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, 320, 240,
			NULL, NULL, hInst, (LPVOID)this);
		::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
		//ShowWindow(m_hWnd, SW_SHOW);
	}
	return m_hWnd;
}


const std::string CBizAccess::bizAccessHost = "www.bizaccess.jp";
const std::string CBizAccess::bizAccessPort = "8443";
const std::string CBizAccess::bizAccessWssUrl = "wss://" + bizAccessHost + ":" + bizAccessPort + "/ws";

GetUserMediaSuccessCallBack CBizAccess::getUserMediaSuccessCallBack;
GetUserMediaErrorCallBack CBizAccess::getUserMediaErrorCallBack;
SendSignalingMessageCallBack CBizAccess::sendSignalingMessageCallBack;

std::map<std::string, std::shared_ptr<MyRTCPeerConnection>> CBizAccess::peerConnectionMap;
//std::vector<std::shared_ptr<ExRTCIceCandidate>> CBizAccess::offerCandidates;
std::string CBizAccess::clientId;

CBizAccess::CBizAccess(HWND hWnd, LPCSTR roomId, LPCSTR clientId, LPCSTR iceServers, LPCSTR remoteUsers,
	GetUserMediaSuccessCallBack successFunc, GetUserMediaErrorCallBack errorfuncp, SendSignalingMessageCallBack messagefuncp) : MyDisplay()
{
	m_hTargetWnd = hWnd;
	getUserMediaSuccessCallBack = successFunc;
	getUserMediaErrorCallBack = errorfuncp;
	sendSignalingMessageCallBack = messagefuncp;
	CBizAccess::clientId = clientId;

	char *context;
	LPSTR pRemoteUsers = new CHAR[strlen(remoteUsers)+1];
	strcpy(pRemoteUsers, remoteUsers);
	char *str = strtok_s(pRemoteUsers, ",", &context);
	while (str) {
		if (strcmp(clientId, str)) {
			CBizAccess::peerConnectionMap[std::string(str)] = NULL;
		}
		str = strtok_s(NULL, ",", &context);
	}
	delete (pRemoteUsers);

	lpIceServers = new CHAR[strlen(iceServers) + 1];
	strcpy(lpIceServers, iceServers);

	TakeFakePeerConnectionFactory();
}

CBizAccess::~CBizAccess()
{
	try
	{
		delete lpIceServers;
		/*
		std::shared_ptr<Sequence<ExMediaStreamTrack>> tracks = localStream.get()->getTracks();
		std::vector<shared_ptr<ExMediaStreamTrack>>::const_iterator it;
		for (it = tracks.get()->values.begin(); it != tracks.get()->values.end(); ++it)
		{
			localStream.get()->removeTrack(*it);
		}
		std::map<std::string, std::shared_ptr<MyRTCPeerConnection>>::const_iterator it2;
		for (it2 = peerConnectionMap.begin(); it2 != peerConnectionMap.end(); ++it2)
		{
			it2->second.get()->close();
		}
		peerConnectionMap.clear();
		*/
	}
	catch (exception &exp) {
		size_t size;
		wchar_t mess[1024];
		errno_t err = mbstowcs_s(
			&size,
			mess,
			strlen(exp.what()),
			exp.what(),
			sizeof(mess)
		);
		::MessageBox(NULL, mess, _TEXT("error"), MB_OK);
	}
	ReleaseFakePeerConnectionFactory();
}

void CBizAccess::GetUserMedia()
{
	std::shared_ptr<MediaConstraintSet> set = std::make_shared<MediaConstraintSet>();
	MediaConstraintSet* pset = set.get();
	pset->insert_or_assign(std::string("chromeMediaSource"), std::string("window"));
	webrtc::WindowId id = reinterpret_cast<webrtc::WindowId>(m_hTargetWnd);
	pset->insert_or_assign(std::string("chromeMediaSourceId"), std::to_string(id));
	std::shared_ptr<ExMediaTrackConstraints> video = std::make_shared<ExMediaTrackConstraints>(set);
	ExMediaStreamConstraints* constraints = new ExMediaStreamConstraints(nullptr, video);
	NavigatorUserMediaSuccessCallback success = std::function<void(std::shared_ptr<ExMediaStream> stream)>([this](std::shared_ptr<ExMediaStream> stream) {
		AttachMediaStream(stream);

		rtcConf = std::make_shared<RTCConfiguration>();
		mediaConstraints = std::make_shared<MediaConstraintSets>();
		RTCConfiguration* pConfig = rtcConf.get();

		char *context;
		char* str = strtok_s(lpIceServers, "&", &context);
		while (str) {
			char *server = new char[strlen(str) + 1];
			strcpy(server, str);
			char *cntxt;
			char *urls = NULL;
			char *username = NULL;
			char *credential = NULL;
			char *cnm = strtok_s(server, ",", &cntxt);
			int pos = 0;
			while (cnm) {
				switch (pos) {
				case 0:
					urls = new char[strlen(cnm) + 1];
					strcpy(urls, cnm);
					break;
				case 1:
					username = new char[strlen(cnm) + 1];
					strcpy(username, cnm);
					break;
				case 2:
					credential = new char[strlen(cnm) + 1];
					strcpy(credential, cnm);
					break;
				default:
					break;
				}
				cnm = strtok_s(NULL, ",", &cntxt);
				++pos;
			}
			cricket::ContentNames names;
			names.push_back(std::string(urls));
			RTCIceServer iceServer(names, username ? std::string(username) : std::string(), credential ? std::string(credential) : std::string());
			pConfig->iceServers.push_back(iceServer);
			delete urls;
			delete username;
			delete credential;
			delete server;
			str = strtok_s(NULL, "&", &context);
		}
		localStream = stream;

		std::map<std::string, std::shared_ptr<MyRTCPeerConnection>>::iterator it;
		for (it = CBizAccess::peerConnectionMap.begin(); it != CBizAccess::peerConnectionMap.end(); it++) {
			std::shared_ptr<MyRTCPeerConnection> ex(new MyRTCPeerConnection(rtcConf.get(), mediaConstraints.get(), stream, true, it->first));
			it->second = ex;
			//if (getUserMediaSuccessCallBack) {
			//	getUserMediaSuccessCallBack(it->first.c_str());
			//}
		}
	});
	NavigatorUserMediaErrorCallback error = std::function<void(std::shared_ptr<ExErrorMessage> e)>(/*getUserMediaError*/[](std::shared_ptr<ExErrorMessage> e) {
		if (getUserMediaErrorCallBack) {
			ExErrorMessage *ex = e.get();
			getUserMediaErrorCallBack(ex->name().c_str());
		}
	});
	::getUserMedia(constraints, success, error);
}

static std::vector<std::string> split(std::string string, std::string separator)
{
	auto separator_length = separator.length(); // ãÊêÿÇËï∂éöÇÃí∑Ç≥

	auto list = std::vector<std::string>();

	if (separator_length == 0) {
		list.push_back(string);
	}
	else {
		auto offset = std::string::size_type(0);
		while (1) {
			auto pos = string.find(separator, offset);
			if (pos == std::string::npos) {
				list.push_back(string.substr(offset));
				break;
			}
			list.push_back(string.substr(offset, pos - offset));
			offset = pos + separator_length;
		}
	}
	return list;
}

static std::string join(vector<std::string> v, const char* delim)
{
	// åãçá
	std::ostringstream os;
	std::copy(v.begin(), v.end(), std::ostream_iterator<std::string>(os, delim));

	std::string s = os.str(); // "a,b,c,"

	// ññîˆãÊêÿÇËï∂éöçÌèú
	s.erase(s.size() - std::char_traits<char>::length(delim));

	return s;
}

static int findLineInRange(std::vector<std::string> sdpLines, int startLine, int endLine, std::string prefix, std::string substr)
{
	size_t realEndLine = endLine != -1 ? endLine : sdpLines.size();
	std::transform(substr.begin(), substr.end(), substr.begin(), ::tolower);
	for (size_t i = startLine; i < realEndLine; ++i) {
		if (sdpLines[i].find(prefix) == 0) {
			std::transform(sdpLines[i].begin(), sdpLines[i].end(), sdpLines[i].begin(), ::tolower);
			if (sdpLines[i].find(substr) != -1) {
				return i;
			}
		}
	}
	return -1;
}

static std::string getCodecPayloadTypeFromLine(std::string sdpLine)
{
	std::cmatch m;
	if (!std::regex_match(sdpLine.c_str(), m, std::regex("a=rtpmap:(\\d+) \\w+\\/\\d+"))) {
		return std::string();
	}
	if (m.size() != 2) {
		return std::string();
	}
	return m[1].str();
}

static std::string getCodecPayloadType(std::vector<std::string> sdpLines, std::string codec)
{
	int index = findLineInRange(sdpLines, 0, -1, "a=rtpmap", codec);
	return index ? getCodecPayloadTypeFromLine(sdpLines[index]) : std::string();
}

static std::string setDefaultCodec(std::string mLine, std::string payload)
{
	std::vector<std::string> elements = split(mLine, " ");
	std::vector<std::string>::iterator bg = elements.begin();
	std::vector<std::string>::iterator en = bg + 3;
	std::vector<std::string>* newLine = new std::vector<std::string>();
	for (std::vector<std::string>::iterator it = bg; it != en; ++it) {
		newLine->push_back(*it);
	}
	newLine->push_back(payload);
	for (int i = 3; i < (int)elements.size(); i++) {
		if (elements[i] != payload) {
			newLine->push_back(elements[i]);
		}
	}
	return join(*newLine, " ");
}

static std::string maybePreferCodec(std::string sdp, std::string dir, std::string codec)
{
	std::vector<std::string> sdpLines = split(sdp, "\r\n");
	int mLineIndex = findLineInRange(sdpLines, 0, -1, "m=", "video");
	if (mLineIndex == -1) {
		return sdp;
	}
	std::string payload = getCodecPayloadType(sdpLines, codec);
	if (!payload.empty()) {
		sdpLines[mLineIndex] = setDefaultCodec(sdpLines[mLineIndex], payload);
	}
	sdp = join(sdpLines, "\r\n");
	return sdp;
}

void CBizAccess::GotRemoteSdp(LPCSTR sdp, LPCSTR type, LPCSTR roomid, LPCSTR clientid, LPCSTR remoteid)
{
	webrtc::SdpParseError error;
	std::string newSdp = maybePreferCodec(std::string(sdp), std::string("send"), std::string("VP9"));
	std::shared_ptr<ExRTCSessionDescription> s = std::make_shared<ExRTCSessionDescription>(std::string(type), newSdp, &error);
	if (!strcmp(type, "answer")) {
		std::shared_ptr<MyRTCPeerConnection> pc = peerConnectionMap[std::string(clientid)];
		MyRTCPeerConnection* peerConnection = pc.get();
		peerConnection->setRemoteDescription(s,
			[this]() {
				if (!offerId.empty()) {
					std::shared_ptr<MyRTCPeerConnection> pc = peerConnectionMap[offerId];
					MyRTCPeerConnection* peerConnection = pc.get();
					webrtc::SdpParseError error;
					std::vector<std::shared_ptr<ExRTCIceCandidate>>::const_iterator it;
					for (it = offerCandidates.begin(); it != offerCandidates.end(); ++it) {
						peerConnection->addIceCandidate(*it);
					}
					offerId.clear();
					offerCandidates.clear();
				}
			},
			[](std::shared_ptr<ExRTCError> e) {
			});
		if (getUserMediaSuccessCallBack) {
			getUserMediaSuccessCallBack(clientid);
		}
	}
	else {
		offerSdp = s;
		offerId = clientid;
	}
}

void CBizAccess::GotRemoteCandidate(int label, LPCSTR id, LPCSTR candidate, LPCSTR roomid, LPCSTR clientid, LPCSTR remoteid)
{
	std::shared_ptr<MyRTCPeerConnection> pc = peerConnectionMap[std::string(clientid)];
	MyRTCPeerConnection* peerConnection = pc.get();
	webrtc::SdpParseError error;
	std::shared_ptr<ExRTCIceCandidate> c = std::make_shared<ExRTCIceCandidate>(id, label, candidate, &error);
	if (offerId.empty()) {
		peerConnection->addIceCandidate(c);
	}
	else {
		offerCandidates.push_back(c);
	}
}

void CBizAccess::GotRemoteEoic(LPCSTR roomid, LPCSTR clientid, LPCSTR remoteid)
{
	if (offerId == clientid) {
		std::shared_ptr<MyRTCPeerConnection> ex(new MyRTCPeerConnection(rtcConf.get(), mediaConstraints.get(), localStream, false, remoteid));
		peerConnectionMap[std::string(clientid)] = ex;
		MyRTCPeerConnection* peerConnection = ex.get();
		peerConnection->setRemoteDescription(offerSdp,
			[this]() {
				if (!offerId.empty()) {
					std::shared_ptr<MyRTCPeerConnection> pc = peerConnectionMap[offerId];
					MyRTCPeerConnection* peerConnection = pc.get();
					webrtc::SdpParseError error;
					std::vector<std::shared_ptr<ExRTCIceCandidate>>::const_iterator it;
					for (it = offerCandidates.begin(); it != offerCandidates.end(); ++it) {
						peerConnection->addIceCandidate(*it);
					}
					offerId.clear();
					offerCandidates.clear();
				}
			},
			[](std::shared_ptr<ExRTCError> e) {
			});
		std::shared_ptr<webrtc::PeerConnectionInterface::RTCOfferAnswerOptions> options = make_shared<webrtc::PeerConnectionInterface::RTCOfferAnswerOptions>(1, 1, false, false, false);
		peerConnection->createAnswer(std::function<void(std::shared_ptr<ExRTCSessionDescription> e)>([](std::shared_ptr<ExRTCSessionDescription> e) {
			std::map<std::string, std::shared_ptr<MyRTCPeerConnection>>::const_iterator it;
				for (it = CBizAccess::peerConnectionMap.begin(); it != CBizAccess::peerConnectionMap.end(); it++) {
					if (it->second != nullptr) {
						MyRTCPeerConnection* conn = it->second.get();
						conn->setLocalDescription(e, std::function<void()>([]() {
						}),
							std::function<void(std::shared_ptr<ExRTCError> e)>([](std::shared_ptr<ExRTCError> e) {
						}));
						ExRTCSessionDescription* sd = e.get();
						if (CBizAccess::sendSignalingMessageCallBack) {
							std::string s1 = "\r\n";
							std::string s2 = "\\r\\n";
							std::string sdp = maybePreferCodec(sd->sdp(), std::string("receive"), std::string("VP9"));
							//std::string sdp = sd->sdp();
							int pos = (int)sdp.find(s1);
							while (pos >= 0) {
								sdp.replace(pos, s1.length(), s2);
								pos = (int)sdp.find(s1, pos + s2.length());
							}
							std::string msg = "{\"sdp\":\"" + sdp + "\",\"type\":\"" + sd->type() + "\",\"name\":\"\"}";
							CBizAccess::sendSignalingMessageCallBack(it->first.c_str(), msg.c_str());
						}
					}
				}
			}),	std::function<void(std::shared_ptr<ExRTCError> e)>([](std::shared_ptr<ExRTCError> e) {
			}), options);
		if (getUserMediaSuccessCallBack) {
			getUserMediaSuccessCallBack(clientid);
		}
	}
}

std::map<std::string, UINT_PTR> MyRTCPeerConnection::m_hTimer;

MyRTCPeerConnection::MyRTCPeerConnection(RTCConfiguration* rtcConfiguration_, const MediaConstraintSets* mediaConstraints, std::shared_ptr<ExMediaStream> stream, bool offer, std::string remoteid) :
	ExRTCPeerConnection(rtcConfiguration_, mediaConstraints),
	MyDisplay(),
	remoteId(remoteid)
{
	Handle();

	onicecandidateSet(std::function<void(std::shared_ptr<ExRTCPeerConnectionIceEvent> e)>([this](std::shared_ptr<ExRTCPeerConnectionIceEvent> e) {
		ExRTCPeerConnectionIceEvent* evt = e.get();
		std::shared_ptr<ExRTCIceCandidate> c = evt->candidate();
		ExRTCIceCandidate* candidate = c.get();

		if (CBizAccess::sendSignalingMessageCallBack) {
			std::string s1 = "\r\n";
			std::string s2 = "\\r\\n";
			std::string cnd = candidate->candidate();
			int pos = (int)cnd.find(s1);
			while (pos >= 0) {
				cnd.replace(pos, s1.length(), s2);
				pos = (int)cnd.find(s1, pos + s2.length());
			}
			std::string msg = "{\"type\":\"candidate\", \"label\":\"" + std::to_string(candidate->sdpMLineIndex()) + "\",\"id\":\"" + candidate->sdpMid() + "\",\"candidate\":\"" + cnd + "\"}";
			CBizAccess::sendSignalingMessageCallBack(remoteId.c_str(), msg.c_str());
			if (m_hTimer[remoteId]) {
				KillTimer(m_hWnd, m_hTimer[remoteId]);
			}
			UINT_PTR idEvent = std::atoi(remoteId.c_str());
			std::string remoteid = remoteId;
			UINT_PTR hTimer = SetTimer(m_hWnd, idEvent, 200, [](HWND hWnd, UINT msg, UINT_PTR idEvent, DWORD lparam) {
				std::map<std::string, UINT_PTR>::const_iterator it;
				for (it = MyRTCPeerConnection::m_hTimer.begin(); it != MyRTCPeerConnection::m_hTimer.end(); ++it) {
					if (it->second == idEvent) {
						std::string msg = "{\"type\":\"eoic\",\"remoteid\":\"" + CBizAccess::clientId + "\"}";
						CBizAccess::sendSignalingMessageCallBack(it->first.c_str(), msg.c_str());
						KillTimer(hWnd, idEvent);
						MyRTCPeerConnection::m_hTimer.erase(it->first);
						break;
					}
				}
			});
			m_hTimer[remoteId] = hTimer;
		}
	}));

	onaddstreamSet(std::function<void(std::shared_ptr<ExMediaStreamEvent> e)>([this](std::shared_ptr<ExMediaStreamEvent> e) {
		ExMediaStreamEvent* se = e.get();
		AttachMediaStream(se->stream());
	}));

	onremovestreamSet(std::function<void(std::shared_ptr<ExMediaStreamEvent> e)>([](std::shared_ptr<ExMediaStreamEvent> e) {
	}));

	onsignalingstatechangeSet(std::function<void()>([](){
	}));

	oniceconnectionstatechangeSet(std::function<void()>([](){
	}));

	ondatachannelSet(std::function<void(std::shared_ptr<ExRTCDataChannelEvent> e)>([](std::shared_ptr<ExRTCDataChannelEvent> e) {
	}));

	addStream(stream);

	if (offer) {
		std::shared_ptr<webrtc::PeerConnectionInterface::RTCOfferAnswerOptions> options = make_shared<webrtc::PeerConnectionInterface::RTCOfferAnswerOptions>(1, 1, false, false, false);
		createOffer(
			std::function<void(std::shared_ptr<ExRTCSessionDescription> e)>([this](std::shared_ptr<ExRTCSessionDescription> e) {
				setLocalDescription(e, std::function<void()>([](){
					}),
					std::function<void(std::shared_ptr<ExRTCError> e)>([](std::shared_ptr<ExRTCError> e) {
					}));
				ExRTCSessionDescription* sd = e.get();
				if (CBizAccess::sendSignalingMessageCallBack) {
					std::string s1 = "\r\n";
					std::string s2 = "\\r\\n";
					std::string sdp = maybePreferCodec(sd->sdp(), std::string("receive"), std::string("VP9"));
					//std::string sdp = sd->sdp();
					int pos = (int)sdp.find(s1);
					while (pos >= 0) {
						sdp.replace(pos, s1.length(), s2);
						pos = (int)sdp.find(s1, pos + s2.length());
					}
					std::string msg = "{\"sdp\":\"" + sdp + "\",\"type\":\"" + sd->type() + "\",\"name\":\"\"}";
					CBizAccess::sendSignalingMessageCallBack(remoteId.c_str(), msg.c_str());
				}
			}),	std::function<void(std::shared_ptr<ExRTCError> e)>([](std::shared_ptr<ExRTCError> e) {
			}), options);
	}
}

MyRTCPeerConnection::~MyRTCPeerConnection()
{
	std::map<std::string, UINT_PTR>::const_iterator it;
	for (it = m_hTimer.begin(); it != m_hTimer.end(); ++it)
	{
		::KillTimer(m_hWnd, it->second);
	}
	//m_hTimer.clear();
}
