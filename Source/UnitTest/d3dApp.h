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

#include "d3dUtil.h"
#include "GameTimer.h"


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
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	virtual void OnResize(); 
	virtual void Update(const GameTimer& gt)=0;
    virtual void Renderer(const GameTimer& gt)=0;

	// Convenience overrides for handling mouse input.
	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }

protected:
	bool InitMainWindow();
	bool InitDirect3D();
	void CalculateFrameStats();
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
public:
    XD3D12Adapter Adapter;
    XD3D12PhysicDevice Device;
    XD3D12AbstractDevice abstrtact_device;
    XD3D12Viewport viewport;

    XD3D12CommandQueue* direct_cmd_queue;
    XD3DDirectContex* direct_ctx;
    XRHICommandList RHICmdList;


    XD3D12DescArrayManager* RenderTargetDescArrayManager;
    XD3D12DescArrayManager* DepthStencilDescArrayManager;

    XD3D12PassStateManager* pass_state_manager;

    std::shared_ptr<XRHITexture2D>TextureDepthStencil;
    //XRHIDepthStencilView* DsView;
    //XD3D12DepthStencilView ds_view;
    XD3D12Resource ds_resource;
protected:
    static D3DApp* mApp;
    
    HWND      mhMainWnd = nullptr; // main window handle
	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
    bool      mFullscreenState = false;// fullscreen enabled

	// Used to keep track of the “delta-time?and game time (?.4).
	GameTimer mTimer;
	
    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

	static const int SwapChainBufferCount = 2;
    Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

	// Derived class should set these in derived constructor to customize starting values.
	std::wstring mMainWndCaption = L"d3d App";
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//int mClientWidth = 800;
	int mClientWidth = 1920;
	//int mClientHeight = 600;
	int mClientHeight = 1080;
};

