#include "VulkanPlatformRHI.h"
#include "VulkanResource.h"
#include "VulkanDevice.h"

VkBufferUsageFlags XVulkanResourceMultiBuffer::UEToVKBufferUsageFlags(EBufferUsage InUEUsage)
{
	VkBufferUsageFlags OutVkUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	if (((uint32)InUEUsage & (uint32)EBufferUsage::BUF_Vertex) != 0)
	{
		OutVkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}

	if (((uint32)InUEUsage & (uint32)EBufferUsage::BUF_Index) != 0)
	{
		OutVkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	}

	return OutVkUsage;
}

XVulkanResourceMultiBuffer::XVulkanResourceMultiBuffer(XVulkanDevice* Device,uint32 Stride, uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData)
	:XRHIBuffer(Stride, Size)
{
	VkMemoryPropertyFlags BufferMemFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkBufferUsageFlags BufferUsageFlags = UEToVKBufferUsageFlags(Usage);
	Device->GetMemoryManager().AllocateBufferPooled(Buffer, this, Size, BufferUsageFlags, BufferMemFlags, EVulkanAllocationMetaMultiBuffer);
}



std::shared_ptr<XRHIBuffer> XVulkanPlatformRHI::RHICreateVertexBuffer(uint32 Stride, uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData)
{
	return std::make_shared<XVulkanResourceMultiBuffer>(Device, Stride, Size, EBufferUsage((uint32)Usage | (uint32)EBufferUsage::BUF_Vertex), ResourceData);
}