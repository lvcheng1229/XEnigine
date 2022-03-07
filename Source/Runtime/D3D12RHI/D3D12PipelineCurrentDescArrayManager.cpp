#include "D3D12PipelineCurrentDescArrayManager.h"
#include "D3D12Context.h"
#include "D3D12PlatformRHI.h"

void XD3D12PipelineCurrentDescArrayManager::Create(XD3D12PhysicDevice* device_in, XD3DDirectContex* direct_ctx_in)
{
	device = device_in;
	direct_ctx = direct_ctx_in;
	pipeline_current_desc_array.Create(device_in);
}

template void XD3D12PipelineCurrentDescArrayManager::SetDescTableSRVs<EShaderType::SV_Pixel>(
	const XD3D12RootSignature* root_signature,
	XD3D12PassShaderResourceManager* SRVManager,
	uint32& slot_start, uint64 slot_mask);

#include "d3dx12.h"
template<EShaderType shader_type>
void XD3D12PipelineCurrentDescArrayManager::SetDescTableSRVs(
	const XD3D12RootSignature* root_signature,
	XD3D12PassShaderResourceManager* SRVManager,
	uint32& slot_start, uint64 slot_mask)
{
	uint32 first_slot_index = slot_start;
	unsigned long slotnum;

	//https://docs.microsoft.com/zh-cn/cpp/intrinsics/bitscanreverse-bitscanreverse64?view=msvc-170
	if (_BitScanReverse64(&slotnum, slot_mask) == 0) { slotnum = -1; };
	slotnum ++;
	slot_start += slotnum;

	const D3D12_CPU_DESCRIPTOR_HANDLE DestCPUPtr = pipeline_current_desc_array.GetCPUDescPtrByIndex(first_slot_index);
	D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptors[64];
	
	for (uint32 i = 0; i < slotnum; i++)
	{
		SrcDescriptors[i] = SRVManager->Views[shader_type][i]->GetCPUPtr();

		//XD3D12PlatformRHI::TransitionResource(
		//	(*direct_ctx->GetCmdList()),
		//	SRVManager->Views[shader_type][i],
		//	(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));	
	}

	uint32 NumCopy = static_cast<uint32>(slotnum);
	device->GetDXDevice()->CopyDescriptors(
		1, &DestCPUPtr,
		&NumCopy, NumCopy,
		SrcDescriptors,
		nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);



	D3D12_GPU_DESCRIPTOR_HANDLE GPUDescPtr = pipeline_current_desc_array.GetGPUDescPtrByIndex(first_slot_index);
	uint32 slot_index = root_signature->GetSRVDescTableBindSlot(shader_type);
	(*direct_ctx->GetCmdList())->SetGraphicsRootDescriptorTable(slot_index, GPUDescPtr);



}
template void XD3D12PipelineCurrentDescArrayManager::SetRootDescCBVs<EShaderType::SV_Vertex>(
	const XD3D12RootSignature* root_signature,
	XD3D12CBVRootDescManager* gpu_virtual_ptr_array,
	uint16 slot_mask);
template void XD3D12PipelineCurrentDescArrayManager::SetRootDescCBVs<EShaderType::SV_Pixel>(
	const XD3D12RootSignature* root_signature,
	XD3D12CBVRootDescManager* gpu_virtual_ptr_array,
	uint16 slot_mask);


template<EShaderType shader_type>
void XD3D12PipelineCurrentDescArrayManager::SetRootDescCBVs(
	const XD3D12RootSignature* root_signature,
	XD3D12CBVRootDescManager* gpu_virtual_ptr_array,
	uint16 slot_mask)
{
	unsigned long slotnum;
	if (_BitScanReverse(&slotnum, slot_mask) == 0) { slotnum = -1; };
	slotnum++;
	uint32 base_index = root_signature->GetCBVRootDescBindSlot(shader_type);
	for (uint32 slot_index = 0; slot_index < slotnum; ++slot_index)
	{
		if (XD3D12CBVRootDescManager::IsSlotDirty(slot_mask, slot_index))
		{
			D3D12_GPU_VIRTUAL_ADDRESS gpu_virtual_ptr = gpu_virtual_ptr_array->CurrentGPUVirtualAddress[shader_type][slot_index];
			(*direct_ctx->GetCmdList())->SetGraphicsRootConstantBufferView(base_index + slot_index, gpu_virtual_ptr);
			XD3D12CBVRootDescManager::CleanSlot(slot_mask, slot_index);
		}

	}
}






