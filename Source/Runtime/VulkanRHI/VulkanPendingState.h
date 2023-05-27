#pragma once
#include "VulkanPipeline.h"

class XVulkanPendingGfxState
{
public:
	void SetGfxPipeline(XVulkanRHIGraphicsPipelineState* InGfxPipeline);
	inline void Bind(VkCommandBuffer CmdBuffer)
	{
		CurrentPipeline->Bind(CmdBuffer);
	}
private:
	XVulkanRHIGraphicsPipelineState* CurrentPipeline;
};