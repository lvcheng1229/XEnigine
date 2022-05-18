#pragma once
#ifdef X_PLATFORM_WIN
#include <windows.h>
#include <windowsx.h>
#include "Runtime/ApplicationCore/Application.h"

class XWindowsApplication :public XApplication
{
private:
	static HWND WinHandle;

	const wchar_t* WindowsClassName = L"MainWnd";
	const wchar_t* WindowName = L"XEngine";
public:
	XWindowsApplication();
	void* GetPlatformHandle()override;
	static LRESULT WINAPI WindowsMsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
protected:
	bool CreateAppWindow()override;
	bool InitRHI()override;
};


#endif
