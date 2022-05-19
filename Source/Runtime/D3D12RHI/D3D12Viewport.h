#pragma once
#include "D3D12View.h"
#include "D3D12AbstractDevice.h"
#define BACK_BUFFER_COUNT_DX12 2

class XD3D12Viewport :public XRHIViewport
{
public:
	void Create(XD3D12AbstractDevice* device_in, uint32 size_x_in, uint32 size_y_in, 
		EPixelFormat EPixFormatIn, 
		HWND WindowHandle_in);
	void Resize(uint32 size_x_in, uint32 size_y_in);
	void Present();
	inline XD3D12Texture2D* GetCurrentBackTexture() { return BackBufferTextures[current_back_buffer].get(); }
private:
	XD3D12AbstractDevice* device;

	uint32 current_back_buffer;

	uint32 size_x;
	uint32 size_y;
	EPixelFormat Format;

	std::vector<std::shared_ptr<XD3D12Texture2D>>BackBufferTextures;
	XD3D12Resource back_buffer_resources[BACK_BUFFER_COUNT_DX12];
	XD3D12RenderTargetView back_rt_views[BACK_BUFFER_COUNT_DX12];
	XDxRefCount<IDXGISwapChain> mSwapChain;
};