#pragma once
#include "D3D12Adapter.h"

class XD3D12PhysicDevice
{
private:
	XDxRefCount<ID3D12Device> d3d12_device;
	XD3D12Adapter* adapter;
public:
	void Create(XD3D12Adapter* adapter_in);
	inline XD3D12Adapter* GetAdapter() { return adapter; }
	inline ID3D12Device* GetDXDevice() { return d3d12_device.Get(); }
};

class XD3D12DeviceChild
{
public:
	explicit XD3D12DeviceChild() :p_device(nullptr) {};
	inline void SetParentDevice(XD3D12PhysicDevice* p_device_in) { p_device = p_device_in; }
	inline XD3D12PhysicDevice* GetParentDevice() { X_Assert((p_device != nullptr)); return p_device; };
private:
	XD3D12PhysicDevice* p_device;
};