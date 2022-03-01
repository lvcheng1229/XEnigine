#include "D3D12DescriptorArray.h"

void XD3D12DescriptorArray::Create(XD3D12PhysicDevice* device_in, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
	SetParentDevice(device_in);
	ThrowIfFailed(device_in->GetDXDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&DescArray)));
	elemt_size = device_in->GetDXDevice()->GetDescriptorHandleIncrementSize(desc.Type);
	array_length = desc.NumDescriptors;
	if (desc.Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) { gpu_acessable = true; }
	else { gpu_acessable = false; }

	cpu_ptr_begin = DescArray->GetCPUDescriptorHandleForHeapStart();
	gpu_ptr_begin = DescArray->GetGPUDescriptorHandleForHeapStart();
}
