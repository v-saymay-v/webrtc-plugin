#include "stdafx.h"
#include "resource.h"
#include "rtc_i.h"
#include "dllmain.h"

HINSTANCE g_hInst;
CrtcModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	g_hInst = hInstance;
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
