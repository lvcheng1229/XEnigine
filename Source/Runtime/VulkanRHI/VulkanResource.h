#include <vulkan\vulkan_core.h>
#include <Runtime\HAL\PlatformTypes.h>
#include "Runtime\RHI\RHIResource.h"

class XVulkanDevice;
struct XVulkanTextureView
{
	XVulkanTextureView()
		: View(VK_NULL_HANDLE)
		, Image(VK_NULL_HANDLE)
		, ViewId(0)
	{
	}

	void Create(XVulkanDevice* Device, VkImage InImage, VkImageViewType ViewType, VkFormat Format);
	VkImageView View;
	VkImage Image;
	uint32 ViewId;
private:
	void CreateImpl(XVulkanDevice* Device, VkImage InImage, VkImageViewType ViewType, VkFormat Format);
};

class XVulkanTextureBase
{
public:
};

class XVulkanTexture2D : public XRHITexture2D, public XVulkanTextureBase
{
public:
};