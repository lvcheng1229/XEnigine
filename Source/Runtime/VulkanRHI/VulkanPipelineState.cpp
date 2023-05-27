#include "VulkanContext.h"
#include "VulkanPipeline.h"
#include "VulkanPendingState.h"
#include "VulkanCommandBuffer.h"

void XVulkanCommandListContext::RHISetGraphicsPipelineState(XRHIGraphicsPSO* GraphicsState)
{
	PendingGfxState->SetGfxPipeline(static_cast<XVulkanRHIGraphicsPipelineState*>(GraphicsState));
	PendingGfxState->Bind(CmdBufferManager->GetActiveCmdBuffer()->GetHandle());
}