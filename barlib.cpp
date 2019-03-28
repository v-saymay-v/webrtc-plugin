
#include "stdafx.h"

#include <tchar.h>
#include <windows.h>
#include <atlbase.h>

#include "resource.h"
#include "barlib.h"

BOOL FindAddressBar(HWND mainWnd, HWND* addressBarWnd, HWND* cmdTargetWnd)
{
	mainWnd = ::FindWindowEx( mainWnd, NULL, TEXT( "WorkerW" ), NULL );
	mainWnd = ::FindWindowEx( mainWnd, NULL, TEXT( "ReBarWindow32" ), NULL );

	*cmdTargetWnd = ::FindWindowEx( mainWnd, NULL, TEXT( "Address Band Root" ), NULL );

	if( *cmdTargetWnd  )
		*addressBarWnd = ::FindWindowEx( *cmdTargetWnd, NULL, TEXT( "ToolbarWindow32" ), L"Page Control" );

	return cmdTargetWnd != NULL;
}

BOOL InjectDll(HINSTANCE bhoDll, DWORD procID)
{
	STARTUPINFO startInfo;
	::ZeroMemory( &startInfo, sizeof( startInfo ) );
	startInfo.cb = sizeof( startInfo );
	startInfo.dwFlags |= STARTF_USESHOWWINDOW;
	startInfo.wShowWindow = FALSE;

	PROCESS_INFORMATION processInfo;
	::ZeroMemory( &processInfo, sizeof( processInfo ) );

	TCHAR params[ MAX_PATH ];
	_itow_s( procID, params, MAX_PATH, 10 );

	TCHAR path[ MAX_PATH ];
	if( !::GetModuleFileName( bhoDll, path, MAX_PATH ) )
		return FALSE;

#ifdef UNICODE
	wchar_t* sp = wcsrchr( path, L'\\' ) + 1;
#elif
	char* sp = strrchr( path, '\\' ) + 1; 
#endif

	lstrcpy( sp, TEXT( "ieb_start.exe" ) );

	if( !::CreateProcess( path, params, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startInfo, &processInfo ) )
		return FALSE;

	::WaitForSingleObject( processInfo.hProcess, INFINITE );

	::CloseHandle( processInfo.hThread );
	::CloseHandle( processInfo.hProcess );

	return TRUE;
}

CCommChannel::CCommChannel(TCHAR* baseName, DWORD processID)
{
	TCHAR name[ MAX_PATH ];

	int len = 0;
	for( int i = 0; i < STATE_COUNT; i++ )
	{
		lstrcpy( name, baseName );

		len = lstrlen( name );
		lstrcpy( name + len, TEXT( "Event" ) );

		len = lstrlen( name );
		_itot_s( processID, name + len, MAX_PATH - len, 10 );

		len = lstrlen( name );
		_tcscpy_s( name + len, MAX_PATH - len, TEXT( "_" ) );

		len = lstrlen( name );
		_itot_s( i, name + len, MAX_PATH - len, 10 );

		_events[ i ] = ::CreateEvent( NULL, FALSE, i == SERVER_AVAILABLE, name );
	}

	_first = ::GetLastError() != ERROR_ALREADY_EXISTS;

	lstrcpy( name, TEXT( "Local\\" ) );

	len = lstrlen( name );
	lstrcpy( name + len, baseName );

	len = lstrlen( name );
	lstrcpy( name + len, TEXT( "Sect" ) );

	len = lstrlen( name );
	_itot_s( processID, name + len, MAX_PATH - len, 10 );

	_section = _first ? ::CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SECTION_SIZE, name ) : ::OpenFileMapping( FILE_MAP_ALL_ACCESS, FALSE, name );
}

CCommChannel::~CCommChannel()
{
	for( int i = 0; i < STATE_COUNT; i++ )
		::CloseHandle( _events[ i ] );

	::CloseHandle( _section );
}

