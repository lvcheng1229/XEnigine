#include "VulkanPendingState.h"

void XVulkanPendingGfxState::SetGfxPipeline(XVulkanRHIGraphicsPipelineState* InGfxPipeline)
{
    if (InGfxPipeline != CurrentPipeline)
    {
        CurrentPipeline = InGfxPipeline;
    }
}

void XVulkanPendingGfxState::PrepareForDraw(XVulkanCmdBuffer* CmdBuffer)
{
    UpdateDynamicStates(CmdBuffer);

    //TODO:BindMultiVertexBuffer
    VkDeviceSize Offset = PendingStreams[0].BufferOffset;
    vkCmdBindVertexBuffers(CmdBuffer->GetHandle(), 0, 1, &PendingStreams[0].Stream, &Offset);
}

