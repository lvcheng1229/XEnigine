#pragma once
#include <map>
class XVulkanRenderPass;
class XVulkanDevice;
class XVulkanRenderTargetLayout;
class XVulkanCmdBuffer;
class XVulkanCommandListContext;

class XVulkanFramebuffer
{
public:
	XVulkanFramebuffer(XVulkanDevice* Device, const XRHISetRenderTargetsInfo* InRTInfo, const XVulkanRenderTargetLayout* RTLayout, const XVulkanRenderPass* RenderPass);
	bool Matches(const XRHISetRenderTargetsInfo& RTInfo) const;

	inline VkRect2D GetRenderArea() const
	{
		return RenderArea;
	}
private:
	VkFramebuffer Framebuffer;
	VkRect2D RenderArea;

	uint32 NumColorRenderTargets;

	uint32 NumColorAttachments;
	VkImage ColorRenderTargetImages[MaxSimultaneousRenderTargets];
	VkImage DepthStencilRenderTargetImage;
};

class XVulkanLayoutManager
{
public:
	XVulkanLayoutManager()
	{

	}

	~XVulkanLayoutManager();

	XVulkanFramebuffer* GetOrCreateFramebuffer(
		XVulkanDevice* InDevice,
		const XRHISetRenderTargetsInfo* RenderTargetsInfo, 
		const XVulkanRenderTargetLayout* RTLayout, 
		XVulkanRenderPass* RenderPass);

	void BeginRenderPass(
		XVulkanCommandListContext* Context, 
		XVulkanDevice* InDevice, 
		XVulkanCmdBuffer* CmdBuffer,
		const XRHIRenderPassInfo* RPInfo, const XVulkanRenderTargetLayout* RTLayout, XVulkanRenderPass* RenderPass, XVulkanFramebuffer* Framebuffer);

	XVulkanRenderPass* GetOrCreateRenderPass(XVulkanDevice* InDevice, const XVulkanRenderTargetLayout* RTLayout);
private:
	struct XFramebufferList
	{
		std::vector<XVulkanFramebuffer*> Framebuffer;
	};
	std::map<uint32, XVulkanRenderPass*>RenderPasses;
	std::map<uint32, XFramebufferList*> Framebuffers;
};