BOOL CCommChannel::Read(LPVOID data, DWORD dataSize, BOOL response/* = FALSE*/)
{
	::WaitForSingleObject( _events[ response ? RESPONSE_AVAILABLE : REQUEST_AVAILABLE ], INFINITE );

	LPVOID source = ::MapViewOfFile( _section, FILE_MAP_ALL_ACCESS, 0, 0, dataSize );
	if( !source )
	{
		if( !response )
			::SetEvent( _events[ SERVER_AVAILABLE ] );
		return FALSE;
	}

	::CopyMemory( data, source, dataSize );
	BOOL ok = ::UnmapViewOfFile( source );

	if( !response )
		::SetEvent( _events[ SERVER_AVAILABLE ] );
	return ok;
}

BOOL CCommChannel::Write(LPVOID data, DWORD dataSize, BOOL response/* = FALSE*/)
{
	if( !response )
		::WaitForSingleObject( _events[ SERVER_AVAILABLE ], INFINITE );

	LPVOID destination = ::MapViewOfFile( _section, FILE_MAP_ALL_ACCESS, 0, 0, dataSize );
	if( !destination )
	{
		if( !response )
			::SetEvent( _events[ SERVER_AVAILABLE ] );
		return FALSE;
	}

	::CopyMemory( destination, data, dataSize );
	if( ::UnmapViewOfFile( destination ) )
	{
		::SetEvent( _events[ response ? RESPONSE_AVAILABLE : REQUEST_AVAILABLE ] );
		return TRUE;
	}
	else
	{
		::SetEvent( _events[ response ? RESPONSE_AVAILABLE : SERVER_AVAILABLE ] );
		return FALSE;
	}
}

LONG CAddressBarAccessServer::_referenceCount = 0;
CAddressBarAccessServer* CAddressBarAccessServer::_instance = NULL;
ATL::CComAutoCriticalSection CAddressBarAccessServer::_instanceLock;

BOOL CAddressBarAccessServer::AddRef(BOOL startListner/* = FALSE*/)
{
	ATL::CComCritSecLock<CComAutoCriticalSection> lock( _instanceLock, true );

	if( ++_referenceCount == 1 )
	{
		_instance = new CAddressBarAccessServer( startListner );

		return TRUE;
	}

	return FALSE;
}

BOOL CAddressBarAccessServer::RemoveRef()
{
	ATL::CComCritSecLock<CComAutoCriticalSection> lock( _instanceLock, true );

	if( --_referenceCount == 0 )
	{
		delete _instance;
		_instance = NULL;

		return TRUE;
	}

	return FALSE;
}

CAddressBarAccessServer::CAddressBarAccessServer(BOOL startListner/* = FALSE*/) : _channel(NULL), _tabCounter(0), _currentProcessID(0), _currentProxyClient(NULL)
{
	if( startListner )
	{
		HANDLE thread = ::CreateThread( NULL, 0, ProxyListen, this, 0, NULL );
		::CloseHandle( thread );
	}
}

