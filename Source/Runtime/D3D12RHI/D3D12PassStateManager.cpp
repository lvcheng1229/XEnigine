#include "D3D12PassStateManager.h"
#include "D3D12PlatformRHI.h"
#include "D3D12Context.h"


void XD3D12PassStateManager::Create(XD3D12PhysicDevice* device_in, XD3DDirectContex* direct_ctx_in)
{
	direct_ctx = direct_ctx_in;
	pipe_curr_desc_array_manager.Create(device_in, direct_ctx_in);

	bool_need_set_rt = false;
	bNeedSetSRV = false;
	bNeedSetCBV = false;
	bNeedSetRootSig = false;
	memset(NumSRVs, 0, sizeof(uint32) * EShaderType::SV_ShaderCount);
	memset(NumCBVs, 0, sizeof(uint32) * EShaderType::SV_ShaderCount);
	memset(PipelineState.Common.SRVManager.Views, 0, sizeof(XD3D12PassShaderResourceManager)); 
}

void XD3D12PassStateManager::ResetState()
{
	bool_need_set_rt = false;
	bNeedSetSRV = false;
	bNeedSetCBV = false;
	bNeedSetRootSig = false;
	memset(NumSRVs, 0, sizeof(uint32) * EShaderType::SV_ShaderCount);
	memset(NumCBVs, 0, sizeof(uint32) * EShaderType::SV_ShaderCount);
	memset(PipelineState.Common.SRVManager.Views, 0, sizeof(XD3D12PassShaderResourceManager));
}


void XD3D12PassStateManager::GetRenderTargets(uint32& num_rt, XD3D12RenderTargetView** ptr_to_rt_array_ptr, XD3D12DepthStencilView** ptr_to_ds_ptr)
{
	num_rt = PipelineState.Graphics.current_num_rendertarget;
	*ptr_to_rt_array_ptr = *PipelineState.Graphics.render_target_array;
	*ptr_to_ds_ptr = PipelineState.Graphics.depth_stencil;
}

void XD3D12PassStateManager::SetRenderTarget(uint32 num_rt, XD3D12RenderTargetView** rt_array_ptr, XD3D12DepthStencilView* ds_ptr)
{
	bool_need_set_rt = true;
	PipelineState.Graphics.depth_stencil = ds_ptr;
	memset(PipelineState.Graphics.render_target_array, 0, num_rt * sizeof(XD3D12RenderTargetView));
	memcpy(PipelineState.Graphics.render_target_array, rt_array_ptr, num_rt * sizeof(XD3D12RenderTargetView));

	uint32 active_rt = 0;
	for (uint32 i = 0; i < num_rt; ++i)
	{
		if (rt_array_ptr[i] != nullptr)++active_rt;
	}
	PipelineState.Graphics.current_num_rendertarget = active_rt;
}

void XD3D12PassStateManager::ApplyCurrentStateToPipeline()
{
	XD3D12DirectCommandList* direct_cmd_list = direct_ctx->GetCmdList();
	ID3D12GraphicsCommandList* dx_cmd_list = direct_cmd_list->GetDXCmdList();
	if (bool_need_set_rt)
	{
		bool_need_set_rt = false;
		D3D12_CPU_DESCRIPTOR_HANDLE RTVDescriptors[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		for (uint32 i = 0; i < PipelineState.Graphics.current_num_rendertarget; ++i)
		{
			if(PipelineState.Graphics.render_target_array[i] != NULL)
			{
				XD3D12PlatformRHI::TransitionResource(
					*direct_cmd_list,
					PipelineState.Graphics.render_target_array[i],
					D3D12_RESOURCE_STATE_RENDER_TARGET);
				RTVDescriptors[i] = PipelineState.Graphics.render_target_array[i]->GetCPUPtr();
			}
		}

		
		if (PipelineState.Graphics.depth_stencil != nullptr)
		{
			XD3D12PlatformRHI::TransitionResource(
				*direct_cmd_list,
				PipelineState.Graphics.depth_stencil,
				D3D12_RESOURCE_STATE_DEPTH_WRITE);

			direct_cmd_list->CmdListFlushBarrier();//TODO TODO

			direct_cmd_list->GetDXCmdList()->OMSetRenderTargets(
				PipelineState.Graphics.current_num_rendertarget,
				RTVDescriptors,
				true,
				&PipelineState.Graphics.depth_stencil->GetCPUPtr());
		}
		else
		{
			direct_cmd_list->CmdListFlushBarrier();//TODO TODO

			direct_cmd_list->GetDXCmdList()->OMSetRenderTargets(
				PipelineState.Graphics.current_num_rendertarget,
				RTVDescriptors,
				true,
				nullptr);
		}
		
	}

	if (bNeedSetRootSig)
	{
		direct_cmd_list->GetDXCmdList()->SetGraphicsRootSignature(PipelineState.Common.RootSignature->GetDXRootSignature());
		bNeedSetRootSig = false;
	}

	ID3D12DescriptorHeap* descriptorHeaps[] = { pipe_curr_desc_array_manager.GetCurrentDescArray()->GetDescHeapPtr() };
	dx_cmd_list->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	dx_cmd_list->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	uint32 desc_table_slot_start = 0;
	if (bNeedSetSRV)
	{
		pipe_curr_desc_array_manager.SetDescTableSRVs<EShaderType::SV_Pixel>(
			PipelineState.Common.RootSignature, 
			&PipelineState.Common.SRVManager,
			desc_table_slot_start,
			NumSRVs[EShaderType::SV_Pixel]);

		bNeedSetSRV = false;
	}

	if (bNeedSetCBV)
	{
		pipe_curr_desc_array_manager.SetRootDescCBVs<EShaderType::SV_Vertex>(
			PipelineState.Common.RootSignature,
			&PipelineState.Common.CBVRootDescManager,
			NumCBVs[EShaderType::SV_Vertex]);

		pipe_curr_desc_array_manager.SetRootDescCBVs<EShaderType::SV_Pixel>(
			PipelineState.Common.RootSignature,
			&PipelineState.Common.CBVRootDescManager,
			NumCBVs[EShaderType::SV_Pixel]);

		bNeedSetCBV = false;
	}
}
