#pragma once
using namespace std;
#include "Display.h"
#include "ExErrorMessage.h"
#include "ExMediaStream.h"
#include "ExRTCPeerConnection.h"

typedef void(*GetUserMediaSuccessCallBack)(LPCSTR);
typedef void(*GetUserMediaErrorCallBack)(LPCSTR);
typedef void(*SendSignalingMessageCallBack)(LPCSTR, LPCSTR);

class MyDisplay : public Display
{
public:
	HWND m_hWnd;
	BOOL m_bVideoRendererStarted;

public:
	MyDisplay();
	~MyDisplay() override;

	void OnStartVideoSink();
	void OnStopVideoSink();
	void AttachMediaStream(std::shared_ptr<ExMediaStream> exMediaStream);

	HWND Handle();

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

class MyRTCPeerConnection : public ExRTCPeerConnection, public MyDisplay
{
private:
	std::string remoteId;

public:
	static std::map<std::string, UINT_PTR> m_hTimer;

public:
	MyRTCPeerConnection(RTCConfiguration* rtcConfiguration_, const MediaConstraintSets* mediaConstraints, std::shared_ptr<ExMediaStream> stream, bool offer, std::string remoteid);
	virtual ~MyRTCPeerConnection();
};

class CBizAccess : public MyDisplay
{
private:
	HWND m_hTargetWnd;
	static GetUserMediaSuccessCallBack getUserMediaSuccessCallBack;
	static GetUserMediaErrorCallBack getUserMediaErrorCallBack;
	std::string offerId;

public:
	static SendSignalingMessageCallBack sendSignalingMessageCallBack;
	static std::map<std::string, std::shared_ptr<MyRTCPeerConnection>> peerConnectionMap;
	static const std::string bizAccessHost;
	static const std::string bizAccessPort;
	static const std::string bizAccessWssUrl;

	static std::string clientId;
	LPSTR lpIceServers;
	std::vector<std::shared_ptr<ExRTCIceCandidate>> offerCandidates;
	std::shared_ptr<ExRTCSessionDescription> offerSdp;
	std::shared_ptr<ExMediaStream> localStream;
	std::shared_ptr<RTCConfiguration> rtcConf;
	std::shared_ptr<MediaConstraintSets> mediaConstraints;

public:
	CBizAccess(HWND hWnd, LPCSTR roomId, LPCSTR clientId, LPCSTR iceServers, LPCSTR remoteUsers,
		GetUserMediaSuccessCallBack successFunc, GetUserMediaErrorCallBack errorfuncp, SendSignalingMessageCallBack messagefuncp);
	~CBizAccess();

	void GetUserMedia();
	void GotRemoteSdp(LPCSTR sdp, LPCSTR type, LPCSTR roomid, LPCSTR clientid, LPCSTR remoteid);
	void GotRemoteCandidate(int label, LPCSTR id, LPCSTR candidate, LPCSTR roomid, LPCSTR clientid, LPCSTR remoteid);
	void GotRemoteEoic(LPCSTR roomid, LPCSTR clientid, LPCSTR remoteid);
};
