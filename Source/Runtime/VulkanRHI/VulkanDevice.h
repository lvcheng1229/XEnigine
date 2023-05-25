#pragma once
#include "VulkanPlatformRHI.h"
#include "VulkanQueue.h"
#include "VulkanContext.h"
#include "VulkanResource.h"

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

	XVulkanShaderFactory* GetVkShaderFactory() { return &ShaderFactory; }

	class XVulkanPipelineStateCacheManager* PipelineStateCache;
private:
	friend class VkHack;

	VkDevice Device;
	VkPhysicalDevice Gpu;
	XVulkanPlatformRHI* RHI;

	uint32 GfxQueueIndex;

	XVulkanQueue* GfxQueue;
	XVulkanQueue* PresentQueue;

	XVulkanCommandListContext* GfxContext;
	XVulkanShaderFactory ShaderFactory;
};