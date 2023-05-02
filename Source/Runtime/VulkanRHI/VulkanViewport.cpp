#include "VulkanViewport.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanPlatformRHI.h"


XVulkanViewport::XVulkanViewport(EPixelFormat& InOutPixelFormat, XVulkanDevice* VulkanDevice, void* InWindowHandle, VkInstance InInstance)
	: Device(VulkanDevice)
	, WindowHandle(InWindowHandle)
	, Instance(InInstance)
{
	VulkanSwapChain = new XVulkanSwapChain(InOutPixelFormat, VulkanDevice, WindowHandle, Instance, swapChainImages , SwapChainExtent);
	ImageViews.resize(swapChainImages.size());
	for (int32 Index = 0; Index < swapChainImages.size(); ++Index)
	{
		ImageViews[Index].Create(Device, swapChainImages[Index], VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_B8G8R8A8_SRGB);
	}
}
XVulkanViewport::~XVulkanViewport()
{
	for (auto imageView : ImageViews) 
	{
		vkDestroyImageView(Device->GetVkDevice(), imageView.View, nullptr);
	}
	delete VulkanSwapChain;
}

//XVulkanBackBuffer::XVulkanBackBuffer()
//{
//}
