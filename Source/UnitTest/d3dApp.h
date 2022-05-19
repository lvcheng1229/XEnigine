//***************************************************************************************
// d3dApp.h by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "Runtime/RHI/RHICommandList.h"
#include "Runtime/D3D12RHI/D3D12PlatformRHI.h"
#include "Runtime/D3D12RHI/D3D12Adapter.h"
#include "Runtime/D3D12RHI/D3D12PhysicDevice.h"
#include "Runtime/D3D12RHI/D3D12AbstractDevice.h"
#include "Runtime/D3D12RHI/D3D12CommandQueue.h"
#include "Runtime/D3D12RHI/D3D12Allocation.h"
#include "Runtime/D3D12RHI/D3D12View.h"
#include "Runtime/D3D12RHI/D3D12PassStateManager.h"
#include "Runtime/D3D12RHI/D3D12Viewport.h"

//#include "d3dUtil.h"
#include "GameTimer.h"

//ImGUI Begin
#include <backends/imgui_impl_win32.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//ImGUI End

#include "Runtime/ApplicationCore/Windows/WindowsApplication.h"

class D3DApp
{
protected:

    D3DApp();
    D3DApp(const D3DApp& rhs) = delete;
    D3DApp& operator=(const D3DApp& rhs) = delete;
    virtual ~D3DApp();
    virtual void TempDelete() = 0;
public:

    static D3DApp* GetApp();
	float     AspectRatio()const;

	int Run();
    virtual bool Initialize();

protected:
	virtual void Update(const GameTimer& gt)=0;
    virtual void Renderer(const GameTimer& gt)=0;

protected:
	bool InitMainWindow();
	void CalculateFrameStats();
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

public:
    XApplication* Application;
    XRHICommandList RHICmdList;
    static D3DApp* mApp;
    HWND      mhMainWnd = nullptr; // main window handle
	GameTimer mTimer;
};

