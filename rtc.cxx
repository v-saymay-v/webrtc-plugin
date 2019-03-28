#include "stdafx.h"
#include "resource.h"
#include "rtc_i.h"
#include "dllmain.h"

#include "CBizAccess.h"

#include "Common.h"
#include "DoubangoDesktopCapturer.h"
#include "ExRTCWindow.h"

using namespace ATL;

// Used to determine whether the DLL can be unloaded by OLE.
STDAPI DllCanUnloadNow(void)
{
	return _AtlModule.DllCanUnloadNow();
}

// Returns a class factory to create an object of the requested type.
_Check_return_
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv)
{
	return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// DllRegisterServer - Adds entries to the system registry.
STDAPI DllRegisterServer(void)
{
	// registers object, typelib and all interfaces in typelib
	HRESULT hr = _AtlModule.DllRegisterServer();
	return hr;
}

// DllUnregisterServer - Removes entries from the system registry.
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}

// DllInstall - Adds/Removes entries to the system registry per user per machine.
STDAPI DllInstall(BOOL bInstall, _In_opt_  LPCWSTR pszCmdLine)
{
	HRESULT hr = E_FAIL;
	static const wchar_t szUserSwitch[] = L"user";

	if (pszCmdLine != NULL)
	{
		if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
		{
			ATL::AtlSetPerUserRegistration(true);
		}
	}

	if (bInstall)
	{	
		hr = DllRegisterServer();
		if (FAILED(hr))
		{
			DllUnregisterServer();
		}
	}
	else
	{
		hr = DllUnregisterServer();
	}

	return hr;
}

static std::vector<std::shared_ptr<ExRTCWindow> > g_windows_list;

extern "C" __declspec(dllexport) int GetWindowsNum()
{
	if (g_windows_list.empty())
		g_windows_list = DoubangoDesktopCapturerFactory::GetWindows();
	return g_windows_list.size();
}

extern "C" __declspec(dllexport) void GetWindowList(void* param, void(*funcp)(void*, HWND, int, int, LPCSTR, LPCSTR))
{
	std::vector<std::shared_ptr<ExRTCWindow> >::const_iterator it;

	if (g_windows_list.empty())
		g_windows_list = DoubangoDesktopCapturerFactory::GetWindows();

	for (it = g_windows_list.begin(); it != g_windows_list.end(); ++it) {
		std::shared_ptr<ExRTCWindow> window = *it;
/*
		WCHAR	wStrW[256];
		size_t wLen = 0;
		errno_t err = 0;
		//ƒƒP[ƒ‹Žw’è
		setlocale(LC_ALL, "japanese");
		//•ÏŠ·
		err = mbstowcs_s(&wLen, wStrW, 20, window->title().c_str(), _TRUNCATE);
*/
		LOG(INFO) << "Window tile: " << window->title().c_str();
		funcp(param, reinterpret_cast<HWND>(window->id()), window->width(), window->height(), window->title().c_str(), window->image().c_str());
	}
}

/*
typedef struct Ratio {
	DWORD Numerator;
	DWORD Denominator;
} Ratio;

static inline LONG Width(const RECT& r)
{
	return r.right - r.left;
}

static inline LONG Height(const RECT& r)
{
	return r.bottom - r.top;
}
*/

//-----------------------------------------------------------------------------
// CorrectAspectRatio
//
// Converts a rectangle from the source's pixel aspect ratio (PAR) to 1:1 PAR.
// Returns the corrected rectangle.
//
// For example, a 720 x 486 rect with a PAR of 9:10, when converted to 1x1 PAR,
// is stretched to 720 x 540.
// Copyright (C) Microsoft
//-----------------------------------------------------------------------------
/*
static inline RECT CorrectAspectRatio(const RECT& src, const Ratio& srcPAR)
{
	// Start with a rectangle the same size as src, but offset to the origin (0,0).
	RECT rc = { 0, 0, src.right - src.left, src.bottom - src.top };

	if ((srcPAR.Numerator != 1) || (srcPAR.Denominator != 1))
	{
		// Correct for the source's PAR.

		if (srcPAR.Numerator > srcPAR.Denominator)
		{
			// The source has "wide" pixels, so stretch the width.
			rc.right = MulDiv(rc.right, srcPAR.Numerator, srcPAR.Denominator);
		}
		else if (srcPAR.Numerator < srcPAR.Denominator)
		{
			// The source has "tall" pixels, so stretch the height.
			rc.bottom = MulDiv(rc.bottom, srcPAR.Denominator, srcPAR.Numerator);
		}
		// else: PAR is 1:1, which is a no-op.
	}
	return rc;
}
*/

