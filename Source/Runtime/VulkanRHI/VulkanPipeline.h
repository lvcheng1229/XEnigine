#pragma once
#include <vulkan\vulkan_core.h>
#include <map>
#include "Runtime\RHI\RHIResource.h"
#include "VulkanDescriptorSets.h"
class XVulkanShader;
class XVulkanDevice;
class XVulkanRenderPass;

struct XGfxPipelineDesc
{
	struct XBlendAttachment
	{
		uint8 ColorBlendOp;
		uint8 SrcColorBlendFactor;
		uint8 DstColorBlendFactor;
		uint8 AlphaBlendOp;
		uint8 SrcAlphaBlendFactor;
		uint8 DstAlphaBlendFactor;
	};
	std::vector<XBlendAttachment> ColorAttachmentStates;

	struct XDepthStencil
	{
		bool bEnableDepthWrite;
		ECompareFunction DepthTest;
	};
	XDepthStencil DepthStencil;

	struct XRenderTargets
	{

	};
	XRenderTargets RenderTargets;

	std::size_t CreateKey();
};

class XVulkanRHIGraphicsPipelineState : public XRHIGraphicsPSO
{
public:
	XVulkanRHIGraphicsPipelineState(XVulkanDevice* Device, const XGraphicsPSOInitializer& PSOInitializer, XGfxPipelineDesc& Desc, std::size_t Key);
	void GetOrCreateShaderModules(XVulkanShader* const* Shaders);

	XVulkanRenderPass* RenderPass;
	VkPipeline VulkanPipeline;
	XGfxPipelineDesc Desc;
	XVulkanLayout* Layout;

	VkShaderModule ShaderModules[(int32)EShaderType::SV_ShaderCount];
};

class XVulkanPipelineStateCacheManager
{
public:
	XVulkanPipelineStateCacheManager(XVulkanDevice* Device);
	std::shared_ptr<XRHIGraphicsPSO> RHICreateGraphicsPipelineState(const XGraphicsPSOInitializer& PSOInit);
private:
	void CreateGfxEntry(const XGraphicsPSOInitializer& PSOInitializer, XVulkanDescriptorSetsLayoutInfo& DescriptorSetLayoutInfo, XGfxPipelineDesc* Desc);
	void CreateGfxPipelineFromEntry(XVulkanRHIGraphicsPipelineState* PSO, XVulkanShader* Shaders[(uint32)EShaderType::SV_ShaderCount], VkPipeline* Pipeline);
	
	XVulkanDevice* Device;
	VkPipelineCache PipelineCache;
	std::map<std::size_t, XVulkanRHIGraphicsPipelineState*>GraphicsPSOMap;
};