void CAddressBarAccessServer::Load(HWND addressBarWnd, HWND cmdTargetWnd, DWORD processID, UINT_PTR proxyClient)
{
	ATL::CComCritSecLock<CComAutoCriticalSection> lock( _clientHandlersLock, true );

	if( processID != ::GetCurrentProcessId() && _clientMessageHandlers.find( processID ) == _clientMessageHandlers.end() )
	{
		// add IPC channel for proxy if it is in a different process
		_clientMessageHandlers[ processID ] = ClienMessageHandlersEntry( new CCommChannel( TEXT( "IeBarMsgPoint" ), processID ), 1 );
		if( _clientMessageHandlers.size() == 1 )
		{
			_currentProcessID = processID;
			_currentProxyClient = proxyClient;
		}
	}

	// do it only the first tab of the browser is initialized
	if( ++_tabCounter == 1 )
	{
		_addressBarWnd = addressBarWnd;
		_cmdTargetWnd = cmdTargetWnd;

		// subclass windows
		_oldAddrBarWndProc = (WndProcPtr)::GetWindowLongPtr( _addressBarWnd, GWLP_WNDPROC );
		::SetWindowLongPtr( _addressBarWnd, GWLP_WNDPROC, (LONG_PTR)AddrBarWndProcS );
		_oldCmdTargetWndProc = (WndProcPtr)::GetWindowLongPtr( _cmdTargetWnd, GWLP_WNDPROC );
		::SetWindowLongPtr( _cmdTargetWnd, GWLP_WNDPROC, (LONG_PTR)CmdTargetWndProcS );
		
		// get toolbar's image lists
		_imageList = (HIMAGELIST)::SendMessage( addressBarWnd, TB_GETIMAGELIST, (WPARAM)0, (LPARAM)0 );
		_hotImageList = (HIMAGELIST)::SendMessage( addressBarWnd, TB_GETHOTIMAGELIST, (WPARAM)0, (LPARAM)0 );
		_pressedImageList = (HIMAGELIST)::SendMessage( addressBarWnd, TB_GETPRESSEDIMAGELIST, (WPARAM)0, (LPARAM)0 );

		// add required buttons
		InitButtons();

		lock.Unlock();

		// refreshes size of the toolbar
		::SendMessage( addressBarWnd, WM_SIZE, 0, 0 );
		::SendMessage( cmdTargetWnd, WM_SIZE, 0, 0 );
	}
}

void CAddressBarAccessServer::Unload(DWORD processID, UINT_PTR proxyClient)
{
	ATL::CComCritSecLock<CComAutoCriticalSection> lock( _clientHandlersLock, true );

	if( processID != ::GetCurrentProcessId() )
	{
		// destory IPC channel between proxy and server if they are in different processes
		ClienMessageHandlersEntry& entry = _clientMessageHandlers[ processID ];
		if( --entry.second == 0 )
		{
			delete entry.first;
			_clientMessageHandlers.erase( processID );
		}
	}

	// if there's no more tabs when should clear changes made to toolbar
	if( --_tabCounter == 0 )
	{
		// reverese subclassing
		::SetWindowLongPtr( _addressBarWnd, GWLP_WNDPROC, (LONG_PTR)_oldAddrBarWndProc );
		::SetWindowLongPtr( _cmdTargetWnd, GWLP_WNDPROC, (LONG_PTR)_oldCmdTargetWndProc );

		_addressBarWnd = _cmdTargetWnd = NULL;

		// remove buttons
		for( ButtonsMap::iterator it = _buttons.begin(); it != _buttons.end(); ++it )
			it->second.Destroy();

		_buttons.clear();

		// destory IPC channel which receives requests
		if( _channel )
		{
			delete _channel;
			_channel = NULL;
		}
	}
}

void CAddressBarAccessServer::SetCurrentProxy(DWORD processID, UINT_PTR proxyClient)
{
	ATL::CComCritSecLock<CComAutoCriticalSection> lock( _clientHandlersLock, true );

	// store current proxy
	_currentProcessID = processID;
	_currentProxyClient = proxyClient;
}

BOOL CAddressBarAccessServer::CmdTargetWndProc(LRESULT* lResult, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch( uMsg )
	{
		/* TODO: ADD TOOLBAR COMMAND MESSAGE HANDLING THAT IS EXECUTED WITHIN THE PROCESS WHICH HOSTS TOOLBAR */
		/* RETURN TRUE IF MESSAGE SHOULD NOT BE PROCESSED FURTHER */
	default:
		break;
	}

	if( uMsg == WM_COMMAND )
	{
		// should we use IPC or access client proxy directly
		if( _currentProcessID != ::GetCurrentProcessId() )
		{
			ATL::CComCritSecLock<CComAutoCriticalSection> lock( _clientHandlersLock, true );

			// send notification to listening thread of process which owns client proxy
			ClienMessageHandlersMap::iterator it = _clientMessageHandlers.find( _currentProcessID );
			if( it != _clientMessageHandlers.end() )
			{
				ForwardedMessage msg( _currentProxyClient, uMsg, wParam, lParam );
				it->second.first->Write( &msg, sizeof( msg ), FALSE );
			}
		}
		else
			( (CAddressBarAccessProxy*)_currentProxyClient )->CmdTargetWndProc( lResult, uMsg, wParam, lParam );
	}

	return FALSE;
}

