#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanRHIPrivate.h"
#include "VulkanCommandBuffer.h"


XVulkanRenderTargetLayout::XVulkanRenderTargetLayout(XVulkanDevice& InDevice, const XRHIRenderPassInfo& RPInfo, VkImageLayout CurrentDSLayout)
{
	RenderPassFullHash = 42;
	RenderPassCompatibleHash = 42;
	for (int32 Index = 0; Index < MaxSimultaneousRenderTargets; Index++)
	{
		if (RPInfo.RenderTargets[Index].RenderTarget == nullptr)
		{
			break;
		}

		VkAttachmentDescription CurrDesc;
		CurrDesc.format = VkFormat(GPixelFormats[(int)RPInfo.RenderTargets[Index].RenderTarget->GetFormat()].PlatformFormat);
		XASSERT(CurrDesc.format == VK_FORMAT_R8G8B8A8_SRGB);
		CurrDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		CurrDesc.loadOp = RenderTargetLoadActionToVulkan(RPInfo.RenderTargets[Index].LoadAction);
		CurrDesc.storeOp = RenderTargetStoreActionToVulkan(RPInfo.RenderTargets[Index].StoreAction);
		CurrDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		CurrDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		CurrDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		CurrDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		XASSERT_TEMP(false);
		//colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		THashCombine(RenderPassFullHash, CurrDesc.loadOp);
		THashCombine(RenderPassFullHash, CurrDesc.storeOp);

		AttachCount++;

		THashCombine(RenderPassCompatibleHash, CurrDesc.format);
	}

	VkImageLayout DepthStencilLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if (RPInfo.DepthStencilRenderTarget.DepthStencilTarget)
	{
		XASSERT(false);

		AttachCount++;
	}

	THashCombine(RenderPassFullHash, RenderPassCompatibleHash);
}


void XVulkanCommandListContext::RHIBeginRenderPass(const XRHIRenderPassInfo& InInfo, const char* InName, uint32 Size)
{
	XVulkanCmdBuffer* CmdBuffer = CmdBufferManager->GetActiveCmdBuffer();

	VkImageLayout CurrentDSLayout;
	XRHITexture* DSTexture = InInfo.DepthStencilRenderTarget.DepthStencilTarget;
	if (DSTexture)
	{
		XASSERT(false);
	}
	else
	{
		CurrentDSLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}

	XVulkanRenderTargetLayout RTLayout(*Device, InInfo, CurrentDSLayout);
	XVulkanRenderPass* RenderPass = GlobalLayoutManager.GetOrCreateRenderPass(Device, &RTLayout);
	XRHISetRenderTargetsInfo RTInfo;
	InInfo.ConvertToRenderTargetsInfo(RTInfo);
	XVulkanFramebuffer* Framebuffer = GlobalLayoutManager.GetOrCreateFramebuffer(Device, &RTInfo, &RTLayout, RenderPass);
	GlobalLayoutManager.BeginRenderPass(this, Device, CmdBuffer, &InInfo, &RTLayout, RenderPass, Framebuffer);

}