#include "VulkanResource.h"

XVulkanSurface::XVulkanSurface(XVulkanDevice* InDevice, EPixelFormat InFormat, uint32 InWidth, uint32 InHeight, VkImageViewType	InViewType)
	: Device(InDevice)
	, PixelFormat(InFormat)
	, ViewType(InViewType)
	, Width(InWidth)
	, Height(InHeight)
{
	ViewFormat = VkFormat(GPixelFormats[(int)PixelFormat].PlatformFormat);
}

XVulkanSurface::XVulkanSurface(XVulkanDevice* InDevice, EPixelFormat InFormat, uint32 InWidth, uint32 InHeight, VkImageViewType	InViewType, VkImage InImage)
	: Device(InDevice)
	, PixelFormat(InFormat)
	, ViewType(InViewType)
	, Image(InImage)
	, Width(InWidth)
	, Height(InHeight)
{
	ViewFormat = VkFormat(GPixelFormats[(int)PixelFormat].PlatformFormat);
}

XVulkanTextureBase::XVulkanTextureBase(XVulkanDevice* Device, EPixelFormat Format, uint32 Width, uint32 Height, VkImageViewType	InViewType)
	:Surface(Device,Format, Width, Height, InViewType)
{
	
}

XVulkanTextureBase::XVulkanTextureBase(XVulkanDevice* Device, EPixelFormat Format, uint32 Width, uint32 Height, VkImageViewType InViewType, VkImage InImage)
	:Surface(Device, Format, Width, Height, InViewType, InImage)
{
	
}

XVulkanTexture2D::XVulkanTexture2D(XVulkanDevice* Device, EPixelFormat Format, uint32 Width, uint32 Height, VkImageViewType	InViewType)
	: XRHITexture2D(Format)
	, XVulkanTextureBase(Device, Format, Width, Height, InViewType)
{

}

XVulkanTexture2D::XVulkanTexture2D(XVulkanDevice* Device, EPixelFormat Format, uint32 Width, uint32 Height, VkImageViewType InViewType, VkImage InImage)
	: XRHITexture2D(Format)
	, XVulkanTextureBase(Device, Format, Width, Height, InViewType, InImage)
{
}
