#pragma once
#include "D3D12PhysicDevice.h"
#include <vector>

class XD3D12PipelineCurrentDescArray :public XD3D12DeviceChild
{
public:
	void Create(XD3D12PhysicDevice* device_in);

	//void AppendCbvSrvUavDescriptors(const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& SrcDescriptors);
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescPtrByIndex(uint32 index) {
		D3D12_CPU_DESCRIPTOR_HANDLE ret_ptr = { cpu_ptr_begin.ptr + +static_cast<UINT64>(index) * static_cast<UINT64>(elemt_size) };
		return ret_ptr;
	}

	inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescPtrByIndex(uint32 index) {
		D3D12_GPU_DESCRIPTOR_HANDLE ret_ptr = { gpu_ptr_begin.ptr + +static_cast<UINT64>(index) * static_cast<UINT64>(elemt_size) };
		return ret_ptr;
	}

	inline uint32 GetElemSize() { return elemt_size; }
	inline ID3D12DescriptorHeap* GetDescHeapPtr() { return DescArray.Get(); }
private:
	uint32 elemt_size;
	//uint32 DescArrayCurrentIndex;

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_ptr_begin;
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_ptr_begin;
	XDxRefCount<ID3D12DescriptorHeap>DescArray;
};