//-------------------------------------------------------------------
// LetterBoxDstRect
//
// Takes a src rectangle and constructs the largest possible
// destination rectangle within the specifed destination rectangle
// such thatthe video maintains its current shape.
//
// This function assumes that pels are the same shape within both the
// source and destination rectangles.
// Copyright (C) Microsoft
//-------------------------------------------------------------------
/*
static inline RECT LetterBoxRect(const RECT& rcSrc, const RECT& rcDst)
{
	// figure out src/dest scale ratios
	int iSrcWidth = Width(rcSrc);
	int iSrcHeight = Height(rcSrc);

	int iDstWidth = Width(rcDst);
	int iDstHeight = Height(rcDst);

	int iDstLBWidth;
	int iDstLBHeight;

	if (MulDiv(iSrcWidth, iDstHeight, iSrcHeight) <= iDstWidth) {

		// Column letter boxing ("pillar box")

		iDstLBWidth = MulDiv(iDstHeight, iSrcWidth, iSrcHeight);
		iDstLBHeight = iDstHeight;
	}
	else {

		// Row letter boxing.

		iDstLBWidth = iDstWidth;
		iDstLBHeight = MulDiv(iDstWidth, iSrcHeight, iSrcWidth);
	}


	// Create a centered rectangle within the current destination rect

	RECT rc;

	LONG left = rcDst.left + ((iDstWidth - iDstLBWidth) >> 1);
	LONG top = rcDst.top + ((iDstHeight - iDstLBHeight) >> 1);

	SetRect(&rc, left, top, left + iDstLBWidth, top + iDstLBHeight);

	return rc;
}
*/

extern "C" __declspec(dllexport) HBITMAP CreateThumbNail(HWND hWnd)
{
#define kWindowThumbWidth 160
#define kWindowThumbHeight 160
	HRESULT hr = S_OK;
	HDC hSrcDC = NULL;
	HDC hMemDC = NULL;
	HBITMAP hBitmap = NULL;
	HBITMAP hOldBitmap = NULL;
	LONG width, height;
	BITMAPINFOHEADER bi;
	DWORD dwBmpSize;
	void *pvBits0 = NULL;
	std::string strWindows = "";
	char windowId[120] = { 0 };
	void *np_base64_ptr = NULL;
	void *bmp_ptr = NULL;
	size_t base64_size = 0;
	size_t bmp_size = 0;
	HBRUSH hBrush = NULL;
	RECT rcSrc;
	const RECT _rcDest = { 0, 0, kWindowThumbWidth, kWindowThumbHeight };
	RECT rcDest;
	static const Ratio pixelAR = { 1, 1 };
	TCHAR strWindowClass[1024];
	fprintf(stderr, "%s(%d): enter", __FILE__, __LINE__);

	hSrcDC = ::GetDC(hWnd);
	if (!hSrcDC) {
		RTC_CHECK_HR_BAIL(hr = E_FAIL);
	}
	hMemDC = ::CreateCompatibleDC(hSrcDC);
	if (!hMemDC) {
		RTC_CHECK_HR_BAIL(hr = E_FAIL);
	}
	fprintf(stderr, "%s(%d): here", __FILE__, __LINE__);

	// get points of rectangle to grab
	if (::GetWindowRect(hWnd, &rcSrc) != TRUE) {
		RTC_CHECK_HR_BAIL(hr = E_FAIL);
	}
	fprintf(stderr, "%s(%d): here", __FILE__, __LINE__);
	width = rcSrc.right - rcSrc.left;
	height = rcSrc.bottom - rcSrc.top;

	hBitmap = ::CreateCompatibleBitmap(hSrcDC, width, height);
	if (!hBitmap) {
		RTC_CHECK_HR_BAIL(hr = E_FAIL);
	}
	fprintf(stderr, "%s(%d): here", __FILE__, __LINE__);

	GetClassName(hWnd, strWindowClass, sizeof(strWindowClass));
	fprintf(stderr, "%s(%d): here", __FILE__, __LINE__);

	// select new bitmap into memory DC
	hOldBitmap = (HBITMAP)::SelectObject(hMemDC, hBitmap);

	if (!wcscmp(strWindowClass, TEXT("Chrome_WidgetWin_1")) ||
		!wcscmp(strWindowClass, TEXT("__video_control_per__"))) {
		::PrintWindow(hWnd, hMemDC, PW_RENDERFULLCONTENT);
	}
	else {
		// bitblt screen DC to memory DC
		::BitBlt(hMemDC, 0, 0, width, height, hSrcDC, 0, 0, SRCCOPY);
	}
	fprintf(stderr, "%s(%d): here", __FILE__, __LINE__);

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;
	dwBmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;
	if (!(pvBits0 = ::VirtualAlloc(NULL, dwBmpSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE))) {
		RTC_CHECK_HR_BAIL(hr = E_OUTOFMEMORY);
	}
	fprintf(stderr, "%s(%d): here", __FILE__, __LINE__);

	// select old bitmap back into memory DC and get handle to
	// bitmap of the screen   
	hBitmap = (HBITMAP)::SelectObject(hMemDC, hOldBitmap);
	if (!hBitmap) {
		RTC_CHECK_HR_BAIL(hr = E_FAIL);
	}
	fprintf(stderr, "%s(%d): here", __FILE__, __LINE__);

	// Copy the bitmap data into the provided BYTE buffer
	if (!::GetDIBits(hSrcDC, hBitmap, 0, height, pvBits0, (BITMAPINFO *)&bi, DIB_RGB_COLORS)) {
		RTC_CHECK_HR_BAIL(hr = E_FAIL);
	}
	fprintf(stderr, "%s(%d): here", __FILE__, __LINE__);

	///////////////////////////////////////////////////////////////////////////////////////

	// Delete Objects and draw preview
	::DeleteDC(hMemDC), hMemDC = NULL;
	::DeleteObject(hBitmap), hBitmap = NULL;

	rcSrc = ::CorrectAspectRatio(rcSrc, pixelAR);
	rcDest = ::LetterBoxRect(rcSrc, _rcDest);

	hMemDC = ::CreateCompatibleDC(hSrcDC);
	if (!hMemDC) {
		RTC_CHECK_HR_BAIL(hr = E_FAIL);
	}
	fprintf(stderr, "%s(%d): here", __FILE__, __LINE__);

	hBitmap = ::CreateCompatibleBitmap(hSrcDC, kWindowThumbWidth, kWindowThumbHeight);
	if (!hBitmap) {
		RTC_CHECK_HR_BAIL(hr = E_FAIL);
	}
	fprintf(stderr, "%s(%d): here", __FILE__, __LINE__);

	hOldBitmap = (HBITMAP)::SelectObject(hMemDC, hBitmap);
	if (!hOldBitmap)
	{
		RTC_CHECK_HR_BAIL(hr = E_FAIL);
	}
	fprintf(stderr, "%s(%d): here", __FILE__, __LINE__);

	::FillRect(hMemDC, &_rcDest, hBrush);
	::SetStretchBltMode(hMemDC, HALFTONE);
	::StretchDIBits(
		hMemDC,
		rcDest.left, rcDest.top, ::Width(rcDest), ::Height(rcDest),
		rcSrc.left, rcSrc.top, ::Width(rcSrc), ::Height(rcSrc),
		pvBits0,
		(BITMAPINFO *)&bi,
		DIB_RGB_COLORS,
		SRCCOPY);
	fprintf(stderr, "%s(%d): here", __FILE__, __LINE__);

	hBitmap = (HBITMAP)::SelectObject(hMemDC, hOldBitmap);
	fprintf(stderr, "%s(%d): here", __FILE__, __LINE__);

bail:
	// Cleanup
	if (hSrcDC) {
		::ReleaseDC(hWnd, hSrcDC), hSrcDC = NULL;
	}
	if (hMemDC) {
		::DeleteDC(hMemDC), hMemDC = NULL;
	}
	if (pvBits0) {
		::VirtualFree(pvBits0, 0, MEM_RELEASE), pvBits0 = NULL;
	}

	return hBitmap;
}

