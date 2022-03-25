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
		std::array<XRHIRenderTargetView, 8>CachedRenderTargets;
		XRHIDepthStencilView CachedDepthStencilTarget;
	} PSOContext;
public:
	void CacheActiveRenderTargets()
	{

	}

	void SetContext(IRHIContext* InContext)
	{
		Context = InContext;
	}
	IRHIContext* GetContext()const
	{
		return Context;
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
};

class XRHIComputeCommandList :public XRHICommandListBase
{
public:
};

class XRHICommandList : public XRHIComputeCommandList
{
public:
	inline void SetGraphicsPipelineState(class XRHIGraphicsPSO* GraphicsPipelineState)
	{
		GetContext()->RHISetGraphicsPipelineState(GraphicsPipelineState);
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
