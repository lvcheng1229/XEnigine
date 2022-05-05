#include "D3D12Allocation.h"
#include "d3dx12.h"
#include "Runtime/Core/Template/XEngineTemplate.h"
#include <iostream>


#ifdef LOG_USED_BLOCK
static int heap_index_static = 0;
#endif

void XD3DBuddyAllocator::Create(
	XD3D12PhysicDevice* device_in, 
	
	XAllocConfig config_in,

	uint32 max_block_size_in,
	uint32 min_block_size_in,
	AllocStrategy strategy_in)
{
	SetParentDevice(device_in);
	config = config_in;

	max_block_size = max_block_size_in;
	min_block_size = min_block_size_in;
	max_order = SizeToOrder(max_block_size);

#ifdef LOG_USED_BLOCK
	heap_index = heap_index_static;
	heap_index_static++;
	blocks_free_index.resize(max_block_size / min_block_size);
	for (auto t : blocks_free_index) { t = 0; }
#endif
	offset_from_left.clear();
	offset_from_left.resize(max_order+1);
	offset_from_left[max_order].insert(static_cast<uint32>(0));

	strategy = strategy_in;

	if (strategy == AllocStrategy::PlacedResource)
	{
		D3D12_HEAP_PROPERTIES heap_properties;
		heap_properties.Type = config.d3d12_heap_type;;
		heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_properties.CreationNodeMask = 0;
		heap_properties.VisibleNodeMask = 0;

		D3D12_HEAP_DESC heap_desc;
		heap_desc.SizeInBytes = max_block_size;
		heap_desc.Properties = heap_properties;
		heap_desc.Alignment = MIN_PLACED_BUFFER_SIZE;
		heap_desc.Flags = config.d3d12_heap_flags;

		ThrowIfFailed(GetParentDevice()->GetDXDevice()->CreateHeap(&heap_desc, IID_PPV_ARGS(&m_heap)));
		m_heap->SetName(L"buddy allocator heap");
	}
	else
	{
		CD3DX12_RESOURCE_DESC ResDesc = CD3DX12_RESOURCE_DESC::Buffer(max_block_size);
		ResDesc.Flags |= config.d3d12_resource_flags;

		ThrowIfFailed(GetParentDevice()->GetDXDevice()->CreateCommittedResource(
			GetRValuePtr((CD3DX12_HEAP_PROPERTIES(config.d3d12_heap_type))),
			D3D12_HEAP_FLAG_NONE,
			&ResDesc,
			config.d3d12_resource_states,
			nullptr,
			IID_PPV_ARGS(back_resource.GetPtrToResourceAdress())));
		back_resource.Create(back_resource.GetResource(), config.d3d12_resource_states);
		back_resource.GetResource()->SetName(L"Alloc back_resource");
	}

}

bool XD3DBuddyAllocator::Allocate(uint32 allocate_size_byte_in, uint32 alignment, XD3D12ResourcePtr_CPUGPU& resource_location)
{
	bool can_allocate = false;
	uint32 allocate_size_byte = allocate_size_byte_in;
	{
		if (alignment != 0 && min_block_size % alignment != 0)
		{
			allocate_size_byte = allocate_size_byte_in + alignment;
		}
		uint32 order = SizeToOrder(allocate_size_byte);
		for (; order <= max_order; ++order)
		{
			if (offset_from_left[order].size() != 0)
			{
				can_allocate = true;
				break;
			}
		}

		if (allocate_size_byte > (max_block_size - TotalUsed))
		{
			can_allocate = false;
		}
	}


	if (can_allocate)
	{
		uint32 order = SizeToOrder(allocate_size_byte);
		TotalUsed += min_block_size * (1 << order);
		
		X_Assert(TotalUsed < max_block_size);

		uint32 OffsetRes = Allocate_Impl(order);
#ifdef LOG_USED_BLOCK
		std::cout << "** Heap Alloc Order " << order << "**" << std::endl;
		std::cout << "heap index: " << heap_index << " used begin :" << OffsetRes << " size:" << (1 << order) << std::endl;
		for (int i = OffsetRes; i < OffsetRes + (1 << order); i++)
		{
			if (blocks_free_index[i] == 1) { X_Assert(false); }
			blocks_free_index[i] = 1;
		}
#endif // LOG_FREE_BLOCK

		resource_location.SetBuddyAllocator(this);
		BuddyAllocatorData& alloc_data = resource_location.GetBuddyAllocData();
		alloc_data.Offset_MinBlockUnit = OffsetRes;
		alloc_data.order = order;

		uint32 AllocatedResourceOffset = uint32(OffsetRes * min_block_size);
		if (alignment != 0 && AllocatedResourceOffset % alignment != 0)
		{
			AllocatedResourceOffset = AlignArbitrary(AllocatedResourceOffset, alignment);
		}
		
		if (strategy == AllocStrategy::ManualSubAllocation)
		{
			resource_location.SetOffsetByteFromBaseResource(AllocatedResourceOffset);
			resource_location.SetBackResource(&back_resource);
			resource_location.SetGPUVirtualPtr(back_resource.GetGPUVirtaulAddress() + AllocatedResourceOffset);

			if (config.d3d12_heap_type == D3D12_HEAP_TYPE_UPLOAD) //if cpu writable
			{
				resource_location.SetMappedCPUResourcePtr((uint8*)back_resource.GetMappedResourceCPUPtr() + AllocatedResourceOffset);
			}
		}
	}

#ifdef LOG_USED_BLOCK
	std::cout << "heap index:" << heap_index << "--------------------------------------------" << std::endl;
	bool bOne = false;
	for (int i = 0; i < blocks_free_index.size(); i++)
	{
		if (blocks_free_index[i] == 1&& bOne==false)
		{
			std::cout << "start: " << i; bOne = true;
		}
		if (blocks_free_index[i] == 0 && bOne == true)
		{
			std::cout << " end: " << i << std::endl; bOne = false;
		}
	
	}
#endif // LOG_FREE_BLOCK

	X_Assert(can_allocate != false);

	return can_allocate;
}

void XD3DBuddyAllocator::Deallocate(XD3D12ResourcePtr_CPUGPU& ResourceLocation)
{

}




uint32 XD3DBuddyAllocator::Allocate_Impl(uint32 order)
{
	uint32 offset_left;
	
	X_Assert(order <= max_order);
	
	if (offset_from_left[order].size() == 0)
	{
		offset_left = Allocate_Impl(order + 1);
		uint32 offset_right = offset_left + uint32(((uint32)1) << order);
		offset_from_left[order].insert(offset_right);
	}
	else
	{
		X_Assert(offset_from_left[order].size() > 0)
		offset_left = *(offset_from_left[order].begin());
		offset_from_left[order].erase(offset_left);
	}
	return offset_left;
}

