#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanRHIPrivate.h"
#include "VulkanBarriers.h"

#include "VulkanResource.h"
#include "VulkanCommandBuffer.h"

XVulkanLayoutManager XVulkanCommandListContext::GlobalLayoutManager;

XVulkanLayoutManager::~XVulkanLayoutManager()
{
	for (auto iter = RenderPasses.begin(); iter != RenderPasses.end(); iter++)
	{
		delete iter->second;
	}
}

XVulkanFramebuffer* XVulkanLayoutManager::GetOrCreateFramebuffer(XVulkanDevice* InDevice, const XRHISetRenderTargetsInfo* RenderTargetsInfo, const XVulkanRenderTargetLayout* RTLayout, XVulkanRenderPass* RenderPass)
{
	static uint32 HashHackTemp = 0;
	HashHackTemp++;
	HashHackTemp = HashHackTemp % 2;

	uint32 RTLayoutHash = RTLayout->GetRenderPassCompatibleHash() + HashHackTemp;

	auto iter = Framebuffers.find(RTLayoutHash);
	XFramebufferList* FramebufferList = nullptr;
	if (iter != Framebuffers.end())
	{
		XASSERT_TEMP(false);
		for (int32 Index = 0; Index < iter->second->Framebuffer.size(); Index++)
		{
			if (iter->second->Framebuffer[Index]->Matches(*RenderTargetsInfo))
			{
				return iter->second->Framebuffer[Index];
			}
		}
	}
	
	FramebufferList = new XFramebufferList;
	Framebuffers[RTLayoutHash] = FramebufferList;
	XVulkanFramebuffer* Framebuffer = new XVulkanFramebuffer(InDevice, RenderTargetsInfo, RTLayout, RenderPass);
	FramebufferList->Framebuffer.push_back(Framebuffer);
	return Framebuffer;
}

void XVulkanLayoutManager::BeginRenderPass(XVulkanCmdBuffer* CmdBuffer, const XVulkanRenderTargetLayout* RTLayout, XVulkanRenderPass* RenderPass, XVulkanFramebuffer* Framebuffer)
{
	CmdBuffer->BeginRenderPass(RTLayout, RenderPass, Framebuffer);
}

XVulkanRenderPass* XVulkanLayoutManager::GetOrCreateRenderPass(XVulkanDevice* InDevice, const XVulkanRenderTargetLayout* RTLayout)
{
	const uint32 RenderPassHash = RTLayout->GetRenderPassFullHash();
	auto iter = RenderPasses.find(RenderPassHash);
	if (iter != RenderPasses.end())
	{
		return iter->second;
	}

	XVulkanRenderPass* RenderPass = new XVulkanRenderPass(InDevice, RTLayout);
	RenderPasses[RenderPassHash] = RenderPass;
	return RenderPass;
}


bool XVulkanFramebuffer::Matches(const XRHISetRenderTargetsInfo& RTInfo) const
{
	bool bMatch = false;
	for (int32 Index = 0; Index < RTInfo.NumColorRenderTargets; Index++)
	{
		const VkImage Image = GetVulkanTextureFromRHITexture(RTInfo.ColorRenderTarget[Index].Texture)->Surface.Image;
		if (ColorRenderTargetImages[Index] != Image)
		{
			return false;
		}
	}
	XASSERT_TEMP(false);
	return true;
}
