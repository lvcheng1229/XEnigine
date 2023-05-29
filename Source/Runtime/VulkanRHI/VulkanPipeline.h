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
		bool bBlendEnable;
		uint8 ColorBlendOp;
		uint8 SrcColorBlendFactor;
		uint8 DstColorBlendFactor;
		uint8 AlphaBlendOp;
		uint8 SrcAlphaBlendFactor;
		uint8 DstAlphaBlendFactor;
		void WriteInto(VkPipelineColorBlendAttachmentState& OutState) const;
	};
	std::vector<XBlendAttachment> ColorAttachmentStates;

	struct XDepthStencil
	{
		bool bEnableDepthWrite;
		VkCompareOp DSCompareOp;
	};
	XDepthStencil DepthStencil;


	struct XVertexBinding
	{
		uint32 Stride;
	};
	XVertexBinding VertexBinding;

	struct XVertexAttribute
	{
		uint32 Location;
		uint32 Binding;
		VkFormat Format;
		uint32 Offset;
	};
	std::vector<XVertexAttribute> VertexAttributes;

	struct XRenderTargets
	{
		VkFormat RtFotmats[MaxSimultaneousRenderTargets];
		VkFormat DsFotmats;
	};
	XRenderTargets RenderTargets;

	std::size_t CreateKey();
};

class XVulkanRHIGraphicsPipelineState : public XRHIGraphicsPSO
{
public:
	XVulkanRHIGraphicsPipelineState(XVulkanDevice* Device, const XGraphicsPSOInitializer& PSOInitializer, XGfxPipelineDesc& Desc, std::size_t Key);
	void GetOrCreateShaderModules(XVulkanShader* const* Shaders);

	inline void Bind(VkCommandBuffer CmdBuffer)
	{
		vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanPipeline);
	}

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
	friend class VkHack;

	void CreateGfxEntry(const XGraphicsPSOInitializer& PSOInitializer, XVulkanDescriptorSetsLayoutInfo& DescriptorSetLayoutInfo, XGfxPipelineDesc* Desc);
	void CreateGfxPipelineFromEntry(XVulkanRHIGraphicsPipelineState* PSO, XVulkanShader* Shaders[(uint32)EShaderType::SV_ShaderCount], VkPipeline* Pipeline);
	
	XVulkanDevice* Device;
	VkPipelineCache PipelineCache;
	std::map<std::size_t, XVulkanRHIGraphicsPipelineState*>GraphicsPSOMap;
};