BOOL CAddressBarAccessServer::AddrBarWndProc(LRESULT* lResult, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch( uMsg )
	{
		/* TODO: ADD TOOLBAR MESSAGE HANDLING THAT IS EXECUTED WITHIN THE PROCESS WHICH HOSTS TOOLBAR */
		/* RETURN TRUE IF MESSAGE SHOULD NOT BE PROCESSED FURTHER */
	}

	return FALSE;
}

void CAddressBarAccessServer::InitButtons()
{
	HINSTANCE module;
	GetModuleHandleEx( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)( &CAddressBarAccessServer::ProxyListen ), &module );

	/* INSERT BUTTONS TO TOOLBAR */

	HICON icon = ::LoadIcon( module, MAKEINTRESOURCE( IDI_ICON1 ) );
	HICON hotIcon = ::LoadIcon( module, MAKEINTRESOURCE( IDI_ICON2 ) );
	HICON pressedIcon = ::LoadIcon( module, MAKEINTRESOURCE( IDI_ICON2 ) );

	AddButton( 0xc001, icon, hotIcon, pressedIcon );
}

void CAddressBarAccessServer::AddButton(WORD id, HICON image, HICON hotImage, HICON pressedImage)
{
	if( _buttons.find( id ) != _buttons.end() )
		return;

	_buttons[ id ] = ButtonImages( image, hotImage, pressedImage );

	int imageOffset = ImageList_AddIcon( _imageList, image );
	ImageList_AddIcon( _hotImageList, hotImage );
	ImageList_AddIcon( _pressedImageList, pressedImage );

	TBBUTTON button;
	memset( &button, 0, sizeof( TBBUTTON ) );

	button.fsState = TBSTATE_ENABLED;
	button.idCommand = id;
	button.iBitmap = imageOffset;
	button.fsStyle = 0;

	::SendMessage( _addressBarWnd, TB_INSERTBUTTON, 0, (LPARAM)&button );
}

DWORD CAddressBarAccessServer::ProxyListen(LPVOID param)
{
	CAddressBarAccessServer* pThis = (CAddressBarAccessServer*)param;

	pThis->_channel = new CCommChannel( TEXT( "IeBarListner" ), ::GetCurrentProcessId() );
	while( pThis->_channel )
	{
		char buffer[ CCommChannel::SECTION_SIZE ];
		if( !pThis->_channel->Read( buffer, CCommChannel::SECTION_SIZE ) )
			break;

		switch( *(UINT*)buffer )
		{

		case SendMessageCmd::CMD_TYPE:
			{
				SendMessageCmd* cmd = (SendMessageCmd*)buffer;
				pThis->SendMessage( cmd->_uMsg, cmd->_wParam, cmd->_lParam );
			}
			break;

		case PostMessageCmd::CMD_TYPE:
			{
				PostMessageCmd* cmd = (PostMessageCmd*)buffer;
				pThis->PostMessage( cmd->_uMsg, cmd->_wParam, cmd->_lParam );
			}
			break;

		case LoadCmd::CMD_TYPE:
			{
				LoadCmd* cmd = (LoadCmd*)buffer;
				pThis->Load( cmd->_addressBarWnd, cmd->_cmdTargetWnd, cmd->_processID, cmd->_proxyClient );
			}
			break;

		case UnloadCmd::CMD_TYPE:
			{
				UnloadCmd* cmd = (UnloadCmd*)buffer;
				pThis->Unload( cmd->_processID, cmd->_proxyClient );
			}
			break;

		case SelectTabCmd::CMD_TYPE:
			{
				SelectTabCmd* cmd = (SelectTabCmd*)buffer;
				pThis->SetCurrentProxy( cmd->_processID, cmd->_proxyClient );
			}
			break;

		}
	}

	return 0;
}

