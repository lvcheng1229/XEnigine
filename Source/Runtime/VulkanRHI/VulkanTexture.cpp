#include "VulkanResource.h"
#include "VulkanPlatformRHI.h"
#include "VulkanDevice.h"

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

XVulkanSurface::XVulkanSurface(XVulkanEvictable* Owner, XVulkanDevice* InDevice, EPixelFormat InFormat, uint32 Width, uint32 Height, VkImageViewType InViewType, ETextureCreateFlags Flag, uint32 NumMipsIn, uint8* TexData)
	: Device(InDevice)
	, PixelFormat(InFormat)
	, ViewType(InViewType)
	, Width(Width)
	, Height(Height)
{
	ViewFormat = VkFormat(GPixelFormats[(int)PixelFormat].PlatformFormat);
	VkImageType ImageType;
	switch (InViewType)
	{
	case VkImageViewType::VK_IMAGE_VIEW_TYPE_2D:
		ImageType = VkImageType::VK_IMAGE_TYPE_2D;
		break;
	default:
		XASSERT(false);
	};

	VkImageCreateInfo ImageCreateInfo{};
	ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageCreateInfo.imageType = ImageType;
	ImageCreateInfo.extent.width = Width;
	ImageCreateInfo.extent.height = Height;
	ImageCreateInfo.extent.depth = 1;
	ImageCreateInfo.mipLevels = NumMipsIn;
	ImageCreateInfo.arrayLayers = 1;
	ImageCreateInfo.format = ViewFormat;
	ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	ImageCreateInfo.usage = 0;
	ImageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	ImageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	ImageCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	
	if (((Flag & ETextureCreateFlags::TexCreate_RenderTargetable) != 0))
	{
		ImageCreateInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	if (((Flag & ETextureCreateFlags::TexCreate_DepthStencilTargetable) != 0))
	{
		ImageCreateInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	if (((Flag & ETextureCreateFlags::TexCreate_UAV) != 0))
	{
		ImageCreateInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
	}
	
	ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VULKAN_VARIFY(vkCreateImage(Device->GetVkDevice(), &ImageCreateInfo, nullptr, &Image));

	vkGetImageMemoryRequirements(Device->GetVkDevice(), Image, &MemoryRequirements);

	const bool bRenderTarget =
		((Flag & ETextureCreateFlags::TexCreate_RenderTargetable) != 0)
		|| ((Flag & ETextureCreateFlags::TexCreate_DepthStencilTargetable) != 0);
		
	const bool bUAV = ((Flag & ETextureCreateFlags::TexCreate_UAV) != 0);
	
	VkMemoryPropertyFlags MemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	EVulkanAllocationMetaType MetaType = (bRenderTarget || bUAV) ? EVulkanAllocationMetaImageRenderTarget : EVulkanAllocationMetaImageOther;
	Device->GetMemoryManager().AllocateImageMemory(Allocation, Owner, MemoryRequirements, MemoryFlags, MetaType);
}

XVulkanTextureBase::XVulkanTextureBase(XVulkanDevice* Device, EPixelFormat Format, uint32 Width, uint32 Height, VkImageViewType	InViewType)
	:Surface(Device,Format, Width, Height, InViewType)
{
	
}

XVulkanTextureBase::XVulkanTextureBase(XVulkanDevice* Device, EPixelFormat Format, uint32 Width, uint32 Height, VkImageViewType InViewType, VkImage InImage)
	:Surface(Device, Format, Width, Height, InViewType, InImage)
{
	
}

XVulkanTextureBase::XVulkanTextureBase(XVulkanDevice* Device, EPixelFormat Format, uint32 Width, uint32 Height, VkImageViewType InViewType, ETextureCreateFlags Flag, uint32 NumMipsIn, uint8* TexData)
	:Surface(Device, Format, Width, Height, VK_IMAGE_VIEW_TYPE_2D, Flag, NumMipsIn, TexData)
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

XVulkanTexture2D::XVulkanTexture2D(XVulkanDevice* Device, EPixelFormat Format, uint32 Width, uint32 Height, ETextureCreateFlags Flag, uint32 NumMipsIn, uint8* TexData)
	: XRHITexture2D(Format)
	, XVulkanTextureBase(Device, Format, Width, Height, VK_IMAGE_VIEW_TYPE_2D, Flag, NumMipsIn, TexData)
{
}

std::shared_ptr<XRHITexture2D> XVulkanPlatformRHI::RHICreateTexture2D(uint32 width, uint32 height, uint32 SizeZ, bool bTextureArray, bool bCubeTexture, EPixelFormat Format, ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data)
{
	return std::make_shared<XVulkanTexture2D>();
}
