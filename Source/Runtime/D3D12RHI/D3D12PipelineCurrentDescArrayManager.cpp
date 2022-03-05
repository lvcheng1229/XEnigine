#include "D3D12PipelineCurrentDescArrayManager.h"
#include "D3D12Context.h"
void XD3D12PipelineCurrentDescArrayManager::Create(XD3D12PhysicDevice* device_in, XD3DDirectContex* direct_ctx_in)
{
	device = device_in;
	direct_ctx = direct_ctx_in;
	pipeline_current_desc_array.Create(device_in);
}

template void XD3D12PipelineCurrentDescArrayManager::SetDescTableSRVs<EShaderType::SV_Pixel>(
	const XD3D12RootSignature* root_signature,
	XD3D12PassShaderResourceManager* SRVManager,
	uint32& slot_start, uint32 slot_num);

template<EShaderType shader_type>
void XD3D12PipelineCurrentDescArrayManager::SetDescTableSRVs(
	const XD3D12RootSignature* root_signature,
	XD3D12PassShaderResourceManager* SRVManager,
	uint32& slot_start, uint32 slot_num)
{
	uint32 first_slot_index = slot_start;
	slot_start += slot_num;


	for (uint32 i = 0; i < slot_num; i++)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_ptr = pipeline_current_desc_array.GetCPUDescPtrByIndex(first_slot_index + i);
		
		device->GetDXDevice()->CopyDescriptors(
			1, &cpu_desc_ptr, 
			&slot_num, slot_num, 
			&SRVManager->Views[shader_type][i]->GetCPUPtr() ,
			nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc_ptr = pipeline_current_desc_array.GetGPUDescPtrByIndex(first_slot_index + i);
		uint32 slot_index = root_signature->GetSRVDescTableBindSlot(shader_type);
		(*direct_ctx->GetCmdList())->SetGraphicsRootDescriptorTable(slot_index, gpu_desc_ptr);
	}



}
template void XD3D12PipelineCurrentDescArrayManager::SetRootDescCBVs<EShaderType::SV_Vertex>(
	const XD3D12RootSignature* root_signature,
	XD3D12CBVRootDescManager* gpu_virtual_ptr_array,
	uint32 slot_num);
template void XD3D12PipelineCurrentDescArrayManager::SetRootDescCBVs<EShaderType::SV_Pixel>(
	const XD3D12RootSignature* root_signature,
	XD3D12CBVRootDescManager* gpu_virtual_ptr_array,
	uint32 slot_num);


template<EShaderType shader_type>
void XD3D12PipelineCurrentDescArrayManager::SetRootDescCBVs(
	const XD3D12RootSignature* root_signature,
	XD3D12CBVRootDescManager* gpu_virtual_ptr_array,
	uint32 slot_num)
{
	
	uint32 base_index = root_signature->GetCBVRootDescBindSlot(shader_type);
	for (uint32 slot_index = 0; slot_index < slot_num; ++slot_index)
	{
		D3D12_GPU_VIRTUAL_ADDRESS gpu_virtual_ptr = gpu_virtual_ptr_array->CurrentGPUVirtualAddress[shader_type][slot_index];
		(*direct_ctx->GetCmdList())->SetGraphicsRootConstantBufferView(base_index + slot_index, gpu_virtual_ptr);
	}
}






