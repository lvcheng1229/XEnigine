#pragma once
#include "D3D12PhysicDevice.h"

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
	//inline void* GetResourceCPUPtr(const D3D12_RANGE* ReadRange = nullptr) 
	//{
	//	d3d12_resource->Map(0, ReadRange, &cpu_ptr);
	//	return cpu_ptr;
	//}
	inline void SetResourceState(D3D12_RESOURCE_STATES state) { m_resourceState.SetResourceState(state); }
	inline ID3D12Resource* GetResource() { return d3d12_resource.Get(); }
	inline ID3D12Resource** GetPtrToResourceAdress() { return &d3d12_resource; }
	inline ResourceState& GetResourceState() { return m_resourceState; }
private:
	//void* cpu_ptr;
	XDxRefCount<ID3D12Resource> d3d12_resource;
	//D3D12_GPU_VIRTUAL_ADDRESS gpu_ptr;
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
public:
	inline void SetBuddyAllocator(XD3DBuddyAllocator* alloc_in) { buddy_alloc = alloc_in; };
	inline XD3DBuddyAllocator* GetBuddyAllocator() { return buddy_alloc; };
	inline BuddyAllocatorData& GetBuddyAllocData() { return buddy_alloc_data; };
};

class XD3D12Heap
{
private:
	XDxRefCount<ID3D12Heap>d3d12_heap;
};