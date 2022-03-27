#pragma once
#include <memory>
#include "D3D12View.h"
#include "D3D12Shader.h"
#include "D3D12PipelineState.h"
#include "D3D12OnlineDescArrayManager.h"


//TODO : Remove
#include "Runtime/RenderCore/Shader.h"

enum class ED3D12PipelineType
{
	D3D12PT_Graphics,
	D3D12PT_Compute,
};

class XD3D12PassStateManager
{
private:
	XD3D12OnlineDescArrayManager pipe_curr_desc_array_manager;
	XD3DDirectContex* direct_ctx;
	bool bNeedSetPSO;
	bool bNeedSetRT;
	bool bNeedSetRootSig;
	bool bNeedClearMRT;
	bool bNeedSetHeapDesc;

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
			XD3DComputePSO* D3DComputePSO;
		}Compute;

		struct
		{
			ID3D12PipelineState* ID3DPSO;

			const XD3D12RootSignature*			RootSignature;
			XD3D12PassShaderResourceManager		SRVManager;
			XD3D12PassUnorderedAcessManager		UAVManager;
			XD3D12CBVRootDescManager			CBVRootDescManager;

			uint32 NumSRVs[EShaderType_Underlying(EShaderType::SV_ShaderCount)];
			uint32 NumUAVs[EShaderType_Underlying(EShaderType::SV_ShaderCount)];
		}Common;

	}PipelineState;

	
