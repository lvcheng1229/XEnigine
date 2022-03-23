#pragma once
#include <memory>
#include "RHIResource.h"



class XPlatformRHI
{
public:
	virtual void Init() = 0;
	virtual std::shared_ptr<XRHIDepthStencilState> RHICreateDepthStencilState(const XDepthStencilStateInitializerRHI& Initializer) = 0;
	virtual std::shared_ptr<XRHIBlendState> RHICreateBlendState(const XBlendStateInitializerRHI& Initializer) = 0;
	virtual std::shared_ptr<XRHIVertexShader> RHICreateVertexShader(XArrayView<uint8> Code) = 0;
	virtual std::shared_ptr<XRHIPixelShader> RHICreatePixelShader(XArrayView<uint8> Code) = 0;
	virtual void RHIBeginRenderPass(const XRHIRenderPassInfo& InInfo, const wchar_t* InName) = 0;
};

extern XPlatformRHI* GPlatformRHI;
extern bool GIsRHIInitialized;
XPlatformRHI* PlatformCreateDynamicRHI();



inline std::shared_ptr<XRHIDepthStencilState> RHICreateDepthStencilState(const XDepthStencilStateInitializerRHI& Initializer)
{
	return GPlatformRHI->RHICreateDepthStencilState(Initializer);
}

inline std::shared_ptr<XRHIBlendState> RHICreateBlendState(const XBlendStateInitializerRHI& Initializer)
{
	return GPlatformRHI->RHICreateBlendState(Initializer);
}