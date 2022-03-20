#include "D3D12DescriptorArray.h"

void XD3D12DescArrayManager::Create(XD3D12PhysicDevice* device_in, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 num)
{
	device = device_in;
	desc_per_heap = num;
	//heap_type = type;

	desc.Type = type;
	desc.NumDescriptors = num;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;

	element_size = device->GetDXDevice()->GetDescriptorHandleIncrementSize(desc.Type);
}

void XD3D12DescArrayManager::AllocateDesc(uint32& index_of_desc_in_heap, uint32& index_of_heap)
{
	if (free_desc_array_index.size() == 0)
	{
		AllocHeap();
	}
	index_of_heap = *free_desc_array_index.begin();
	index_of_desc_in_heap= *all_desc_arrays[index_of_heap].desc_index_free.begin();
	
	all_desc_arrays[index_of_heap].desc_index_free.erase(index_of_desc_in_heap);
	
	if(all_desc_arrays[index_of_heap].desc_index_free.size()==0)
		free_desc_array_index.erase(index_of_heap);
}

void XD3D12DescArrayManager::AllocHeap()
{
	all_desc_arrays.push_back(DescArray());
	size_t index = all_desc_arrays.size() - 1;
	free_desc_array_index.insert(static_cast<uint32>(index));

	ThrowIfFailed(device->GetDXDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&all_desc_arrays[index].d3d12_heap)));
	all_desc_arrays[index].cpu_ptr_begin = all_desc_arrays[index].d3d12_heap->GetCPUDescriptorHandleForHeapStart();
	all_desc_arrays[index].gpu_ptr_begin = all_desc_arrays[index].d3d12_heap->GetGPUDescriptorHandleForHeapStart();
	for (uint32 i = 0; i < desc_per_heap; i++)
	{
		all_desc_arrays[index].desc_index_free.insert(i);
	}
}

void XD3D12DescArrayManager::FreeDesc(uint32 index_of_desc_in_heap, uint32 index_of_heap)
{
	if (free_desc_array_index.find(index_of_heap) == free_desc_array_index.end())
		free_desc_array_index.insert(index_of_heap);

	all_desc_arrays[index_of_heap].desc_index_free.insert(index_of_desc_in_heap);

}
