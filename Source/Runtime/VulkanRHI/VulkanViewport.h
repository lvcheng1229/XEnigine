#pragma once
#include "vulkan\vulkan_core.h"
#include "Runtime\Core\PixelFormat.h"
#include <vector>
#include "VulkanResource.h"
class XVulkanDevice;
class XVulkanSwapChain;

class XVulkanBackBuffer : XVulkanTexture2D
{
public:
	//XVulkanBackBuffer();
};

class XVulkanViewport
{
public:
	XVulkanViewport(EPixelFormat& InOutPixelFormat, XVulkanDevice* VulkanDevice, void* InWindowHandle, VkInstance InInstance);
	~XVulkanViewport();
private:
	XVulkanDevice* Device;
	void* WindowHandle;
	VkInstance Instance;
	
	XVulkanSwapChain* VulkanSwapChain;
	std::vector<XVulkanTextureView> ImageViews;
	std::vector<VkImage> swapChainImages;

	VkExtent2D SwapChainExtent;

	friend class VkHack;
};