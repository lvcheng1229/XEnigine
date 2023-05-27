#include "VulkanPendingState.h"

void XVulkanPendingGfxState::SetGfxPipeline(XVulkanRHIGraphicsPipelineState* InGfxPipeline)
{
    if (InGfxPipeline != CurrentPipeline)
    {
        CurrentPipeline = InGfxPipeline;
    }
}
