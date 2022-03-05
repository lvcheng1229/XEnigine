#pragma once
#include "D3D12PhysicDevice.h"
#include "Runtime/RHI/RHIResource.h"
class ResourceState
{
private:
	D3D12_RESOURCE_STATES m_ResourceState;
public:
	inline D3D12_RESOURCE_STATES GetResourceState() { return m_ResourceState; }
	inline void SetResourceState(const D3D12_RESOURCE_STATES state) { m_ResourceState = state; }
};

class XD3D12Resource
{
public:
	void Create(ID3D12Resource* resource_in, D3D12_RESOURCE_STATES state);

	inline void SetResourceState(D3D12_RESOURCE_STATES state) { m_resourceState.SetResourceState(state); }
	inline ID3D12Resource* GetResource() { return d3d12_resource.Get(); }
	inline ID3D12Resource** GetPtrToResourceAdress() { return &d3d12_resource; }
	inline ResourceState& GetResourceState() { return m_resourceState; }
	inline void* GetMappedResourceCPUPtr() { 
		d3d12_resource->Map(0, nullptr, &mapped_resource_cpu_ptr);
		return mapped_resource_cpu_ptr;
	}

	inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtaulAddress() { return GPUVirtualPtr; }
private:
	D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualPtr;
	void* mapped_resource_cpu_ptr;
	XDxRefCount<ID3D12Resource> d3d12_resource;
	ResourceState	m_resourceState;
};

class XD3DBuddyAllocator;
struct BuddyAllocatorData
{
	uint32 offset;
	uint32 order;
};

class XD3D12ResourceLocation
{
private:
	BuddyAllocatorData buddy_alloc_data;
	XD3DBuddyAllocator* buddy_alloc;
	
	/// for mannual suballocation
	void* mapped_resource_cpu_ptr;
	D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualPtr;
	XD3D12Resource* UploadCommitBackResource;
	//End
	
public:

	/// for mannual suballocation
	inline void SetBackResource(XD3D12Resource* ResourceIn) { UploadCommitBackResource = ResourceIn; }
	inline void SetMappedCPUResourcePtr(void* address_in) { mapped_resource_cpu_ptr = address_in; }
	inline void SetGPUVirtualPtr(D3D12_GPU_VIRTUAL_ADDRESS address) { GPUVirtualPtr = address; }
	inline void* GetMappedCPUResourcePtr() { return mapped_resource_cpu_ptr; }
	inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualPtr() { return GPUVirtualPtr; };
	//End

	inline void SetBuddyAllocator(XD3DBuddyAllocator* alloc_in) { buddy_alloc = alloc_in; };
	inline XD3DBuddyAllocator* GetBuddyAllocator() { return buddy_alloc; };
	inline BuddyAllocatorData& GetBuddyAllocData() { return buddy_alloc_data; };
};

class XD3D12ConstantBuffer :public XRHIConstantBuffer
{
public:
	XD3D12ResourceLocation ResourceLocation;
	inline void UpdateData(void* data, uint32 size,uint32 offset_byte)override
	{
		void* data_ptr = ResourceLocation.GetMappedCPUResourcePtr();
		data_ptr = static_cast<uint8*>(data_ptr) + offset_byte;
		memcpy(data_ptr, data, size);
	}
};

