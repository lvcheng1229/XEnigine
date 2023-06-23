#include "VulkanPendingState.h"
#include "VulkanRHIPrivate.h"
#include "VulkanDevice.h"

XVulkanDescriptorPool::XVulkanDescriptorPool(XVulkanDevice* InDevice, const XVulkanDescriptorSetsLayout* InLayout, uint32 MaxSetsAllocations)
    : Device(InDevice)
    , Layout(InLayout)
    , DescriptorPool(VK_NULL_HANDLE)
{
    MaxDescriptorSets = MaxSetsAllocations;

    VkDescriptorPoolSize Types[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT - VK_DESCRIPTOR_TYPE_SAMPLER + 1];
    for (uint32 TypeIndex = VK_DESCRIPTOR_TYPE_SAMPLER; TypeIndex <= VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; ++TypeIndex)
    {
        VkDescriptorType DescriptorType = (VkDescriptorType)TypeIndex;
        uint32 NumTypesUsed = Layout->GetTypesUsed(DescriptorType);
        if (NumTypesUsed > 0)
        {
            VkDescriptorPoolSize* Type = &Types[TypeIndex];
            memset(Type, 0, sizeof(VkDescriptorPoolSize));
            Type->type = DescriptorType;
            Type->descriptorCount = NumTypesUsed * MaxSetsAllocations;
        }
    }


    VkDescriptorPoolCreateInfo PoolInfo = {};
    PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    PoolInfo.poolSizeCount = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT - VK_DESCRIPTOR_TYPE_SAMPLER + 1;
    PoolInfo.pPoolSizes = Types;
    PoolInfo.maxSets = MaxDescriptorSets;

    VULKAN_VARIFY(vkCreateDescriptorPool(Device->GetVkDevice(), &PoolInfo, nullptr, &DescriptorPool));
}

bool XVulkanDescriptorPool::AllocateDescriptorSets(const VkDescriptorSetAllocateInfo& InDescriptorSetAllocateInfo, VkDescriptorSet* OutSets)
{
    VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = InDescriptorSetAllocateInfo;
    DescriptorSetAllocateInfo.descriptorPool = DescriptorPool;

    return VK_SUCCESS == vkAllocateDescriptorSets(Device->GetVkDevice(), &DescriptorSetAllocateInfo, OutSets);
}

XVulkanDescriptorPool* XVulkanTypedDescriptorPoolSet::PushNewPool()
{
    // Max number of descriptor sets layout allocations
    const uint32 MaxSetsAllocationsBase = 32;
    // Allow max 128 setS per pool (32 << 2)
    const uint32 MaxSetsAllocations = MaxSetsAllocationsBase << 2u;

    XVulkanDescriptorPool* NewPool = new XVulkanDescriptorPool(Device, Layout, MaxSetsAllocations);

    PoolArray.push_back(NewPool);

    return NewPool;

}

XVulkanTypedDescriptorPoolSet::~XVulkanTypedDescriptorPoolSet()
{
    for (auto iter : PoolArray)
    {
        delete iter;
    }
}

bool XVulkanTypedDescriptorPoolSet::AllocateDescriptorSets(const XVulkanDescriptorSetsLayout* Layout, VkDescriptorSet* OutSets)
{
    const VkDescriptorSetLayout LayoutHandle = Layout->LayoutHandle;

    for (int32 Index = 0; Index < (std::min)(PoolArray.size(), std::size_t(32)); Index++)
    {
        if (PoolArray[Index]->AllocateDescriptorSets(Layout->GetAllocateInfo(), OutSets) == false)
        {
            PushNewPool();
        }
        else
        {
            return true;
        }
    }

    return true;

}


XVulkanDescriptorPoolSetContainer::~XVulkanDescriptorPoolSetContainer()
{
    for (auto iter : TypedDescriptorPools)
    {
        if (iter.second)
        {
            delete iter.second;
        }
    }
}

XVulkanTypedDescriptorPoolSet* XVulkanDescriptorPoolSetContainer::AcquireTypedPoolSet(const XVulkanDescriptorSetsLayout* Layout)
{
    const uint32 Hash = Layout->GetHash();

    auto iter = TypedDescriptorPools.find(Hash);
    if (iter == TypedDescriptorPools.end())
    {
        XVulkanTypedDescriptorPoolSet* TypedPool = new XVulkanTypedDescriptorPoolSet(Device, Layout);
        TypedDescriptorPools[Hash] = TypedPool;
        return TypedPool;
    }

    return iter->second;
}

void XVulkanPendingGfxState::SetGfxPipeline(XVulkanRHIGraphicsPipelineState* InGfxPipeline)
{
    bool bChanged = false;
    if (InGfxPipeline != CurrentPipeline)
    {
        CurrentPipeline = InGfxPipeline;
        auto iter = PipelineStates.find(InGfxPipeline);
        if (iter != PipelineStates.end())
        {
            CurrentState = iter->second;
        }
        else
        {
            CurrentState = new XVulkanGraphicsPipelineDescriptorState(Device, InGfxPipeline);
            PipelineStates[CurrentPipeline] = CurrentState;
        }
        bChanged = true;
    }

    if (bChanged)
    {
        //CurrentState->Reset();
    }
}

void XVulkanPendingGfxState::PrepareForDraw(XVulkanCmdBuffer* CmdBuffer)
{
    UpdateDynamicStates(CmdBuffer);

    //TODO:BindMultiVertexBuffer
    VkDeviceSize Offset = PendingStreams[0].BufferOffset;
    vkCmdBindVertexBuffers(CmdBuffer->GetHandle(), 0, 1, &PendingStreams[0].Stream, &Offset);
}



bool XVulkanGraphicsPipelineDescriptorState::UpdateDescriptorSets(XVulkanCommandListContext* CmdListContext, XVulkanCmdBuffer* CmdBuffer)
{
    if (!CmdBuffer->AcquirePoolSetAndDescriptorsIfNeeded(DescriptorSetsLayout, true, &DescriptorSetHandle))
    {
        return false;
    }
}

void XVulkanGraphicsPipelineDescriptorState::SetTexture(uint8 DescriptorSet, uint32 BindingIndex, const XVulkanTextureBase* TextureBase, VkImageLayout Layout)
{
    // If the texture doesn't support sampling, then we read it through a UAV
    MarkDirty(DSWriter[DescriptorSet].WriteImage(BindingIndex, TextureBase->DefaultView, Layout));
}