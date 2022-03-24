#pragma once
#include "D3D12View.h"
#include "D3D12PipelineCurrentDescArrayManager.h"

//
#include "Runtime/RenderCore/Shader.h"
//

#include "D3D12PipelineState.h"
#include <memory>

enum class ED3D12PipelineType
{
	D3D12PT_Graphics,
	D3D12PT_Compute,
};
class XD3D12PassStateManager
{
private:
	XD3DDirectContex* direct_ctx;
	bool bNeedSetPSO;
	bool bNeedSetRT;
	bool bNeedSetDS;
	//bool bNeedSetSRV;
	//bool bNeedSetUAV;
	//bool bNeedSetCBV;
	bool bNeedSetRootSig;
	//bool bNeedSetRootSigNew;
	bool bNeedClearMRT;
	bool bNeedSetHeapDesc;
	//uint32 CurrentDescHeapSlotIndex;
	//uint32 NumSRVs[EShaderType::SV_ShaderCount];
	//uint32 NumCBVs[EShaderType::SV_ShaderCount];
	struct
	{
		struct
		{
			XD3DGraphicsPSO* D3DGraphicsPSO;
			uint32						current_num_rendertarget;
			XD3D12DepthStencilView*		depth_stencil;
			XD3D12RenderTargetView*		render_target_array[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];

		}Graphics;

		struct
		{
			ID3D12PipelineState* ID3DPSO;

			const XD3D12RootSignature*				RootSignature;
			XD3D12PassShaderResourceManager		SRVManager;
			XD3D12PassUnorderedAcessManager		UAVManager;
			XD3D12CBVRootDescManager			CBVRootDescManager;

			uint32 NumSRVs[EShaderType_Underlying(EShaderType::SV_ShaderCount)];
			uint32 NumUAVs[EShaderType_Underlying(EShaderType::SV_ShaderCount)];
		}Common;

	}PipelineState;

	XD3D12PipelineCurrentDescArrayManager pipe_curr_desc_array_manager;
public:
	inline const XD3D12RootSignature* GetCurrentRootSig()
	{
		//return PipelineState.Graphics.D3DGraphicsPSO.get() ? PipelineState.Graphics.D3DGraphicsPSO->RootSig : nullptr;
		return PipelineState.Common.RootSignature;
	}

	inline void TempResetPSO(XD3DGraphicsPSO* ptrl)
	{
		if (PipelineState.Graphics.D3DGraphicsPSO)
			PipelineState.Graphics.D3DGraphicsPSO = nullptr;
	}

	inline void SetGraphicsPipelineState(XD3DGraphicsPSO* GraphicsPipelineState)
	{
		if (PipelineState.Graphics.D3DGraphicsPSO != GraphicsPipelineState)
		{
			if (GetCurrentRootSig() != GraphicsPipelineState->RootSig)
			{
				bNeedSetRootSig = true;

				//Temp
				PipelineState.Common.RootSignature = GraphicsPipelineState->RootSig;
			}

			bNeedSetPSO = true;
			PipelineState.Graphics.D3DGraphicsPSO = GraphicsPipelineState;
			PipelineState.Common.ID3DPSO = GraphicsPipelineState->XID3DPSO->GetID3DPSO();
			
			
		}
	}

	inline void ResetDescHeapIndex() { pipe_curr_desc_array_manager.GetCurrentDescArray()->ResetIndexToZero(); }

	void ResetState();
	void Create(XD3D12PhysicDevice* device_in, XD3DDirectContex* direct_ctx_in);
	void SetRenderTarget(uint32 num_rt, XD3D12RenderTargetView** rt_array_ptr, XD3D12DepthStencilView* ds_ptr);
	
	template<EShaderType ShaderType>
	inline void SetShader(XShader* ShaderIn)
	{
		PipelineState.Common.NumSRVs[EShaderType_Underlying(ShaderType)] = ShaderIn != nullptr ? ShaderIn->GetSRVCount() : 0;
		PipelineState.Common.NumUAVs[EShaderType_Underlying(ShaderType)] = ShaderIn != nullptr ? ShaderIn->GetUAVCount() : 0;
	}

	inline void SetHeapDesc() { bNeedSetHeapDesc = true; }
	inline void SetRootSignature(XD3D12RootSignature* root_sig) { 
		if (PipelineState.Common.RootSignature != root_sig)
		{
			PipelineState.Common.RootSignature = root_sig;
			bNeedSetRootSig = true;
		}
	};

	template<EShaderType ShaderType>
	inline void SetUAV(XD3D12UnorderedAcessView* UAV, uint32 UAVIndex)
	{
		auto& View = PipelineState.Common.UAVManager.Views[EShaderType_Underlying(ShaderType)][UAVIndex];
		if (View != UAV)
		{
			View = UAV;
			//bNeedSetUAV = true;
			XD3D12PassUnorderedAcessManager::SetSlotNeedSetBit(PipelineState.Common.UAVManager.Mask[EShaderType_Underlying(ShaderType)], UAVIndex);
		}
	}

	template<EShaderType ShaderType>
	inline void SetShaderResourceView(XD3D12ShaderResourceView* SRV, uint32 resource_index)
	{
		auto& View = PipelineState.Common.SRVManager.Views[EShaderType_Underlying(ShaderType)][resource_index];
		if (View != SRV)
		{
			View = SRV;
			//bNeedSetSRV = true;
			XD3D12PassShaderResourceManager::SetSlotNeedSetBit(PipelineState.Common.SRVManager.Mask[EShaderType_Underlying(ShaderType)], resource_index);
			//NumSRVs[ShaderType]++;
		}
	}

	template<EShaderType ShaderType>
	inline void SetCBV(XD3D12ConstantBuffer* CBuffer, uint32 resource_index)
	{
		auto& GpuVirtualPtr = PipelineState.Common.CBVRootDescManager.CurrentGPUVirtualAddress[EShaderType_Underlying(ShaderType)][resource_index];
		if (GpuVirtualPtr != CBuffer->ResourceLocation.GetGPUVirtualPtr())
		{
			GpuVirtualPtr = CBuffer->ResourceLocation.GetGPUVirtualPtr();
			//bNeedSetCBV = true;
			XD3D12CBVRootDescManager::SetSlotNeedSetBit(PipelineState.Common.CBVRootDescManager.Mask[EShaderType_Underlying(ShaderType)], resource_index);
			//NumCBVs[ShaderType]++;
		}
	}

	template<ED3D12PipelineType PipelineType>
	void ApplyCurrentStateToPipeline();
};