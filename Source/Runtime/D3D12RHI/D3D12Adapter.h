#pragma once
#include "Runtime/HAL/Mch.h"
#include "Runtime/HAL/PlatformTypes.h"
class XD3D12Adapter
{
public:
	void Create();
	
	inline IDXGIAdapter* GetDXAdapter() { return dxgi_adapter.Get(); }
	inline IDXGIFactory4* GetDXFactory() { return dxgi_factory.Get(); }
private:
	XDxRefCount<IDXGIFactory4> dxgi_factory;
	XDxRefCount<IDXGIAdapter> dxgi_adapter;
};