#include "D3D12DescArrayShaderAcesss.h"

#define MAX_CBV_SRV_UAV_DESC_COUNT 32

void XD3D12PipelineCurrentDescArray::Create(XD3D12PhysicDevice* device_in)
{
	SetParentDevice(device_in);

	D3D12_DESCRIPTOR_HEAP_DESC desc_heap_desc = {};
	desc_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc_heap_desc.NumDescriptors = MAX_CBV_SRV_UAV_DESC_COUNT;
	desc_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ThrowIfFailed(device_in->GetDXDevice()->CreateDescriptorHeap(&desc_heap_desc, IID_PPV_ARGS(&DescArray)));
	DescArray->SetName(L"XD3D12DescArrayShaderAcess");

	cpu_ptr_begin = DescArray->GetCPUDescriptorHandleForHeapStart();
	gpu_ptr_begin = DescArray->GetGPUDescriptorHandleForHeapStart();

	//DescArrayCurrentIndex = 0;
	elemt_size = device_in->GetDXDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

//void XD3D12PipelineCurrentDescArray::AppendCbvSrvUavDescriptors(const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& SrcDescriptors)
//{
//	uint32_t SlotsNeeded = (uint32_t)SrcDescriptors.size();
//	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = { cpu_ptr_begin.ptr + elemt_size * DescArrayCurrentIndex };
//	X_Assert(SlotsNeeded + DescArrayCurrentIndex < MAX_CBV_SRV_UAV_DESC_COUNT);
//	GetParentDevice()->GetDXDevice()->CopyDescriptors(1, &cpu_handle, &SlotsNeeded, SlotsNeeded, SrcDescriptors.data(), nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//	DescArrayCurrentIndex += SlotsNeeded;
//}




