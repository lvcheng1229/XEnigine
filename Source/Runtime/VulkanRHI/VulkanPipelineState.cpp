#include "VulkanContext.h"
#include "VulkanPipeline.h"
#include "VulkanPendingState.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"

void XVulkanCommandListContext::RHISetGraphicsPipelineState(XRHIGraphicsPSO* GraphicsState)
{
	PendingGfxState->SetGfxPipeline(static_cast<XVulkanRHIGraphicsPipelineState*>(GraphicsState));
	PendingGfxState->Bind(CmdBufferManager->GetActiveCmdBuffer()->GetHandle());
}

void XVulkanCommonPipelineDescriptorState::CreateDescriptorWriteInfos()
{
    const XVulkanSamplerState& DefaultSampler = Device->GetDefaultSampler();
    const XVulkanTextureView& DefaultImageView = Device->GetDefaultImageView();

    const auto& SetInfo = DescriptorSetsLayout->RemappingInfo.SetInfo;
    DSWriteContainer.DescriptorWrites.emplace_back(SetInfo.Types.size());
    DSWriteContainer.DescriptorImageInfo.emplace_back(SetInfo.NumImageInfos);
    DSWriteContainer.DescriptorBufferInfo.emplace_back(SetInfo.NumBufferInfos);
    DSWriter.SetupDescriptorWrites(SetInfo.Types,
        DSWriteContainer.DescriptorWrites.data(), 
        DSWriteContainer.DescriptorImageInfo.data(), 
        DSWriteContainer.DescriptorBufferInfo.data(), 
        DefaultSampler, DefaultImageView);
}

XVulkanGraphicsPipelineDescriptorState::XVulkanGraphicsPipelineDescriptorState(XVulkanDevice* InDevice, XVulkanRHIGraphicsPipelineState* InGfxPipeline)
    :
    XVulkanCommonPipelineDescriptorState(InDevice),
    GfxPipeline(InGfxPipeline)
{
    DescriptorSetsLayout = InGfxPipeline->Layout->GetDescriptorSetsLayout();
    CreateDescriptorWriteInfos();
}


