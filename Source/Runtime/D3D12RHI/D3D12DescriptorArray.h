#pragma once
#include "D3D12PhysicDevice.h"

#include <vector>
#include <unordered_set>

class XD3D12DescArrayManager
{
private:
	struct DescArray
	{
		XDxRefCount<ID3D12DescriptorHeap> d3d12_heap;
		std::unordered_set<uint32>	desc_index_free;
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_ptr_begin;
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_ptr_begin;
	};

	uint32							desc_per_heap;
	uint32							element_size;

	XD3D12PhysicDevice*				device;
	D3D12_DESCRIPTOR_HEAP_DESC		desc;

	std::unordered_set<uint32>		free_desc_array_index;;
	std::vector<DescArray>			all_desc_arrays;
public:

	void FreeDesc(uint32 index_of_desc_in_heap, uint32 index_of_heap);
	void AllocateDesc(uint32& index_of_desc_in_heap, uint32& index_of_heap);
	void Create(XD3D12PhysicDevice* device_in, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 num);

	inline ID3D12DescriptorHeap* GetDxHeapByIndex(const uint32 IndexOfHeap)
	{
		return all_desc_arrays[IndexOfHeap].d3d12_heap.Get();
	}

	inline D3D12_CPU_DESCRIPTOR_HANDLE compute_cpu_ptr(const uint32 index_of_desc_in_heap,const  uint32 index_of_heap)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE ret_ptr = { 
			all_desc_arrays[index_of_heap].cpu_ptr_begin.ptr + 
			static_cast<UINT64>(index_of_desc_in_heap) * static_cast<UINT64>(element_size) };
		return ret_ptr;
	}

	inline D3D12_GPU_DESCRIPTOR_HANDLE compute_gpu_ptr(const uint32 index_of_desc_in_heap, const  uint32 index_of_heap)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE ret_ptr = {
			all_desc_arrays[index_of_heap].gpu_ptr_begin.ptr +
			static_cast<UINT64>(index_of_desc_in_heap) * static_cast<UINT64>(element_size) };
		return ret_ptr;
	}

private:
	void AllocHeap();
};