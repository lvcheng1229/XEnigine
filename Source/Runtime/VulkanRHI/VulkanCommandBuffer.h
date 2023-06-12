#pragma once
#include <Runtime\HAL\PlatformTypes.h>
#include "VulkanContext.h"

class XVulkanCommandBufferPool;
class XVulkanDescriptorPoolSetContainer;

class XVulkanCmdBuffer
{
public:
	XVulkanCmdBuffer(XVulkanDevice* InDevice, XVulkanCommandBufferPool* InCommandBufferPool);
	void AllocMemory();
	void BeginRenderPass(const XVulkanRenderTargetLayout* Layout,XVulkanRenderPass* RenderPass, XVulkanFramebuffer* Framebuffer);
	VkCommandBuffer GetHandle() { return CommandBufferHandle; }
	VkCommandBuffer* GetHandlePtr() { return &CommandBufferHandle; }

	bool AcquirePoolSetAndDescriptorsIfNeeded(const class XVulkanDescriptorSetsLayout* Layout, bool bNeedDescriptors, VkDescriptorSet* OutDescriptors);
private:
	void AcquirePoolSetContainer();

	friend class VkHack;
	VkCommandBuffer CommandBufferHandle;
	XVulkanDevice* Device;
	XVulkanCommandBufferPool* CmdBufferPool;

	XVulkanDescriptorPoolSetContainer* CurrentDescriptorPoolSetContainer = nullptr;
};

class XVulkanCommandBufferPool
{
public:
	XVulkanCommandBufferPool(XVulkanDevice* InDevice, XVulkanCommandBufferManager* InVulkanCommandBufferManager);
	~XVulkanCommandBufferPool();
	void Create(uint32 InQueueFamilyIndexdex);
	XVulkanCmdBuffer* CreateCmdBuffer();
	VkCommandPool GetVkPool();
private:
	friend class VkHack;

	std::vector<XVulkanCmdBuffer*> CmdBuffers;
	std::vector<XVulkanCmdBuffer*> FreeCmdBuffers;

	XVulkanDevice* Device;
	XVulkanCommandBufferManager* CmdBufferManager;
	VkCommandPool CmdPool;;
};
class XVulkanCommandBufferManager
{
public:
	XVulkanCommandBufferManager(XVulkanDevice* InDevice, XVulkanCommandListContext* InContext);
	XVulkanCmdBuffer* GetActiveCmdBuffer() { return ActiveCmdBuffer; }
private:
	friend class VkHack;

	XVulkanDevice* Device;
	XVulkanCommandListContext* Context;
	XVulkanCommandBufferPool Pool;
	XVulkanQueue* Queue;
	XVulkanCmdBuffer* ActiveCmdBuffer;
};