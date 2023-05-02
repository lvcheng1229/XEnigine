#include "VulkanRHIPrivate.h"
#include "VulkanDevice.h"
VkRenderPass CreateVulkanRenderPass(XVulkanDevice* InDevice, const XVulkanRenderTargetLayout* RTLayout)
{
	XASSERT_TEMP(false);//Sub Pass
    
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = RTLayout->AttachCount;
    XASSERT(RTLayout->AttachCount <= 1);
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = RTLayout->AttachCount;
    renderPassInfo.pAttachments = RTLayout->GetAttachment();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkRenderPass RetRP;
    VULKAN_VARIFY(vkCreateRenderPass(InDevice->GetVkDevice(), &renderPassInfo, nullptr, &RetRP));
    return RetRP;
}
XVulkanRenderPass::XVulkanRenderPass(XVulkanDevice* InDevice, const XVulkanRenderTargetLayout* RTLayout)
{
	RenderPass = CreateVulkanRenderPass(InDevice, RTLayout);
}