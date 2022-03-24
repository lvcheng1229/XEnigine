#include "PlatformRHI.h"
#include "Runtime/RenderCore/RenderResource.h"
#include "Runtime/HAL/Mch.h"
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
