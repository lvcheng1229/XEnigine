#pragma once
#include "D3D12PhysicDevice.h"

//TODO:FD3D12OfflineDescriptorManager In UE
//no list now
class XD3D12DescriptorArray :public XD3D12DeviceChild
{
public:
	void Create(XD3D12PhysicDevice* device_in, const D3D12_DESCRIPTOR_HEAP_DESC& desc);
	
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescPtrByIndex(uint32 index) {
		D3D12_CPU_DESCRIPTOR_HANDLE ret_ptr = { cpu_ptr_begin.ptr + +static_cast<UINT64>(index) * static_cast<UINT64>(elemt_size) };
		return ret_ptr;
	}
	
	inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescPtrByIndex(uint32 index) {
		D3D12_GPU_DESCRIPTOR_HANDLE ret_ptr = { gpu_ptr_begin.ptr + +static_cast<UINT64>(index) * static_cast<UINT64>(elemt_size) };
		return ret_ptr;
	}

	inline uint32 GetElemSize() { return elemt_size; }
	inline bool IsGPUAcessable() { return gpu_acessable; };
	inline uint32 GetDescArrayLength() { return array_length; }
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescPtrBegin() { return cpu_ptr_begin; }
	inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescPtrBegin() { return gpu_ptr_begin; }
	inline ID3D12DescriptorHeap* GetDescHeapPtr() { return DescArray.Get(); }
private:
	uint32 array_length;
	uint32 elemt_size;
	bool gpu_acessable;

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_ptr_begin;
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_ptr_begin;
	XDxRefCount<ID3D12DescriptorHeap>DescArray;
};