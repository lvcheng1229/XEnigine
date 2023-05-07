#include "VulkanViewport.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanPlatformRHI.h"
#include "VulkanRHIPrivate.h"

XVulkanViewport::XVulkanViewport(EPixelFormat& InOutPixelFormat, XVulkanDevice* VulkanDevice, uint32 InSizeX, uint32 InSizeY, void* InWindowHandle, VkInstance InInstance)
	: Device(VulkanDevice)
	, WindowHandle(InWindowHandle)
	, Instance(InInstance)
	, CurrentBackBuffer(0)
{
	VulkanSwapChain = new XVulkanSwapChain(InOutPixelFormat, VulkanDevice, WindowHandle, Instance, swapChainImages , SwapChainExtent);
	ImageViews.resize(swapChainImages.size());
	
	for (int32 Index = 0; Index < swapChainImages.size(); ++Index)
	{
		XASSERT_TEMP(false);

		ImageViews[Index].Create(Device, swapChainImages[Index], VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_B8G8R8A8_SRGB);
		std::shared_ptr<XVulkanTexture2D> BackTex2D =
			std::make_shared<XVulkanTexture2D>(VulkanDevice, InOutPixelFormat, InSizeX, InSizeY, VK_IMAGE_VIEW_TYPE_2D, swapChainImages[Index]);
		BackBufferTextures.push_back(BackTex2D);
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

void XVulkanViewport::Prsent()
{
	CurrentBackBuffer++;
	CurrentBackBuffer = CurrentBackBuffer % 2;
}

XVulkanFramebuffer::XVulkanFramebuffer(XVulkanDevice* Device, const XRHISetRenderTargetsInfo* InRTInfo, const XVulkanRenderTargetLayout* RTLayout, const XVulkanRenderPass* RenderPass)
	: Framebuffer(VK_NULL_HANDLE)
	, NumColorRenderTargets(InRTInfo->NumColorRenderTargets)
	, NumColorAttachments(0)
	, DepthStencilRenderTargetImage(VK_NULL_HANDLE)
{
	AttachmentViews.clear();
	VulkanTextureViews.clear();
	for (int32 Index = 0; Index < InRTInfo->NumColorRenderTargets; ++Index)
	{
		XRHITexture* RHITexture = InRTInfo->ColorRenderTarget[Index].Texture;
		if (!RHITexture)
		{
			continue;
		}

		XVulkanTextureBase* Texture = GetVulkanTextureFromRHITexture(RHITexture);
		XVulkanTextureView VulkanTextureView;
		VulkanTextureView.Create(Device, Texture->Surface.Image, Texture->Surface.GetViewType(), Texture->Surface.ViewFormat);
		VulkanTextureViews.push_back(VulkanTextureView);
		AttachmentViews.push_back(VulkanTextureView.View);
	}
	const VkExtent2D RTExtents = RTLayout->GetExtent2D();

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = RenderPass->GetRenderPass();
	framebufferInfo.attachmentCount = AttachmentViews.size();
	framebufferInfo.pAttachments = AttachmentViews.data();
	framebufferInfo.width = RTExtents.width;
	framebufferInfo.height = RTExtents.height;
	framebufferInfo.layers = 1;

	RenderArea.offset = VkOffset2D{ 0,0 };
	RenderArea.extent = VkExtent2D{ RTExtents.width, RTExtents.height };
	VULKAN_VARIFY(vkCreateFramebuffer(Device->GetVkDevice(), &framebufferInfo, nullptr, &Framebuffer));
}