public:
	
	void ResetState();
	void Create(XD3D12PhysicDevice* device_in, XD3DDirectContex* direct_ctx_in);
	void SetRenderTarget(uint32 num_rt, XD3D12RenderTargetView** rt_array_ptr, XD3D12DepthStencilView* ds_ptr);

	template<ED3D12PipelineType PipelineType>
	void ApplyCurrentStateToPipeline();

	inline const XD3D12RootSignature* GetCurrentRootSig()
	{
		return PipelineState.Common.RootSignature;
	}

	inline void TempResetPSO(XD3DGraphicsPSO* ptrl)
	{
		if (PipelineState.Graphics.D3DGraphicsPSO)
			PipelineState.Graphics.D3DGraphicsPSO = nullptr;
		if (PipelineState.Compute.D3DComputePSO)
			PipelineState.Compute.D3DComputePSO = nullptr;
	}

	inline XD3D12VertexShader* GetXD3D12VertexShader(XD3DGraphicsPSO* GraphicsPipelineState)
	{
		return static_cast<XD3D12VertexShader*>(GraphicsPipelineState->GraphicsPSOInitializer.BoundShaderState.RHIVertexShader);
	}

	inline XD3D12PixelShader* GetXD3D12PixelShader(XD3DGraphicsPSO* GraphicsPipelineState)
	{
		return static_cast<XD3D12PixelShader*>(GraphicsPipelineState->GraphicsPSOInitializer.BoundShaderState.RHIPixelShader);
	}

	inline void SetVertexShaderResource(XD3DGraphicsPSO* GraphicsPipelineState)
	{
		XD3D12VertexShader* Shader = GetXD3D12VertexShader(GraphicsPipelineState);
		PipelineState.Common.NumSRVs[(int)EShaderType::SV_Vertex] = Shader != nullptr ? Shader->ResourceCount.NumSRV : 0;
		PipelineState.Common.NumUAVs[(int)EShaderType::SV_Vertex] = Shader != nullptr ? Shader->ResourceCount.NumUAV : 0;
	}

	inline void SetPixelShaderResource(XD3DGraphicsPSO* GraphicsPipelineState)
	{
		XD3D12PixelShader* Shader = GetXD3D12PixelShader(GraphicsPipelineState);
		PipelineState.Common.NumSRVs[(int)EShaderType::SV_Pixel] = Shader != nullptr ? Shader->ResourceCount.NumSRV : 0;
		PipelineState.Common.NumUAVs[(int)EShaderType::SV_Pixel] = Shader != nullptr ? Shader->ResourceCount.NumUAV : 0;
	}

	inline void SetComputePipelineState(XD3DComputePSO* ComputePipelineState)
	{
		if (PipelineState.Compute.D3DComputePSO != ComputePipelineState)
		{
			PipelineState.Common.NumSRVs[(int)EShaderType::SV_Vertex] = 0;
			PipelineState.Common.NumUAVs[(int)EShaderType::SV_Vertex] = 0;
			PipelineState.Common.NumSRVs[(int)EShaderType::SV_Pixel] = 0;
			PipelineState.Common.NumUAVs[(int)EShaderType::SV_Pixel] = 0;

			const XD3D12ComputeShader* ComputeShader = ComputePipelineState->ComputeShader;
			PipelineState.Common.NumSRVs[(int)EShaderType::SV_Compute] = ComputeShader->ResourceCount.NumSRV;
			PipelineState.Common.NumUAVs[(int)EShaderType::SV_Compute] = ComputeShader->ResourceCount.NumUAV;

			if (PipelineState.Common.RootSignature != ComputePipelineState->ComputeShader->RootSignature)
			{
				PipelineState.Common.RootSignature = ComputePipelineState->ComputeShader->RootSignature;
				bNeedSetRootSig = true;
			}

			bNeedSetPSO = true;
			PipelineState.Compute.D3DComputePSO = ComputePipelineState;
			PipelineState.Common.ID3DPSO = ComputePipelineState->XID3DPSO->GetID3DPSO();
		}
	}

	inline void SetGraphicsPipelineState(XD3DGraphicsPSO* GraphicsPipelineState)
	{
		if (PipelineState.Graphics.D3DGraphicsPSO != GraphicsPipelineState)
		{
			SetVertexShaderResource(GraphicsPipelineState);
			SetPixelShaderResource(GraphicsPipelineState);
			PipelineState.Common.NumSRVs[(int)EShaderType::SV_Compute] = 0;
			PipelineState.Common.NumUAVs[(int)EShaderType::SV_Compute] = 0;

			if (GetCurrentRootSig() != GraphicsPipelineState->RootSig)
			{
				PipelineState.Common.RootSignature = GraphicsPipelineState->RootSig;
				bNeedSetRootSig = true;
			}

			bNeedSetPSO = true;
			PipelineState.Graphics.D3DGraphicsPSO = GraphicsPipelineState;
			PipelineState.Common.ID3DPSO = GraphicsPipelineState->XID3DPSO->GetID3DPSO();
		}
	}

	inline void ResetDescHeapIndex() { pipe_curr_desc_array_manager.GetCurrentDescArray()->ResetIndexToZero(); }
	
	template<EShaderType ShaderType>
	inline void SetShader(XShader* ShaderIn)
	{
		PipelineState.Common.NumSRVs[EShaderType_Underlying(ShaderType)] = ShaderIn != nullptr ? ShaderIn->GetSRVCount() : 0;
		PipelineState.Common.NumUAVs[EShaderType_Underlying(ShaderType)] = ShaderIn != nullptr ? ShaderIn->GetUAVCount() : 0;
	}

	inline void SetHeapDesc() 
	{
		bNeedSetHeapDesc = true; 
	}

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
			XD3D12PassShaderResourceManager::SetSlotNeedSetBit(PipelineState.Common.SRVManager.Mask[EShaderType_Underlying(ShaderType)], resource_index);
		}
	}

	template<EShaderType ShaderType>
	inline void SetCBV(XD3D12ConstantBuffer* CBuffer, uint32 resource_index)
	{
		auto& GpuVirtualPtr = PipelineState.Common.CBVRootDescManager.CurrentGPUVirtualAddress[EShaderType_Underlying(ShaderType)][resource_index];
		if (GpuVirtualPtr != CBuffer->ResourceLocation.GetGPUVirtualPtr())
		{
			GpuVirtualPtr = CBuffer->ResourceLocation.GetGPUVirtualPtr();
			XD3D12CBVRootDescManager::SetSlotNeedSetBit(PipelineState.Common.CBVRootDescManager.Mask[EShaderType_Underlying(ShaderType)], resource_index);
		}
	}
};