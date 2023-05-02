#pragma once
#include "vulkan\vulkan_core.h"
#include <vector>
class XVulkanDevice;
class XVulkanSwapChain
{
public:
	XVulkanSwapChain(EPixelFormat& InOutPixelFormat ,XVulkanDevice* VulkanDevice, void* InWindowHandle, VkInstance InInstance , std::vector<VkImage>& OutImages , VkExtent2D& SwapChainExtent);
	~XVulkanSwapChain();
private:
	friend class  VkHack;

	XVulkanDevice* Device;
	void* WindowHandle; 
	VkInstance Instance;
	VkSurfaceKHR Surface;

	VkSwapchainKHR swapChain;

	
};