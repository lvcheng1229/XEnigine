#pragma once
#include "VulkanPlatformRHI.h"
#include "VulkanQueue.h"
#include "VulkanContext.h"
#include "VulkanResource.h"
#include "VulkanMemory.h"

class XVulkanDescriptorPoolsManager;

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

	XMemoryManager& GetMemoryManager() { return MemoryManager; }
	XStagingManager& GetStagingManager() { return StagingManager; }
	XDeviceMemoryManager& GetDeviceMemoryManager() { return DeviceMemoryManager; }

	XVulkanDescriptorPoolsManager* GetDescriptorPoolsManager(){return DescriptorPoolsManager;}

	inline XFenceManager& GetFenceManager()
	{
		return FenceManager;
	}

	inline const XVulkanSamplerState& GetDefaultSampler() const
	{
		return *DefaultSampler;
	}

	inline const XVulkanTextureView& GetDefaultImageView() const
	{
		return DefaultTextureView;
	}

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

	XFenceManager FenceManager;
	XMemoryManager MemoryManager;
	XDeviceMemoryManager DeviceMemoryManager;

	XStagingManager StagingManager;

	XVulkanDescriptorPoolsManager* DescriptorPoolsManager = nullptr;

	XVulkanSamplerState* DefaultSampler;
	XVulkanTextureView DefaultTextureView;
};