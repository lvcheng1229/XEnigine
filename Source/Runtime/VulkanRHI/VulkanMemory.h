#pragma once
#include <vector>
#include "Runtime\HAL\PlatformTypes.h"
#include "vulkan\vulkan_core.h"
class XMemoryManager;
class XVulkanDevice;

enum EVulkanAllocationType : uint8 {
	EVulkanAllocationEmpty,
	EVulkanAllocationPooledBuffer,
	EVulkanAllocationSize,
};

enum EVulkanAllocationMetaType : uint8
{
	EVulkanAllocationMetaUnknown,
	EVulkanAllocationMetaMultiBuffer,
	EVulkanAllocationMetaSize,t
};

enum class EType
{
	Image,
	Buffer,
};

class XDeviceMemoryAllocation
{
public:
	VkDeviceMemory Handle;
};

class XStagingBuffer
{
public:
	XStagingBuffer(XVulkanDevice* InDevice);
	VkBuffer Buffer;
private:
	XVulkanDevice* Device;
};

class XStagingManager
{
public:
	XStagingManager():Device(nullptr){}
	~XStagingManager();
	void Init(XVulkanDevice* InDevice);
	XStagingBuffer* AcquireBuffer(uint32 Size, VkBufferUsageFlags InUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VkMemoryPropertyFlagBits InMemoryReadFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
private:
	std::vector<XStagingBuffer*>UsedStagingBuffers;
	XVulkanDevice* Device;
};
// Holds a reference to -any- vulkan gpu allocation
class XVulkanAllocation
{
public:
	void Init(XVulkanDevice* InDevice, EVulkanAllocationType Type, EVulkanAllocationMetaType MetaType, VkBuffer InVkBuffer, uint32 AlignedOffset);

	void* Lock(EResourceLockMode LockMode, uint32 Size, uint32 Offset);
	void Unlock();

	VkBuffer VulkanHandle;
	uint32 Offset;

	EVulkanAllocationMetaType MetaType = EVulkanAllocationMetaUnknown;
	uint8 Type : 7;

	XVulkanDevice* Device;
};

class XVulkanEvictable
{

};

class XVulkanSubresourceAllocator
{
public:
	struct XRange
	{
		uint32 Offset;
		uint32 Size;
		
		static void AllocateFromEntry(std::vector<XRange>& Ranges, int32 Index, uint32 SizeToAllocate);
	};


	XVulkanSubresourceAllocator(XVulkanDevice* InDevice,EVulkanAllocationType InType, XMemoryManager* InOwner, VkBuffer InBuffer, uint32 InBufferSize);
	bool TryAllocate(XVulkanAllocation& OutAllocation, XVulkanEvictable* Owner, uint32 Size, uint32 Alignment, EVulkanAllocationMetaType MetaType);

	VkMemoryPropertyFlags MemoryPropertyFlags;
	VkBufferUsageFlags BufferUsageFlags;

	uint32 MaxSize;

private:
	EVulkanAllocationType Type;
	VkBuffer Buffer;
	std::vector<XRange> FreeList;
	XVulkanDevice* Device;
};

class XVulkanResourceHeap
{
public:
	XVulkanResourceHeap(XMemoryManager* InOwner, uint32 InMemoryTypeIndex, uint32 InOverridePageSize = 0);
	bool AllocateResource(XVulkanAllocation& OutAllocation, XVulkanEvictable* AllocationOwner, EType Type, uint32 Size, uint32 Alignment, EVulkanAllocationMetaType MetaType);
};

class XDeviceMemoryManager
{
public:
	XDeviceMemoryManager();
	~XDeviceMemoryManager();

	void Init(XVulkanDevice* Device);
	VkResult GetMemoryTypeFromProperties(uint32 TypeBits, VkMemoryPropertyFlags Properties, uint32* OutTypeIndex);
	XDeviceMemoryAllocation* Alloc(VkDeviceSize AllocationSize, uint32 MemoryTypeIndex);
	
	struct XHeapInfo
	{
		VkDeviceSize UsedSize;
		VkDeviceSize PeakSize;
		std::vector<XDeviceMemoryAllocation*> Allocations;

		XHeapInfo() :
			UsedSize(0),
			PeakSize(0)
		{
		}
	};
	std::vector<XHeapInfo> HeapInfos;

	VkPhysicalDeviceMemoryProperties MemoryProperties;
	XVulkanDevice* Device;

	
};

class XMemoryManager
{
public:
	enum class EPoolSizes : uint8
	{
		E128,
		E256,
		E512,
		E1k,
		E2k,
		E8k,
		E16k,
		SizesCount,
	};
	


	std::vector<XVulkanSubresourceAllocator*>UsedBufferAllocations;
	~XMemoryManager();
	XMemoryManager(XVulkanDevice* InDevice, XDeviceMemoryManager* InDeviceMemoryManager);
	bool AllocateBufferMemory(XVulkanAllocation& OutAllocation, XVulkanEvictable* AllocationOwner, const VkMemoryRequirements& MemoryReqs, VkMemoryPropertyFlags MemoryPropertyFlags, EVulkanAllocationMetaType MetaType);
	void AllocateBufferPooled(
		XVulkanAllocation& OutAllocation, 
		XVulkanEvictable* Owner, uint32 Size,
		VkBufferUsageFlags BufferUsage, 
		VkMemoryPropertyFlags MemoryPropertyFlags, 
		EVulkanAllocationMetaType MetaType);
private:
	XDeviceMemoryManager* DeviceMemoryManager;
	XVulkanDevice* Device;
	std::vector<XVulkanResourceHeap*>ResourceTypeHeaps;

};