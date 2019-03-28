#pragma once
#include "stdafx.h"

#include <hash_map>
#include <windows.h>
#include <commctrl.h>
#include <atlcore.h>

BOOL FindAddressBar(HWND mainWnd, HWND* addressBarWnd, HWND* cmdTargetWnd);
BOOL InjectDll(HINSTANCE plugin,  DWORD procID);

class CCommChannel
{

public:

	static const DWORD SECTION_SIZE = 1024;

	enum RWStates
	{

		REQUEST_AVAILABLE,
		RESPONSE_AVAILABLE,
		SERVER_AVAILABLE,
		STATE_COUNT

	};

private:

	HANDLE _section;

	HANDLE _events[ STATE_COUNT ];

	BOOL _first;

public:

	CCommChannel(TCHAR* baseName, DWORD processID);
	~CCommChannel();

	BOOL Read(LPVOID data, DWORD dataSize, BOOL response = FALSE);
	BOOL Write(LPVOID data, DWORD dataSize, BOOL response = FALSE);

	BOOL SetReady() { return ::SetEvent( _events[ SERVER_AVAILABLE ] ); }
	BOOL IsFirst() { return _first; }

};

struct Command
{
	UINT _type;

	Command() : _type(0x00) { }
	Command(UINT type) : _type(type) { }
};

struct MessageCmd : public Command
{
	UINT _uMsg;
	WPARAM _wParam;
	LPARAM _lParam;

	MessageCmd() : _uMsg(0), _wParam(0), _lParam(0) { }
	MessageCmd(UINT type, UINT uMsg, WPARAM wParam, LPARAM lParam) : Command(type), _uMsg(uMsg), _wParam(wParam), _lParam(lParam) { }
	MessageCmd(CCommChannel* channel) { channel->Read( &_type, sizeof( MessageCmd ) ); }
};

struct SendMessageCmd : public MessageCmd
{
	static const UINT CMD_TYPE = 0x01;
	SendMessageCmd(UINT uMsg, WPARAM wParam, LPARAM lParam) : MessageCmd( CMD_TYPE, uMsg, wParam, lParam ) { }
	SendMessageCmd(CCommChannel* channel) : MessageCmd(channel) { }
};

struct PostMessageCmd : public MessageCmd
{
	static const UINT CMD_TYPE = 0x02;
	PostMessageCmd(UINT uMsg, WPARAM wParam, LPARAM lParam) : MessageCmd( CMD_TYPE, uMsg, wParam, lParam ){ }
	PostMessageCmd(CCommChannel* channel) : MessageCmd(channel) { }
};

struct TabCmd : public Command
{
	DWORD _processID;
	INT_PTR _proxyClient;

	TabCmd(UINT type) : Command(type) { }
	TabCmd(UINT type, DWORD processID, INT_PTR proxyClient) : Command(type), _processID(processID), _proxyClient(proxyClient) { }
	TabCmd(CCommChannel* channel) { channel->Read( &_type, sizeof( TabCmd ) ); }
};

struct LoadCmd : public TabCmd
{
	HWND _addressBarWnd;
	HWND _cmdTargetWnd;

	static const UINT CMD_TYPE = 0x03;
	LoadCmd(HWND addressBarWnd, HWND cmdTargetWnd, DWORD processID, INT_PTR proxyClient) : TabCmd(CMD_TYPE, processID, proxyClient),
		_addressBarWnd(addressBarWnd), _cmdTargetWnd(cmdTargetWnd) { }
	LoadCmd(CCommChannel* channel) : TabCmd(CMD_TYPE) { channel->Read( &_type, sizeof( LoadCmd ) ); }
};

struct UnloadCmd : public TabCmd
{
	static const UINT CMD_TYPE = 0x04;
	UnloadCmd(DWORD processID, INT_PTR proxyClient) : TabCmd(CMD_TYPE, processID, proxyClient) { }
	UnloadCmd(CCommChannel* channel) : TabCmd(channel) { }
};

struct SelectTabCmd : public TabCmd
{
	static const UINT CMD_TYPE = 0x05;

