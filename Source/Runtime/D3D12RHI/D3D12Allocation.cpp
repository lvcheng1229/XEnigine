#include "D3D12Allocation.h"
#include "d3dx12.h"

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
		ThrowIfFailed(GetParentDevice()->GetDXDevice()->CreateCommittedResource(
			&(CD3DX12_HEAP_PROPERTIES(config.d3d12_heap_type)),
			D3D12_HEAP_FLAG_NONE,
			&(CD3DX12_RESOURCE_DESC::Buffer(max_block_size)),
			config.d3d12_resource_states,
			nullptr,
			IID_PPV_ARGS(&back_resource)));
		back_resource->SetName(L"back_resource");
	}

}

bool XD3DBuddyAllocator::Allocate(uint32 allocate_size_byte, /*uint32 alignment,*/ XD3D12ResourceLocation& resource_location)
{
	bool can_allocate = false;
	uint32 order = SizeToOrder(allocate_size_byte);
	for (; order <= max_order; ++order)
	{
		if (offset_from_left[order].size() != 0)
		{
			can_allocate = true;
			break;
		}
	}

	if (can_allocate)
	{
		uint32 offset_res = Allocate_Impl(order);
		resource_location.SetBuddyAllocator(this);
		BuddyAllocatorData& alloc_data = resource_location.GetBuddyAllocData();
		alloc_data.offset = offset_res;
		alloc_data.order = order;
	}

	return can_allocate;
}


uint32 XD3DBuddyAllocator::Allocate_Impl(uint32 order)
{
	uint32 offset_left;
	if (offset_from_left[order].size() == 0)
	{
		offset_left = Allocate_Impl(order + 1);
		uint32 offset_right = 1 << order;
		offset_from_left[order].insert(offset_right);
	}
	else
	{
		offset_left = *(offset_from_left[order].begin());
		offset_from_left[order].erase(offset_left);
	}
	return offset_left;
}

