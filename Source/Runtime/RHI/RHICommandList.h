#pragma once
#include "PlatformRHI.h"
#include "RHIContext.h"
#include <array>
class XRHICommandListBase
{
protected:
	struct FPSOContext
	{
		uint32 CachedNumRTs = 0;
		XRHIDepthStencilView CachedDepthStencilTarget;
		std::array<XRHIRenderTargetView, 8>CachedRenderTargets;
	} PSOContext;
public:

	void SetContext(IRHIContext* InContext)
	{
		Context = InContext;
		ComputeContext = InContext;
	}
	IRHIContext* GetContext()const
	{
		return Context;
	}

	inline void SetComputeContext(IRHIContext* Context)
	{
		ComputeContext = Context;
	}
	IRHIContext* GetComputeContext()const
	{
		return ComputeContext;
	}

	void CacheActiveRenderTargets(
		uint32 NewNumRTs,
		const XRHIRenderTargetView* NewRenderTargetsRHI,
		const XRHIDepthStencilView* NewDepthStencilRHI)
	{
		PSOContext.CachedNumRTs = NewNumRTs;
		for (int i = 0; i < NewNumRTs; i++)
		{
			PSOContext.CachedRenderTargets[i] = NewRenderTargetsRHI[i];
		}
		PSOContext.CachedDepthStencilTarget = (NewDepthStencilRHI == nullptr ? XRHIDepthStencilView() : *NewDepthStencilRHI);
	}

	void CacheActiveRenderTargets(const XRHIRenderPassInfo& Info)
	{
		XRHISetRenderTargetsInfo RTInfo;
		Info.ConvertToRenderTargetsInfo(RTInfo);
		CacheActiveRenderTargets(RTInfo.NumColorRenderTargets, RTInfo.ColorRenderTarget, &RTInfo.DepthStencilRenderTarget);
	}

	void ApplyCachedRenderTargets(XGraphicsPSOInitializer& GraphicsPSOInit)
	{
		GraphicsPSOInit.RTNums = PSOContext.CachedNumRTs;
		for (uint32 i = 0; i < PSOContext.CachedNumRTs; i++)
		{
			GraphicsPSOInit.RT_Format[i] = PSOContext.CachedRenderTargets[i].Texture->GetFormat();
		}

		if (PSOContext.CachedDepthStencilTarget.Texture != nullptr)
		{
			GraphicsPSOInit.DS_Format = PSOContext.CachedDepthStencilTarget.Texture->GetFormat();
		}
		else
		{
			GraphicsPSOInit.DS_Format = EPixelFormat::FT_Unknown;
		}
	}
private:
	IRHIContext* Context;
	IRHIContext* ComputeContext;
};

class XRHIComputeCommandList :public XRHICommandListBase
{
public:
	inline void RHIDispatchComputeShader(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ)
	{
		GetComputeContext()->RHIDispatchComputeShader(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
	}

	inline void SetComputePipelineState(class XRHIComputePSO* ComputesPipelineState)
	{
		GetComputeContext()->RHISetComputePipelineState(ComputesPipelineState);
	}

	inline void SetConstantBuffer(EShaderType ShaderType, uint32 BufferIndex, XRHIConstantBuffer* RHICBV)
	{
		GetComputeContext()->RHISetShaderConstantBuffer(ShaderType, BufferIndex, RHICBV);
	}

	inline void SetShaderTexture(EShaderType ShaderType, uint32 TextureIndex, XRHITexture* Texture)
	{
		GetComputeContext()->RHISetShaderTexture(ShaderType, TextureIndex, Texture);
	}

	inline void SetShaderUAV(EShaderType ShaderType, uint32 TextureIndex, XRHIUnorderedAcessView* RHIUAV)
	{
		GetComputeContext()->RHISetShaderUAV(ShaderType, TextureIndex, RHIUAV);
	}
};

class XRHICommandList : public XRHIComputeCommandList
{
public:
	inline void RHIBeginRenderPass(const XRHIRenderPassInfo& InInfo, const wchar_t* InName)
	{
		GetContext()->RHIBeginRenderPass(InInfo, InName);
	}

	inline void RHIDrawIndexedPrimitive()
	{
		GetContext()->RHIDrawIndexedPrimitive();
	}

	inline void SetGraphicsPipelineState(class XRHIGraphicsPSO* GraphicsPipelineState)
	{
		GetContext()->RHISetGraphicsPipelineState(GraphicsPipelineState);
	}

	inline void SetConstantBuffer(EShaderType ShaderType, uint32 BufferIndex, XRHIConstantBuffer* RHICBV)
	{
		GetContext()->RHISetShaderConstantBuffer(ShaderType, BufferIndex, RHICBV);
	}

	inline void SetShaderValue(EShaderType ShaderType, uint32 BufferIndex,uint32 VariableOffsetInBuffer, uint32 NumBytes, const void* NewValue)
	{
		GetContext()->SetShaderValue(ShaderType, BufferIndex, VariableOffsetInBuffer, NumBytes, NewValue);
	}

	inline void SetShaderTexture(EShaderType ShaderType, uint32 TextureIndex, XRHITexture* Texture)
	{
		GetContext()->RHISetShaderTexture(ShaderType, TextureIndex, Texture);
	}
};


inline std::shared_ptr<XRHIVertexShader> RHICreateVertexShader(XArrayView<uint8> Code)
{
	return GPlatformRHI->RHICreateVertexShader(Code);
}

inline std::shared_ptr<XRHIPixelShader> RHICreatePixelShader(XArrayView<uint8> Code)
{
	return GPlatformRHI->RHICreatePixelShader(Code);
}

inline std::shared_ptr<XRHIComputeShader> RHICreateComputeShader(XArrayView<uint8> Code)
{
	return GPlatformRHI->RHICreateComputeShader(Code);
}
