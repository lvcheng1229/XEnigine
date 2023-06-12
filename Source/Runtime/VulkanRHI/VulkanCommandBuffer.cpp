#include <vulkan\vulkan_core.h>
#include "VulkanCommandBuffer.h"
#include "VulkanQueue.h"
#include "VulkanPlatformRHI.h"
#include "VulkanDevice.h"
#include "VulkanRHIPrivate.h"

XVulkanCmdBuffer::XVulkanCmdBuffer(XVulkanDevice* InDevice, XVulkanCommandBufferPool* InCommandBufferPool)
	: Device(InDevice)
	, CmdBufferPool(InCommandBufferPool)
{
	AllocMemory();
}

void XVulkanCmdBuffer::AllocMemory()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = CmdBufferPool->GetVkPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VULKAN_VARIFY(vkAllocateCommandBuffers(Device->GetVkDevice(), &allocInfo, &CommandBufferHandle));
}

void XVulkanCmdBuffer::BeginRenderPass(const XVulkanRenderTargetLayout* Layout, XVulkanRenderPass* RenderPass, XVulkanFramebuffer* Framebuffer)
{
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = RenderPass->GetRenderPass();
	renderPassInfo.framebuffer = Framebuffer->GetFramebuffer();
	renderPassInfo.renderArea = Framebuffer->GetRenderArea();
	
	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(CommandBufferHandle, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

bool XVulkanCmdBuffer::AcquirePoolSetAndDescriptorsIfNeeded(const XVulkanDescriptorSetsLayout* Layout, bool bNeedDescriptors, VkDescriptorSet* OutDescriptors)
{
	if (CurrentDescriptorPoolSetContainer == nullptr)
	{
		AcquirePoolSetContainer();
	}


	return false;
}

void XVulkanCmdBuffer::AcquirePoolSetContainer()
{
	CurrentDescriptorPoolSetContainer = &Device->GetDescriptorPoolsManager()->AcquirePoolSetContainer();
}

XVulkanCommandBufferPool::XVulkanCommandBufferPool(XVulkanDevice* InDevice, XVulkanCommandBufferManager* InVulkanCommandBufferManager)
	: Device(InDevice)
	, CmdBufferManager(InVulkanCommandBufferManager)
{
}

XVulkanCommandBufferPool::~XVulkanCommandBufferPool()
{
	for (auto iter = FreeCmdBuffers.begin(); iter != FreeCmdBuffers.end(); iter++)
	{
		delete* iter;
	}

	for (auto iter = CmdBuffers.begin(); iter != CmdBuffers.end(); iter++)
	{
		delete* iter;
	}
}

void XVulkanCommandBufferPool::Create(uint32 QueueFamilyIndexdex)
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = QueueFamilyIndexdex;

	VULKAN_VARIFY(vkCreateCommandPool(Device->GetVkDevice(), &poolInfo, nullptr, &CmdPool));
}

XVulkanCmdBuffer* XVulkanCommandBufferPool::CreateCmdBuffer()
{
	for (auto iter = FreeCmdBuffers.begin(); iter != FreeCmdBuffers.end(); iter++)
	{
		XVulkanCmdBuffer* CmdBuffer = *iter;
		FreeCmdBuffers.erase(iter);
		CmdBuffers.push_back(CmdBuffer);
		return CmdBuffer;
	}

	XVulkanCmdBuffer* CmdBuffer = new XVulkanCmdBuffer(Device, this);
	CmdBuffers.push_back(CmdBuffer);
	return CmdBuffer;
}

VkCommandPool XVulkanCommandBufferPool::GetVkPool()
{
	return CmdPool;
}

XVulkanCommandBufferManager::XVulkanCommandBufferManager(XVulkanDevice* InDevice, XVulkanCommandListContext* InContext)
	: Device(InDevice)
	, Context(InContext)
	, Pool(InDevice, this)
	, Queue(InContext->GetQueue())
{

	Pool.Create(Queue->GetFamilyIndex());
	ActiveCmdBuffer = Pool.CreateCmdBuffer();
}


