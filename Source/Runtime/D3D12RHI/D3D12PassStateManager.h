#pragma once
#include "D3D12View.h"
#include "D3D12PipelineCurrentDescArrayManager.h"

#include <memory>

class XD3D12PassStateManager
{
private:
	XD3DDirectContex* direct_ctx;
	bool bool_need_set_rt;
	bool bNeedSetSRV;
	bool bNeedSetCBV;
	bool bNeedSetRootSig;
	bool bNeedClearMRT;
	//uint32 NumSRVs[EShaderType::SV_ShaderCount];
	//uint32 NumCBVs[EShaderType::SV_ShaderCount];
	struct
	{
		struct
		{
			uint32 current_num_rendertarget;
			XD3D12RenderTargetView* render_target_array[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];

			XD3D12DepthStencilView* depth_stencil;

		}Graphics;

		struct
		{
			XD3D12RootSignature* RootSignature;
			XD3D12PassShaderResourceManager SRVManager;
			XD3D12CBVRootDescManager CBVRootDescManager;
			
		}Common;

	}PipelineState;

	XD3D12PipelineCurrentDescArrayManager pipe_curr_desc_array_manager;
public:
	inline void SetRootSignature(XD3D12RootSignature* root_sig) { 
		if (PipelineState.Common.RootSignature != root_sig)
		{
			PipelineState.Common.RootSignature = root_sig;
			bNeedSetRootSig = true;
		}
	};
	void Create(XD3D12PhysicDevice* device_in, XD3DDirectContex* direct_ctx_in);
	void ResetState();

	template<EShaderType ShaderType>
	inline void SetShaderResourceView(XD3D12ShaderResourceView* SRV, uint32 resource_index)
	{
		auto& View = PipelineState.Common.SRVManager.Views[ShaderType][resource_index];
		if (View != SRV)
		{
			View = SRV;
			bNeedSetSRV = true;
			XD3D12PassShaderResourceManager::DirtySlot(PipelineState.Common.SRVManager.Mask[ShaderType], resource_index);
			//NumSRVs[ShaderType]++;
		}

	}

	template<EShaderType ShaderType>
	inline void SetCBV(XD3D12ConstantBuffer* CBuffer, uint32 resource_index)
	{
		auto& GpuVirtualPtr = PipelineState.Common.CBVRootDescManager.CurrentGPUVirtualAddress[ShaderType][resource_index];
		if (GpuVirtualPtr != CBuffer->ResourceLocation.GetGPUVirtualPtr())
		{
			GpuVirtualPtr = CBuffer->ResourceLocation.GetGPUVirtualPtr();
			bNeedSetCBV = true;
			XD3D12CBVRootDescManager::DirtySlot(PipelineState.Common.CBVRootDescManager.Mask[ShaderType], resource_index);
			//NumCBVs[ShaderType]++;
		}
	}

	//void GetRenderTargets(uint32& num_rt, XD3D12RenderTargetView** rt_array_ptr, XD3D12DepthStencilView** ds_ptr);
	void SetRenderTarget(uint32 num_rt, XD3D12RenderTargetView** rt_array_ptr, XD3D12DepthStencilView* ds_ptr);
	void ApplyCurrentStateToPipeline();
};