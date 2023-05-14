#include "VulkanResource.h"
#include "VulkanDevice.h"

void XVulkanTextureView::CreateImpl(XVulkanDevice* Device,VkImage InImage , VkImageViewType ViewType , VkFormat Format)
{
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = InImage;

	createInfo.viewType = ViewType;
	createInfo.format = Format;

	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;
	VULKAN_VARIFY(vkCreateImageView(Device->GetVkDevice(), &createInfo, nullptr, &View));
}

void XVulkanTextureView::Create(XVulkanDevice* Device, VkImage InImage, VkImageViewType ViewType, VkFormat Format)
{
	CreateImpl(Device, InImage, ViewType, Format);
	Image = InImage;
}


