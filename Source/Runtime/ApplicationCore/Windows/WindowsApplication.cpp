#ifdef X_PLATFORM_WIN
#include "WindowsApplication.h"
#include "Runtime/RHI/RHI.h"
#include <backends/imgui_impl_win32.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

XApplication* XApplication::APPPtr = nullptr;
HWND XWindowsApplication::WinHandle = nullptr;

XWindowsApplication::XWindowsApplication()
{
	XApplication::APPPtr = this;
}

bool XWindowsApplication::CreateAppWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowsMsgProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = nullptr;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = WindowsClassName;

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	RECT R = { 0, 0, ClientWidth, ClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int Width  = R.right - R.left;
	int Height = R.bottom - R.top;
	WinHandle = CreateWindow(WindowsClassName, WindowName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Width, Height, 0, 0, nullptr, 0);
	
	if (!WinHandle)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(WinHandle, SW_SHOW);
	UpdateWindow(WinHandle);

	return true;
}
bool XWindowsApplication::InitRHI()
{
	RHIInit(ClientWidth, ClientHeight);
	return true;
}
void* XWindowsApplication::GetPlatformHandle()
{
	return WinHandle;
}
LRESULT WINAPI XWindowsApplication::WindowsMsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_ACTIVATE:
		return 0; 
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);
		
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
		XAppInput::InputPtr->InputMsgProcsss(EInputType::MOUSE, EInputEvent::DOWN, EInputKey::MOUSE_LEFT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
		SetCapture(WinHandle);
		return 0;

	case WM_LBUTTONUP:
		XAppInput::InputPtr->InputMsgProcsss(EInputType::MOUSE, EInputEvent::UP, EInputKey::MOUSE_LEFT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
		ReleaseCapture();
		return 0;

	case WM_RBUTTONDOWN:
		XAppInput::InputPtr->InputMsgProcsss(EInputType::MOUSE, EInputEvent::DOWN, EInputKey::MOUSE_RIGHT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
		SetCapture(WinHandle);
		return 0;

	case WM_RBUTTONUP:
		XAppInput::InputPtr->InputMsgProcsss(EInputType::MOUSE, EInputEvent::UP, EInputKey::MOUSE_RIGHT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
		ReleaseCapture();
		return 0;

	case WM_MOUSEMOVE:
		XAppInput::InputPtr->InputMsgProcsss(EInputType::MOUSE, EInputEvent::MOUSE_MOVE, EInputKey::MOUSE_MAX, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
		return 0;

	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}

		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
#endif
