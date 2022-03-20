#include "D3D12PipelineCurrentDescArray.h"

//#define MAX_CBV_SRV_UAV_DESC_COUNT 32

void XD3D12PipelineCurrentDescArray::Create(XD3D12PhysicDevice* device_in)
{
	SetParentDevice(device_in);

	TotalNum = 10000;

	D3D12_DESCRIPTOR_HEAP_DESC desc_heap_desc = {};
	desc_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc_heap_desc.NumDescriptors = TotalNum;//TODO
	desc_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ThrowIfFailed(device_in->GetDXDevice()->CreateDescriptorHeap(&desc_heap_desc, IID_PPV_ARGS(&DescArray)));
	DescArray->SetName(L"XD3D12DescArrayShaderAcess");

	cpu_ptr_begin = DescArray->GetCPUDescriptorHandleForHeapStart();
	gpu_ptr_begin = DescArray->GetGPUDescriptorHandleForHeapStart();

	elemt_size = device_in->GetDXDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CurrentSlotIndex = 0;
}




