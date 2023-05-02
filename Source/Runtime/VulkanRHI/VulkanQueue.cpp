#include "VulkanPlatformRHI.h"
#include "VulkanQueue.h"
#include "VulkanDevice.h"

XVulkanQueue::XVulkanQueue(XVulkanDevice* VulkanDevice, uint32 InFamilyIndex)
	: Queue(VK_NULL_HANDLE)
	, FamilyIndex(InFamilyIndex)
	, Device(VulkanDevice)
{
	vkGetDeviceQueue(Device->GetVkDevice(), FamilyIndex, 0, &Queue);
}
