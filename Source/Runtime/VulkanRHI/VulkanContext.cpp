#include "VulkanContext.h"
#include "VulkanPlatformRHI.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRHIPrivate.h"
#include "VulkanDevice.h"
#include "VulkanViewport.h"
#include "VulkanPendingState.h"

XVulkanCommandListContext::XVulkanCommandListContext(XVulkanPlatformRHI* InRHI, XVulkanDevice* InDevice, XVulkanQueue* InQueue)
	: RHI(InRHI)
	, Device(InDevice)
	, Queue(InQueue)
{
	CmdBufferManager = new XVulkanCommandBufferManager(Device, this);
	PendingGfxState = new XVulkanPendingGfxState();
}

void XVulkanCommandListContext::RHIEndFrame()
{
	RHI->GetVulkanViewport()->Prsent();
}

XVulkanCommandListContext::~XVulkanCommandListContext()
{
	delete CmdBufferManager;
}
