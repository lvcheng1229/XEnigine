#pragma once
#include "D3D12PhysicDevice.h"
#include "D3D12Resource.h"

#include <vector>
#include <set>
//64KB
#define MIN_PLACED_BUFFER_SIZE (64*1024)

enum class AllocStrategy
{
	PlacedResource,
	ManualSubAllocation,
};
struct XAllocConfig
{
	//PlacedResource
	D3D12_HEAP_TYPE d3d12_heap_type;
	D3D12_HEAP_FLAGS d3d12_heap_flags;

	//ManualSubAllocation
	D3D12_RESOURCE_STATES d3d12_resource_states;
	D3D12_RESOURCE_FLAGS d3d12_resource_flags;
};

class XD3DBuddyAllocator: public XD3D12DeviceChild
{
	
private:
	std::vector<std::set<uint32>>offset_from_left;

	uint32 max_block_size;
	uint32 min_block_size;
	uint32 max_order;

	XAllocConfig config;

	XDxRefCount<ID3D12Heap>m_heap;
	//XDxRefCount<ID3D12Resource>back_resource;
	XD3D12Resource back_resource;

	AllocStrategy strategy;
public:
	XD3DBuddyAllocator():TotalUsed(0) {};
	~XD3DBuddyAllocator() {};
	//~XD3DBuddyAllocator() = default;
	void Create(XD3D12PhysicDevice* device_in, 
		XAllocConfig config_in,
		uint32 max_block_size_in,
		uint32 min_block_size_in, 
		AllocStrategy strategy_in);

	bool Allocate(uint32 allocate_size_byte, /*uint32 alignment,*/ XD3D12ResourceLocation& resource_location);
	void Deallocate(XD3D12ResourceLocation& ResourceLocation);
	inline ID3D12Heap* GetDXHeap() { return m_heap.Get(); };
	inline ID3D12Resource* GetDXResource() { return back_resource.GetResource(); };
	//void deallocate();
private:
	uint32 Allocate_Impl(uint32 order);
	inline uint32 SizeToOrder(uint32 size)
	{
		uint32 min_block_times = (size + (min_block_size - 1)) / min_block_size;
		return ceil(log(min_block_times) / log(2.0));
	}
	uint32 TotalUsed;

}; 

//class XD3DMultiBuddyAllocator
//{
//private:
//	std::vector<XD3DBuddyAllocator>alloc;
//public:
//};
//
//class TextureAllocator
//{
//private:
//	XD3DMultiBuddyAllocator multi_buddy_allocator;
//public:
//};