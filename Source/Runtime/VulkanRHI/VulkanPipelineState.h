#pragma once
#include "VulkanPipeline.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDescriptorSets.h"
#include "VulkanResource.h"

class XVulkanCommonPipelineDescriptorState
{
public:
	XVulkanCommonPipelineDescriptorState(XVulkanDevice* InDevice)
		:Device(InDevice)
	{

	}

	void CreateDescriptorWriteInfos();
protected:

	inline void Bind(VkCommandBuffer CmdBuffer, VkPipelineLayout PipelineLayout, VkPipelineBindPoint BindPoint)
	{
		vkCmdBindDescriptorSets(CmdBuffer, BindPoint, PipelineLayout, 0,
			1, &DescriptorSetHandle, (uint32)DynamicOffsets.size(), DynamicOffsets.data());
	}

	XVulkanDescriptorSetWriter DSWriter;
	XVulkanDescriptorSetWriteContainer DSWriteContainer;

	const XVulkanDescriptorSetsLayout* DescriptorSetsLayout = nullptr;
	
	VkDescriptorSet DescriptorSetHandle;
	std::vector<uint32> DynamicOffsets;

	XVulkanDevice* Device;
};

class XVulkanGraphicsPipelineDescriptorState : XVulkanCommonPipelineDescriptorState
{
public:
	XVulkanGraphicsPipelineDescriptorState(XVulkanDevice* InDevice, XVulkanRHIGraphicsPipelineState* InGfxPipeline);

	bool UpdateDescriptorSets(XVulkanCommandListContext* CmdListContext, XVulkanCmdBuffer* CmdBuffer);

	inline const XVulkanGfxPipelineDescriptorInfo& GetGfxPipelineDescriptorInfo() const
	{
		return *PipelineDescriptorInfo;
	}

	void SetTexture(uint8 DescriptorSet, uint32 BindingIndex, const XVulkanTextureBase* TextureBase, VkImageLayout Layout);
	
	inline void MarkDirty(bool bDirty)
	{
		bIsResourcesDirty |= bDirty;
		bIsDSetsKeyDirty |= bDirty;
	}
private:
	std::vector<XVulkanDescriptorSetWriter> DSWriter;

	const XVulkanGfxPipelineDescriptorInfo* PipelineDescriptorInfo;

	XVulkanRHIGraphicsPipelineState* GfxPipeline;

	bool bIsResourcesDirty = true;
	mutable bool bIsDSetsKeyDirty = true;
};