#pragma once
#include <memory>
#include "RHIResource.h"



class XPlatformRHI
{
public:
	virtual void Init() = 0;
	virtual std::shared_ptr<XRHIDepthStencilState> RHICreateDepthStencilState(const XDepthStencilStateInitializerRHI& Initializer) = 0;
};

extern XPlatformRHI* GPlatformRHI;
XPlatformRHI* PlatformCreateDynamicRHI();



inline std::shared_ptr<XRHIDepthStencilState> RHICreateDepthStencilState(const XDepthStencilStateInitializerRHI& Initializer)
{
	return GPlatformRHI->RHICreateDepthStencilState(Initializer);
}