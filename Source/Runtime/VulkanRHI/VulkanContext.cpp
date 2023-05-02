#include "VulkanContext.h"
#include "VulkanPlatformRHI.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRHIPrivate.h"

XVulkanCommandListContext::XVulkanCommandListContext(XVulkanPlatformRHI* InRHI, XVulkanDevice* InDevice, XVulkanQueue* InQueue)
	: RHI(InRHI)
	, Device(InDevice)
	, Queue(InQueue)
{
	CmdBufferManager = new XVulkanCommandBufferManager(Device, this);
}

XVulkanCommandListContext::~XVulkanCommandListContext()
{
	delete CmdBufferManager;
}
