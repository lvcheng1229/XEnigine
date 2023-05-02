#pragma once
#include "VulkanPlatformRHI.h"
#include "VulkanQueue.h"
#include "VulkanContext.h"
class XVulkanDevice
{
public:
	XVulkanDevice(XVulkanPlatformRHI* InRHI, VkPhysicalDevice InGpu);
	~XVulkanDevice();
	
	void CreateDevice();
	void SetPresentQueue(VkSurfaceKHR Surface);
	void InitGPU();

	VkDevice GetVkDevice()
	{
		return Device;
	}

	VkPhysicalDevice* GetVkPhysicalDevice()
	{
		return &Gpu;
	}

	XVulkanCommandListContext* GetGfxContex() { return GfxContext; }
private:
	friend class VkHack;

	VkDevice Device;
	VkPhysicalDevice Gpu;
	XVulkanPlatformRHI* RHI;

	uint32 GfxQueueIndex;

	XVulkanQueue* GfxQueue;
	XVulkanQueue* PresentQueue;

	XVulkanCommandListContext* GfxContext;
};