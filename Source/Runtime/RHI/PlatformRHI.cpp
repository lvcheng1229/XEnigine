#include "PlatformRHI.h"
#include "Runtime/HAL/Mch.h"
#include "Runtime/RenderCore/RenderResource.h"//TODO

XPlatformRHI* GPlatformRHI = nullptr;
bool GIsRHIInitialized = false;

void RHIInit()
{
	if (GPlatformRHI == nullptr)
	{
		GPlatformRHI = PlatformCreateDynamicRHI();
	}
	
	XRenderResource::InitRHIForAllResources();

	X_Assert(GIsRHIInitialized == false);
	GIsRHIInitialized = true;
}
