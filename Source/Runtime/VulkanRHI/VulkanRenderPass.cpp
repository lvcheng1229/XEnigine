#include "VulkanRHIPrivate.h"
#include "VulkanDevice.h"
//VkRenderPass& CreateVulkanRenderPass(XVulkanDevice* InDevice, const XVulkanRenderTargetLayout* RTLayout)
//{
//	XASSERT_TEMP(false);//Sub Pass
//    
//    VkAttachmentReference colorAttachmentRef{};
//    colorAttachmentRef.attachment = 0;
//    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//    VkSubpassDescription subpass{};
//    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//    subpass.colorAttachmentCount = RTLayout->AttachCount;
//    XASSERT(RTLayout->AttachCount <= 1);
//    subpass.pColorAttachments = &colorAttachmentRef;
//
//    VkRenderPassCreateInfo renderPassInfo{};
//    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//    renderPassInfo.attachmentCount = RTLayout->AttachCount;
//    renderPassInfo.pAttachments = RTLayout->GetAttachment();
//    renderPassInfo.subpassCount = 1;
//    renderPassInfo.pSubpasses = &subpass;
//
//    VkRenderPass RetRP;
//    VULKAN_VARIFY(vkCreateRenderPass(InDevice->GetVkDevice(), &renderPassInfo, nullptr, &RetRP));
//    return RetRP;
//}
XVulkanRenderPass::XVulkanRenderPass(XVulkanDevice* InDevice, const XVulkanRenderTargetLayout* RTLayout)
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

    VULKAN_VARIFY(vkCreateRenderPass(InDevice->GetVkDevice(), &renderPassInfo, nullptr, &RenderPass));
}
//vkQueuePresentKHR(queue, pPresentInfo) 
//returns VkResultvalidation layer : Validation Error : 
//[VUID - VkPresentInfoKHR - pImageIndices - 01296] 
//Object 0 : handle = 0x1c422d0dc80, type = VK_OBJECT_TYPE_QUEUE; | MessageID = 0xc7aabc16 | vkQueuePresentKHR() : 
//    pSwapchains[0] images passed to present must be in layout VK_IMAGE_LAYOUT_PRESENT_SRC_KHR or VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR 
//    but is in VK_IMAGE_LAYOUT_UNDEFINED.The Vulkan spec states : 
//Each element of pImageIndices must be the index of a presentable image acquired from the swapchain 
//specified by the corresponding element of the pSwapchains array, 
//and the presented image subresource must be in the VK_IMAGE_LAYOUT_PRESENT_SRC_KHR layout at the time the operation is executed on a VkDevice(
//    https ://github.com/KhronosGroup/Vulkan-Docs/search?q=)VUID-VkPresentInfoKHR-pImageIndices-01296)