static CBizAccess* g_bizAccess = NULL;

extern "C" __declspec(dllexport) void DllGetUserMedia(HWND hWnd, LPCSTR roomId, LPCSTR clientId, LPCSTR iceServers, LPCSTR remoteUsers,
	void(*successfuncp)(LPCSTR), void(*errorfuncp)(LPCSTR), void(*messagefuncp)(LPCSTR, LPCSTR))
{
	//if (!g_bizAccess) {
		g_bizAccess = new CBizAccess(hWnd, roomId, clientId, iceServers, remoteUsers, successfuncp, errorfuncp, messagefuncp);
	//}
	g_bizAccess->GetUserMedia();
}

extern "C" __declspec(dllexport) void DllGotRemoteSdp(LPCSTR sdp, LPCSTR type, LPCSTR roomid, LPCSTR clientid, LPCSTR remoteid)
{
	if (g_bizAccess/*[std::string(clientid)]*/)
	{
		g_bizAccess/*[std::string(clientid)]*/->GotRemoteSdp(sdp, type, roomid, clientid, remoteid);
	}
}

extern "C" __declspec(dllexport) void DllGotRemoteCandidate(int label, LPCSTR id, LPCSTR candidate, LPCSTR roomid, LPCSTR clientid, LPCSTR remoteid)
{
	if (g_bizAccess/*[std::string(clientid)]*/) {
		g_bizAccess/*[std::string(clientid)]*/->GotRemoteCandidate(label, id, candidate, roomid, clientid, remoteid);
	}
}

extern "C" __declspec(dllexport) void DllGotEoic(LPCSTR roomid, LPCSTR clientid, LPCSTR remoteid)
{
	if (g_bizAccess/*[std::string(clientid)]*/) {
		g_bizAccess/*[std::string(clientid)]*/->GotRemoteEoic(roomid, clientid, remoteid);
	}
}

extern "C" __declspec(dllexport) void DllExitApplication()
{
	/*
	map<std::string, CBizAccess*>::const_iterator it;
	for(it = g_bizAccess.begin(); it != g_bizAccess.end(); ++it) {
		delete it->second;
	}
	g_bizAccess.clear();
	*/
	if (g_bizAccess/*[std::string(clientid)]*/)
	{
		delete g_bizAccess;
		//g_bizAccess = NULL;
	}
}
