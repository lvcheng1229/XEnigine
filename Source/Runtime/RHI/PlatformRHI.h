#pragma once
#include <memory>
#include "RHIResource.h"



class XPlatformRHI
{
public:
	virtual void Init() = 0;
	virtual std::shared_ptr<XRHIVertexLayout> RHICreateVertexDeclaration(const XRHIVertexLayoutArray& Elements) = 0;
	virtual std::shared_ptr<XRHIDepthStencilState> RHICreateDepthStencilState(const XDepthStencilStateInitializerRHI& Initializer) = 0;
	virtual std::shared_ptr<XRHIBlendState> RHICreateBlendState(const XBlendStateInitializerRHI& Initializer) = 0;
	virtual std::shared_ptr<XRHIVertexShader> RHICreateVertexShader(XArrayView<uint8> Code) = 0;
	virtual std::shared_ptr<XRHIPixelShader> RHICreatePixelShader(XArrayView<uint8> Code) = 0;
	virtual std::shared_ptr<XRHIComputeShader> RHICreateComputeShader(XArrayView<uint8> Code) = 0;
	virtual std::shared_ptr<XRHIGraphicsPSO> RHICreateGraphicsPipelineState(const  XGraphicsPSOInitializer& PSOInit) = 0;
	virtual std::shared_ptr<XRHIComputePSO> RHICreateComputePipelineState(const XRHIComputeShader* RHIComputeShader) = 0;
};

extern XPlatformRHI* GPlatformRHI;
extern bool GIsRHIInitialized;
XPlatformRHI* PlatformCreateDynamicRHI();

inline std::shared_ptr<XRHIGraphicsPSO> RHICreateGraphicsPipelineState(const XGraphicsPSOInitializer& PSOInit)
{
	return GPlatformRHI->RHICreateGraphicsPipelineState(PSOInit);
}

inline std::shared_ptr<XRHIComputePSO> RHICreateComputePipelineState(const XRHIComputeShader* RHIComputeShader)
{
	return GPlatformRHI->RHICreateComputePipelineState(RHIComputeShader);
}

inline std::shared_ptr<XRHIVertexLayout> RHICreateVertexDeclaration(const XRHIVertexLayoutArray& Elements)
{
	return GPlatformRHI->RHICreateVertexDeclaration(Elements);
}


inline std::shared_ptr<XRHIDepthStencilState> RHICreateDepthStencilState(const XDepthStencilStateInitializerRHI& Initializer)
{
	return GPlatformRHI->RHICreateDepthStencilState(Initializer);
}

inline std::shared_ptr<XRHIBlendState> RHICreateBlendState(const XBlendStateInitializerRHI& Initializer)
{
	return GPlatformRHI->RHICreateBlendState(Initializer);
}