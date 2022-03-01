#pragma once
#include "D3D12View.h"
#include "XTTTTTT.h"
class XD3D12PassStateManager
{
private:
	XD3DDirectContex* direct_ctx;
	bool bool_need_set_rt;
	struct
	{
		struct
		{
			uint32 current_num_rendertarget;
			XD3D12RenderTargetView* render_target_array[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];

			XD3D12DepthStenciltView* depth_stencil;

		}Graphics;

	}PipelineState;

	XD3D12PipelineCurrentDescArrayManager pipe_curr_desc_array_manager;
public:
	void Create(XD3D12PhysicDevice* device_in, XD3DDirectContex* direct_ctx_in);
	void SetRenderTarget(uint32 num_rt, XD3D12RenderTargetView** rt_array_ptr, XD3D12DepthStenciltView* ds_ptr);
	void ApplyCurrentStateToPipeline();
};