#include "VulkanDescriptorSets.h"
#include "VulkanResource.h"

void XVulkanDescriptorSetsLayoutInfo::FinalizeBindings_Gfx(XVulkanShader* VertexShader, XVulkanShader* PixelShader)
{
    if (VertexShader->ResourceCount.NumCBV)
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = VertexShader->ResourceCount.NumCBV;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        SetLayout.LayoutBindings.push_back(uboLayoutBinding);
    }

    if (PixelShader->ResourceCount.NumCBV)
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = PixelShader->ResourceCount.NumCBV;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        SetLayout.LayoutBindings.push_back(uboLayoutBinding);
    }

    SetLayout.GenerateHash();
    Hash = SetLayout.Hash;
}

XVulkanDescriptorPoolSetContainer& XVulkanDescriptorPoolsManager::AcquirePoolSetContainer()
{
    // TODO: 在此处插入 return 语句
    XVulkanDescriptorPoolSetContainer aa;
    return aa;
}
