#include "PlatformRHI.h"
#include "Runtime/RenderCore/RenderResource.h"

XPlatformRHI* GPlatformRHI = nullptr;
bool GIsRHIInitialized = false;
void RHIInit()
{
	if (GPlatformRHI == nullptr)
	{
		GPlatformRHI = PlatformCreateDynamicRHI();
	}

	XRenderResource::InitRHIForAllResources();

	GIsRHIInitialized = true;
}
