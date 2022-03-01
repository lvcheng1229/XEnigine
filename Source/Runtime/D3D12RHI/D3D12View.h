#pragma once
#include "D3D12Resource.h"
class XD3D12View
{
protected:
	//ID3D12Resource* pResource;
	XD3D12Resource* pResource;
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_ptr;
public:
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUPtr() { return cpu_ptr; }
	inline XD3D12Resource* GetResource() { return pResource; };
};


class XD3D12RenderTargetView:public XD3D12View
{
private:
	D3D12_RENDER_TARGET_VIEW_DESC desc;
public:
	inline void Create(
		XD3D12PhysicDevice* device, 
		XD3D12Resource* resource, 
		const D3D12_RENDER_TARGET_VIEW_DESC desc_in,
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_ptr_in)
	{
		pResource = resource;
		desc = desc_in;
		cpu_ptr = cpu_ptr_in;
		device->GetDXDevice()->CreateRenderTargetView(resource->GetResource(), &desc, cpu_ptr);
	}
};


class XD3D12DepthStenciltView :public XD3D12View
{
private:
	D3D12_DEPTH_STENCIL_VIEW_DESC desc;
public:
	inline void Create(
		XD3D12PhysicDevice* device,
		XD3D12Resource* resource,
		const D3D12_DEPTH_STENCIL_VIEW_DESC desc_in,
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_ptr_in)
	{
		pResource = resource;
		desc = desc_in;
		cpu_ptr = cpu_ptr_in;
		device->GetDXDevice()->CreateDepthStencilView(resource->GetResource(), &desc, cpu_ptr);
	}
};