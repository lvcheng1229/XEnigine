#include "VulkanDescriptorSets.h"
#include "VulkanResource.h"

void XVulkanDescriptorSetsLayoutInfo::FinalizeBindings_Gfx(XVulkanShader* VertexShader, XVulkanShader* PixelShader)
{
    if (VertexShader->ResourceCount.NumCBV)
    {
        for (int32 Index = 0; Index < VertexShader->ResourceCount.NumCBV; Index++)
        {
            RemappingInfo.SetInfo.Types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            RemappingInfo.SetInfo.NumBufferInfos++;
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = RemappingInfo.SetInfo.Types.size() - 1;
            uboLayoutBinding.descriptorCount = 1;//?
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            SetLayout.LayoutBindings.push_back(uboLayoutBinding);

            LayoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER]++;
        }
    }

    if (PixelShader->ResourceCount.NumCBV)
    {
        for (int32 Index = 0; Index < PixelShader->ResourceCount.NumCBV; Index++)
        {
            RemappingInfo.SetInfo.Types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            RemappingInfo.SetInfo.NumBufferInfos++;
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = RemappingInfo.SetInfo.Types.size() - 1;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            SetLayout.LayoutBindings.push_back(uboLayoutBinding);

            LayoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER]++;
        }
    }

    //COMBINED IMAGE AND SAMPLER!!

    if (PixelShader->ResourceCount.NumSRV)
    {
        for (int32 Index = 0; Index < PixelShader->ResourceCount.NumSRV; Index++)
        {
            RemappingInfo.SetInfo.Types.push_back(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
            RemappingInfo.SetInfo.NumImageInfos++;
            VkDescriptorSetLayoutBinding textureLayoutBinding{};
            textureLayoutBinding.binding = RemappingInfo.SetInfo.Types.size() - 1;
            textureLayoutBinding.descriptorCount = 1;
            textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            textureLayoutBinding.pImmutableSamplers = nullptr;
            textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            SetLayout.LayoutBindings.push_back(textureLayoutBinding);

            LayoutTypes[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE]++;
        }
    }

    if (PixelShader->ResourceCount.NumSampler)
    {
        for (int32 Index = 0; Index < PixelShader->ResourceCount.NumSampler; Index++)
        {
            RemappingInfo.SetInfo.Types.push_back(VK_DESCRIPTOR_TYPE_SAMPLER);
            RemappingInfo.SetInfo.NumImageInfos++;
            VkDescriptorSetLayoutBinding samplerBinding{};
            samplerBinding.binding = RemappingInfo.SetInfo.Types.size() - 1;
            samplerBinding.descriptorCount = 1;
            samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            samplerBinding.pImmutableSamplers = nullptr;
            samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            SetLayout.LayoutBindings.push_back(samplerBinding);

            LayoutTypes[VK_DESCRIPTOR_TYPE_SAMPLER]++;
        }
    }

    SetLayout.GenerateHash();
    Hash = SetLayout.Hash;
}

XVulkanDescriptorPoolSetContainer& XVulkanDescriptorPoolsManager::AcquirePoolSetContainer()
{
    for (int32 Index = 0; Index < PoolSets.size(); ++Index)
    {
        auto* PoolSet = PoolSets[Index];
        if (PoolSet->IsUnused())
        {
            PoolSet->SetUsed(true);
            return *PoolSet;
        }
    }

    XVulkanDescriptorPoolSetContainer* PoolSet = new XVulkanDescriptorPoolSetContainer(Device);
    PoolSets.push_back(PoolSet);
    return *PoolSet;
}

void XVulkanGfxPipelineDescriptorInfo::Initialize(const XDescriptorSetRemappingInfo& InRemappingInfo)
{
    DescriptorSetRemappingInfo = InRemappingInfo;
}

bool XVulkanGfxPipelineDescriptorInfo::GetDescriptorSetAndBindingIndex(const EShaderType Stage, int32 ParameterIndex, uint8& OutDescriptorSet, uint32& OutBindingIndex) const
{
    return false;
}

XDescriptorSetRemappingInfo::~XDescriptorSetRemappingInfo()
{
    int32 ForDebug;
}
