#include "VulkanPlatformRHI.h"
#include "VulkanDevice.h"

std::shared_ptr<XRHIGraphicsPSO> XVulkanPlatformRHI::RHICreateGraphicsPipelineState(const XGraphicsPSOInitializer& PSOInit)
{
	//VkGraphicsPipelineCreateInfo pipelineInfo{};
	//
	//CreateGfxEntry(Initializer, DescriptorSetLayoutInfo, &Desc);
	//
	//vkCreateGraphicsPipelines(Device->GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline)
	//VULKAN_VARIFY(vkCreateGraphicsPipelines(Device->GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline));
	return std::shared_ptr<XRHIGraphicsPSO>();
}