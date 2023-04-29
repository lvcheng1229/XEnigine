#include "PlatformRHI.h"
#include "Runtime/HAL/Mch.h"
#include "Runtime/RenderCore/RenderResource.h"
#include "Runtime/RHI/RHICommandList.h"

#include "Runtime/VulkanRHI/VulkanPlatformRHI.h"
#include "Runtime/D3D12RHI/D3D12PlatformRHI.h"

XPlatformRHI* GPlatformRHI = nullptr;
XRHICommandList GRHICmdList;
bool GIsRHIInitialized = false;

void RHIRelease()
{
	delete GPlatformRHI;
}

void RHIInit(uint32 Width, uint32 Height , bool bUseDX12)
{
	if (GPlatformRHI == nullptr)
	{
		if (bUseDX12)
		{
			GPlatformRHI = new XD3D12PlatformRHI();
		}
		else
		{
			GPlatformRHI = new XVulkanPlatformRHI();
		}
		
		GPlatformRHI->Init();
	}
	
#if USE_DX12
	GRHICmdList.ReseizeViewport(Width, Height);
	GRHICmdList.Open();
	XRenderResource::InitRHIForAllResources();
	GRHICmdList.Execute();
#endif USE_DX12

	XASSERT(GIsRHIInitialized == false);
	GIsRHIInitialized = true;
}
