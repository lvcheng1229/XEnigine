#include "D3D12PassStateManager.h"
#include "D3D12PlatformRHI.h"
void XD3D12PassStateManager::Create(XD3D12PhysicDevice* device_in, XD3DDirectContex* direct_ctx_in)
{
	direct_ctx = direct_ctx_in;
	pipe_curr_desc_array_manager.Create(device_in, direct_ctx_in);
}

void XD3D12PassStateManager::SetRenderTarget(uint32 num_rt, XD3D12RenderTargetView** rt_array_ptr, XD3D12DepthStenciltView* ds_ptr)
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

			direct_cmd_list->CmdListFlush();//TODO TODO

			direct_cmd_list->GetDXCmdList()->OMSetRenderTargets(
				PipelineState.Graphics.current_num_rendertarget,
				RTVDescriptors,
				true,
				&PipelineState.Graphics.depth_stencil->GetCPUPtr());
		}
		else
		{
			direct_cmd_list->CmdListFlush();//TODO TODO

			direct_cmd_list->GetDXCmdList()->OMSetRenderTargets(
				PipelineState.Graphics.current_num_rendertarget,
				RTVDescriptors,
				true,
				nullptr);
		}
		
	}
}