	SelectTabCmd(DWORD processID, INT_PTR proxyClient) : TabCmd(CMD_TYPE, processID, proxyClient) { }
	SelectTabCmd(CCommChannel* channel) : TabCmd(channel) { }
};

class CAddressBarAccessServer
{

public:

	typedef LRESULT (CALLBACK *WndProcPtr)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	typedef std::pair<CCommChannel*, LONG> ClienMessageHandlersEntry;
	typedef stdext::hash_map<DWORD, ClienMessageHandlersEntry> ClienMessageHandlersMap;

private:

	static LONG _referenceCount;
	static ATL::CComAutoCriticalSection  _instanceLock;
	static CAddressBarAccessServer* _instance;

	HWND _addressBarWnd;
	HWND _cmdTargetWnd;

	HIMAGELIST _imageList;
	HIMAGELIST _hotImageList;
	HIMAGELIST _pressedImageList;

	struct ButtonImages
	{
		HICON _image;
		HICON _hotImage;
		HICON _pressedImage;

		ButtonImages() : _image(NULL), _hotImage(NULL), _pressedImage(NULL) { }
		ButtonImages(HICON image, HICON hotImage, HICON pressedImage) : _image(image), _hotImage(hotImage), _pressedImage(pressedImage) { }

		void Destroy()
		{
			::DestroyIcon(_image);
			::DestroyIcon(_hotImage);
			::DestroyIcon(_pressedImage);

			_image = _hotImage = _pressedImage = NULL;
		}
	};

	typedef stdext::hash_map<WORD, ButtonImages> ButtonsMap;

	ButtonsMap _buttons;

	WndProcPtr _oldCmdTargetWndProc;
	WndProcPtr _oldAddrBarWndProc;

	CCommChannel* _channel;

	ATL::CComAutoCriticalSection _clientHandlersLock;
	ClienMessageHandlersMap _clientMessageHandlers;

	DWORD _tabCounter;

	DWORD _currentProcessID;
	UINT_PTR _currentProxyClient;

public:

	static BOOL AddRef(BOOL startListner = FALSE);
	static BOOL RemoveRef();
	static CAddressBarAccessServer* GetInstance() { return _instance; }

	CAddressBarAccessServer(BOOL startListner);

	void Load(HWND addressBarWnd, HWND cmdTargetWnd, DWORD processID, UINT_PTR proxyClient);
	void Unload(DWORD processID, UINT_PTR proxyClient);
	void SetCurrentProxy(DWORD processID, UINT_PTR proxyClient);

	LRESULT SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) { return ::SendMessage( _addressBarWnd, uMsg, wParam, lParam ); }
	LRESULT PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) { return ::PostMessage( _addressBarWnd, uMsg, wParam, lParam ); }

	BOOL CmdTargetWndProc(LRESULT* lResult, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL AddrBarWndProc(LRESULT* lResult, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:

	void InitButtons();
	void AddButton(WORD id, HICON image, HICON hotImage, HICON pressedImage);

	static DWORD WINAPI ProxyListen(LPVOID param);

	static LRESULT CALLBACK CmdTargetWndProcS(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK AddrBarWndProcS(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

};

struct ForwardedMessage
{
	UINT_PTR _proxyClient;
	UINT _uMsg;
	WPARAM _wParam;
	LPARAM _lParam;

	ForwardedMessage() { }
	ForwardedMessage(UINT_PTR proxyClient, UINT uMsg, WPARAM wParam, LPARAM lParam) : _proxyClient(proxyClient), _uMsg(uMsg), _wParam(wParam), _lParam(lParam) { }
};

class CAddressBarAccessProxy
{

private:

	CAddressBarAccessServer* _server;

	CCommChannel* _cmdChannel;

	CCommChannel* _msgChannel;

public:

	CAddressBarAccessProxy(HINSTANCE bhoDll, HWND addressBarWnd, HWND cmdTargetWnd);
	virtual ~CAddressBarAccessProxy();

	void SetCurrent();

	LRESULT SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual BOOL CmdTargetWndProc(LRESULT* lResult, UINT uMsg, WPARAM wParam, LPARAM lParam) { return 0; }

private:

	static DWORD WINAPI MessageHandlerListner(LPVOID param);

};
