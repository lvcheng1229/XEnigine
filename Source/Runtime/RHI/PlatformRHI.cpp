#include "PlatformRHI.h"
#include "Runtime/HAL/Mch.h"
#include "Runtime/RenderCore/RenderResource.h"
#include "Runtime/RHI/RHICommandList.h"

#include "Runtime/D3D12RHI/D3D12PlatformRHI.h"

XPlatformRHI* GPlatformRHI = nullptr;
XRHICommandList GRHICmdList;
bool GIsRHIInitialized = false;

static XRHIModule* RHIModule;

void RHIRelease()
{
	RHIModule->ReleaseRHI();
	delete RHIModule;
}

void RHIInit(uint32 Width, uint32 Height)
{
	if (GPlatformRHI == nullptr)
	{
		RHIModule = new XD3D12RHIModule();
		GPlatformRHI = RHIModule->CreateRHI();
	}
	
	GRHICmdList.ReseizeViewport(Width, Height);
	GRHICmdList.Open();
	XRenderResource::InitRHIForAllResources();
	GRHICmdList.Execute();

	X_Assert(GIsRHIInitialized == false);
	GIsRHIInitialized = true;
}
