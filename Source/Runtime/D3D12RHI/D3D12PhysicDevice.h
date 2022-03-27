#pragma once
#include "D3D12Adapter.h"
#include "D3D12PipelineLibrary.h"
class XD3D12PhysicDevice
{
private:
	
	XD3D12Adapter* adapter;
	XDxRefCount<ID3D12Device> d3d12_device;
	XDxRefCount<ID3D12Device1> ID3D12Device1Ptr;
	XD3D12PipelineLibrary D3D12PipelineLibrary;

public:
	void Create(XD3D12Adapter* adapter_in);

	inline XD3D12Adapter*			GetAdapter()				{ return adapter; }
	inline ID3D12Device*			GetDXDevice()				{ return d3d12_device.Get(); }
	inline ID3D12Device1*			GetDXDevice1()				{ return ID3D12Device1Ptr.Get(); }
	inline XD3D12PipelineLibrary*	GetD3D12PipelineLibrary()	{ return &D3D12PipelineLibrary; }
};

class XD3D12DeviceChild
{
public:
	explicit XD3D12DeviceChild() :p_device(nullptr) {};

	inline void					SetParentDevice(XD3D12PhysicDevice* p_device_in) { p_device = p_device_in; }
	inline XD3D12PhysicDevice*	GetParentDevice() { X_Assert((p_device != nullptr)); return p_device; };
private:
	XD3D12PhysicDevice* p_device;
};