#pragma once
#include "Runtime\RHI\RHIResource.h"
#include "VulkanContext.h"

class XVulkanRenderPass
{
public:
	XVulkanRenderPass(XVulkanDevice* InDevice, const XVulkanRenderTargetLayout* RTLayout);
	const VkRenderPass GetRenderPass()const { return RenderPass; }
private:
	VkRenderPass	RenderPass;
};

static inline VkAttachmentLoadOp RenderTargetLoadActionToVulkan(ERenderTargetLoadAction InLoadAction)
{
	VkAttachmentLoadOp OutLoadAction = VK_ATTACHMENT_LOAD_OP_MAX_ENUM;

	switch (InLoadAction)
	{
	case ERenderTargetLoadAction::ELoad:		OutLoadAction = VK_ATTACHMENT_LOAD_OP_LOAD;			break;
	case ERenderTargetLoadAction::EClear:		OutLoadAction = VK_ATTACHMENT_LOAD_OP_CLEAR;		break;
	case ERenderTargetLoadAction::ENoAction:	OutLoadAction = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	break;
	default:	
		XASSERT(false); break;
	}
	
	return OutLoadAction;
}

static inline VkAttachmentStoreOp RenderTargetStoreActionToVulkan(ERenderTargetStoreAction InStoreAction)
{
	VkAttachmentStoreOp OutStoreAction = VK_ATTACHMENT_STORE_OP_MAX_ENUM;

	switch (InStoreAction)
	{
	case ERenderTargetStoreAction::EStore:
		OutStoreAction = VK_ATTACHMENT_STORE_OP_STORE;
		break;
	case ERenderTargetStoreAction::ENoAction:
		OutStoreAction = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		break;
	default:
		XASSERT(false); break;
	}
	
	return OutStoreAction;
}

class XVulkanRenderTargetLayout
{
public:
	XVulkanRenderTargetLayout(const XGraphicsPSOInitializer& Initializer);
	XVulkanRenderTargetLayout(XVulkanDevice& InDevice, const XRHIRenderPassInfo& RPInfo, VkImageLayout CurrentDSLayout);
	const uint32 GetRenderPassFullHash()const { return RenderPassFullHash; }
	const uint32 GetRenderPassCompatibleHash()const { return RenderPassCompatibleHash; }
	const VkAttachmentDescription* GetAttachment()const { return Desc; }
	const VkExtent2D GetExtent2D() const { return Extent2D; }
	uint32 AttachCount = 0;
private:
	std::size_t RenderPassFullHash = 0;
	std::size_t RenderPassCompatibleHash = 0;
	VkAttachmentDescription Desc[MaxSimultaneousRenderTargets + 1];

	VkExtent2D Extent2D;
};