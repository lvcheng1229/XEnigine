//***************************************************************************************
// d3dApp.h by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "Runtime/D3D12RHI/DX12GIAdapter.h"
#include "Runtime/D3D12RHI/D3D12PhysicDevice.h"
#include "Runtime/D3D12RHI/D3D12AbstractDevice.h"
#include "Runtime/D3D12RHI/D3D12CommandQueue.h"
#include "Runtime/D3D12RHI/D3D12Allocation.h"
#include "Runtime/D3D12RHI/D3D12View.h"
#include "Runtime/D3D12RHI/D3D12PassStateManager.h"

#include "d3dUtil.h"
#include "GameTimer.h"

class D3DApp
{
protected:

    D3DApp();
    D3DApp(const D3DApp& rhs) = delete;
    D3DApp& operator=(const D3DApp& rhs) = delete;
    virtual ~D3DApp();

public:

    static D3DApp* GetApp();
    
	
	HWND      MainWnd()const;
	float     AspectRatio()const;

    bool Get4xMsaaState()const;
    void Set4xMsaaState(bool value);

	int Run();
 
    virtual bool Initialize();
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	virtual void OnResize(); 
	virtual void Update(const GameTimer& gt)=0;
    virtual void Draw(const GameTimer& gt)=0;

	// Convenience overrides for handling mouse input.
	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }

protected:

	bool InitMainWindow();
	bool InitDirect3D();
    void CreateSwapChain();

	ID3D12Resource* CurrentBackBuffer()const;

	void CalculateFrameStats();
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

public:
    XDXGIAdapter Adapter;
    XD3D12PhysicDevice Device;
    XD3D12AbstractDevice abstrtact_device;
    XD3D12CommandQueue* direct_cmd_queue;
    XD3DDirectContex* direct_ctx;
    XD3D12Fence *d3d12_fence;

    XD3DBuddyAllocator texture_default_heap_alloc;
    XD3DBuddyAllocator texture_upload_heap_alloc;

    XD3D12DescriptorArray* rt_desc_array;
    XD3D12DescriptorArray* ds_desc_array;

    XD3D12PassStateManager pass_state_manager;

    XD3D12DepthStenciltView ds_view;
    XD3D12Resource ds_resource;

    std::vector<XD3D12RenderTargetView> rt_views;
    std::vector<XD3D12RenderTargetView*> rt_view_ptrs;
    std::vector<XD3D12Resource>swap_rt_buffers;

protected:
    static D3DApp* mApp;

    
    HWND      mhMainWnd = nullptr; // main window handle
	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
    bool      mFullscreenState = false;// fullscreen enabled

	// Set true to use 4X MSAA (?.1.8).  The default is false.
    bool      m4xMsaaState = false;    // 4X MSAA enabled
    UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

	// Used to keep track of the “delta-time?and game time (?.4).
	GameTimer mTimer;
	
    Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

    //Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
    //UINT64 mCurrentFence = 0;
	
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

	static const int SwapChainBufferCount = 2;
	int mCurrBackBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
    Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;


	UINT mCbvSrvUavDescriptorSize = 0;

	// Derived class should set these in derived constructor to customize starting values.
	std::wstring mMainWndCaption = L"d3d App";
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int mClientWidth = 800;
	int mClientHeight = 600;
};

