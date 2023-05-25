#include "VulkanPipeline.h"
#include "VulkanPlatformRHI.h"
#include "VulkanDevice.h"
#include "VulkanState.h"
#include "VulkanRHIPrivate.h"

XVulkanPipelineStateCacheManager::XVulkanPipelineStateCacheManager(XVulkanDevice* InDevice)
    :Device(InDevice)
{
}

std::size_t XGfxPipelineDesc::CreateKey()
{
    return std::size_t();
}

void GetVulkanShaders(const XRHIBoundShaderStateInput& BSI, XVulkanShader* OutShaders[(uint32)EShaderType::SV_ShaderCount])
{
    OutShaders[(uint32)EShaderType::SV_Vertex] = static_cast<XVulkanVertexShader*>(BSI.RHIVertexShader);
    OutShaders[(uint32)EShaderType::SV_Pixel] = static_cast<XVulkanPixelShader*>(BSI.RHIPixelShader); 
}

std::shared_ptr<XRHIGraphicsPSO> XVulkanPipelineStateCacheManager::RHICreateGraphicsPipelineState(const XGraphicsPSOInitializer& PSOInit)
{
    std::size_t Key;
    XGfxPipelineDesc Desc;
    XVulkanDescriptorSetsLayoutInfo DescriptorSetLayoutInfo;//UnUsed For Now

    CreateGfxEntry(PSOInit, DescriptorSetLayoutInfo, &Desc);
    Key = Desc.CreateKey();

    XVulkanRHIGraphicsPipelineState* NewPSO = nullptr;
    auto iter = GraphicsPSOMap.find(Key);
    if (iter != GraphicsPSOMap.end())
    {
        return  std::shared_ptr<XVulkanRHIGraphicsPipelineState>(iter->second);
    }

    NewPSO = new XVulkanRHIGraphicsPipelineState(Device, PSOInit, Desc, Key);
    NewPSO->RenderPass = Device->GetGfxContex()->PrepareRenderPassForPSOCreation(PSOInit);

    XVulkanShader* VulkanShaders[(uint32)EShaderType::SV_ShaderCount];
    memset(VulkanShaders, 0, sizeof(XVulkanShader*) * (uint32)EShaderType::SV_ShaderCount);
    GetVulkanShaders(PSOInit.BoundShaderState, VulkanShaders);
    CreateGfxPipelineFromEntry(NewPSO, VulkanShaders, &NewPSO->VulkanPipeline);

    GraphicsPSOMap[Key] = NewPSO;
    return std::shared_ptr<XRHIGraphicsPSO>(NewPSO);
}

void XVulkanPipelineStateCacheManager::CreateGfxEntry(const XGraphicsPSOInitializer& PSOInitializer, XVulkanDescriptorSetsLayoutInfo& DescriptorSetLayoutInfo, XGfxPipelineDesc* Desc)
{
    XGfxPipelineDesc* OutGfxEntry = Desc;
}

void XVulkanPipelineStateCacheManager::CreateGfxPipelineFromEntry(XVulkanRHIGraphicsPipelineState* PSO, XVulkanShader* Shaders[(uint32)EShaderType::SV_ShaderCount], VkPipeline* Pipeline)
{
    XGfxPipelineDesc* GfxEntry = &PSO->Desc;
    PSO->GetOrCreateShaderModules(Shaders);

    VkGraphicsPipelineCreateInfo PipelineInfo;
    VULKAN_VARIFY(vkCreateGraphicsPipelines(Device->GetVkDevice(), PipelineCache, 1, &PipelineInfo, nullptr, Pipeline));
}

std::shared_ptr<XRHIGraphicsPSO> XVulkanPlatformRHI::RHICreateGraphicsPipelineState(const XGraphicsPSOInitializer& PSOInit)
{
    return Device->PipelineStateCache->RHICreateGraphicsPipelineState(PSOInit);
}

XVulkanRHIGraphicsPipelineState::XVulkanRHIGraphicsPipelineState(XVulkanDevice* Device, const XGraphicsPSOInitializer& PSOInitializer, XGfxPipelineDesc& Desc, std::size_t Key)
{

}

void XVulkanRHIGraphicsPipelineState::GetOrCreateShaderModules(XVulkanShader* const* Shaders)
{
    for (int32 Index = 0; Index < (int32)EShaderType::SV_ShaderCount; ++Index)
    {
        XVulkanShader* Shader = Shaders[Index];
        if (Shader)
        {
            ShaderModules[Index] = Shader->GetOrCreateHandle(Desc, Layout, Layout->GetDescriptorSetLayoutHash());
        }
    }
}