LRESULT CAddressBarAccessServer::CmdTargetWndProcS(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	if( _instance->CmdTargetWndProc( &result, uMsg, wParam, lParam ) )
		return result;

	return CallWindowProc( _instance->_oldCmdTargetWndProc, hwnd, uMsg, wParam, lParam );
}

LRESULT CAddressBarAccessServer::AddrBarWndProcS(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	if( _instance->AddrBarWndProc( &result, uMsg, wParam, lParam ) )
		return result;

	return CallWindowProc( _instance->_oldAddrBarWndProc, hwnd, uMsg, wParam, lParam );
}

CAddressBarAccessProxy::CAddressBarAccessProxy(HINSTANCE bhoDll, HWND addressBarWnd, HWND cmdTargetWnd) : _cmdChannel(NULL), _msgChannel(NULL)
{
	DWORD procID = 0;
	::GetWindowThreadProcessId( addressBarWnd, &procID );

	if( procID != ::GetCurrentProcessId() )
	{
		_server = NULL;

		_cmdChannel = new CCommChannel( TEXT( "IeBarListner" ), procID );
		_msgChannel = new CCommChannel( TEXT( "IeBarMsgPoint" ), ::GetCurrentProcessId() );

		if( _msgChannel->IsFirst() )
		{
			HANDLE thread = ::CreateThread( NULL, 0, MessageHandlerListner, _msgChannel, 0, NULL );
			::CloseHandle( thread );
		}

		if( _cmdChannel->IsFirst() )
			InjectDll( bhoDll, procID );

		LoadCmd cmd ( addressBarWnd, cmdTargetWnd, ::GetCurrentProcessId(), (INT_PTR)this );
		_cmdChannel->Write( &cmd, sizeof( cmd ) );
	}
	else
	{
		CAddressBarAccessServer::AddRef();
		_server = CAddressBarAccessServer::GetInstance();

		_server->Load( addressBarWnd, cmdTargetWnd, ::GetCurrentProcessId(), (INT_PTR)this );
	}
}

CAddressBarAccessProxy::~CAddressBarAccessProxy()
{
	if( _server )
	{
		if( CAddressBarAccessServer::RemoveRef() )
			delete _server;
	}
	else
	{
		UnloadCmd cmd( ::GetCurrentProcessId(), (INT_PTR)this );
		_cmdChannel->Write( &cmd, sizeof( cmd ) );
	}

	if( _cmdChannel )
		delete _cmdChannel;

	if( _msgChannel )
		delete _msgChannel;
}

void CAddressBarAccessProxy::SetCurrent()
{
	if( _server )
		_server->SetCurrentProxy( ::GetCurrentProcessId(), (INT_PTR)this );
	else
	{
		SelectTabCmd cmd( ::GetCurrentProcessId(), (INT_PTR)this );
		_cmdChannel->Write( &cmd, sizeof( cmd ) );
	}
}

LRESULT CAddressBarAccessProxy::SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( _server )
		return _server->SendMessage( uMsg, wParam, lParam );

	SendMessageCmd cmd ( uMsg, wParam, lParam );
	_cmdChannel->Write( &cmd, sizeof( cmd ) );

	LRESULT result;
	_cmdChannel->Read( &result, sizeof( result ), TRUE );

	return result;
}

LRESULT CAddressBarAccessProxy::PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( _server )
		return _server->PostMessage( uMsg, wParam, lParam );

	PostMessageCmd cmd ( uMsg, wParam, lParam );
	_cmdChannel->Write( &cmd, sizeof( cmd ) );

	return 0;
}

DWORD CAddressBarAccessProxy::MessageHandlerListner(LPVOID param)
{
	CCommChannel* channel = (CCommChannel*)param;

	char buffer[ CCommChannel::SECTION_SIZE ];
	ForwardedMessage* msg = (ForwardedMessage*)buffer;

	while(channel->Read( buffer, CCommChannel::SECTION_SIZE ) )
	{
		LRESULT result;
		( (CAddressBarAccessProxy*)msg->_proxyClient )->CmdTargetWndProc( &result, msg->_uMsg, msg->_wParam, msg->_lParam );
	}

	return 0;
}
