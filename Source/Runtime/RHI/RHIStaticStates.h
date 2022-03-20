#pragma once
#include "RHIResource.h"

template<bool bEnableDepthWrite = true, ECompareFunction DepthTest = ECompareFunction::CF_Greater>
class TStaticDepthStencilState
{
public:
	std::shared_ptr<XRHIDepthStencilState> CreateRHI()
	{
		XDepthStencilStateInitializerRHI Initializer(bEnableDepthWrite, DepthTest);
	}
};