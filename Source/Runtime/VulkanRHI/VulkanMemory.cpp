#include "VulkanMemory.h"
#include "VulkanResource.h"
#include "VulkanDevice.h"
#include <Runtime\Core\Template\AlignmentTemplate.h>

enum
{
	GPU_ONLY_HEAP_PAGE_SIZE = 128 * 1024 * 1024,
	STAGING_HEAP_PAGE_SIZE = 32 * 1024 * 1024,
	ANDROID_MAX_HEAP_PAGE_SIZE = 16 * 1024 * 1024,
	ANDROID_MAX_HEAP_IMAGE_PAGE_SIZE = 16 * 1024 * 1024,
	ANDROID_MAX_HEAP_BUFFER_PAGE_SIZE = 4 * 1024 * 1024,

};
constexpr static uint32 PoolSize = 8192;
constexpr static uint32 BufferSize = 1 * 1024 * 1024;

XVulkanSubresourceAllocator::XVulkanSubresourceAllocator(XVulkanDevice* InDevice, EVulkanAllocationType InType, XMemoryManager* InOwner, VkBuffer InBuffer, uint32 InBufferSize)
	:Buffer(InBuffer)
	,Device(InDevice)
	,Type(InType)
	,MaxSize(InBufferSize)
{
	XRange FullRange;
	FullRange.Offset = 0;
	FullRange.Size = MaxSize;
	FreeList.push_back(FullRange);
}

bool XVulkanSubresourceAllocator::TryAllocate(XVulkanAllocation& OutAllocation, XVulkanEvictable* Owner, uint32 InSize, uint32 Alignment, EVulkanAllocationMetaType MetaType)
{
	for (int32 Index = 0; Index < FreeList.size(); ++Index)
	{
		XRange& Entry = FreeList[Index];
		uint32 AllocatedOffset = Entry.Offset;
		uint32 AlignedOffset = Align(Entry.Offset, Alignment);
		uint32 AlignmentAdjustment = AlignedOffset - Entry.Offset;
		uint32 AllocatedSize = AlignmentAdjustment + InSize;
		if (AllocatedSize <= Entry.Size)
		{
			XRange::AllocateFromEntry(FreeList, Index, AllocatedSize);
			OutAllocation.Init(Device,Type, MetaType, Buffer, AlignedOffset);
			return true;
		}
	}
	return false;
}


XMemoryManager::~XMemoryManager()
{
	for (auto iter : UsedBufferAllocations)
	{
		delete iter;
	}
}

XMemoryManager::XMemoryManager(XVulkanDevice* InDevice, XDeviceMemoryManager* InDeviceMemoryManager)
	:Device(InDevice)
	,DeviceMemoryManager(InDeviceMemoryManager)
{
}

void XMemoryManager::Init()
{

	const VkPhysicalDeviceMemoryProperties& MemoryProperties = DeviceMemoryManager->GetMemoryProperties();
	const uint32 TypeBits = (1 << MemoryProperties.memoryTypeCount) - 1;
	ResourceTypeHeaps.resize(MemoryProperties.memoryTypeCount);

	{
		uint32 TypeIndex;
		VULKAN_VARIFY(DeviceMemoryManager->GetMemoryTypeFromProperties(TypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &TypeIndex));
		ResourceTypeHeaps[TypeIndex] = new XVulkanResourceHeap(this, TypeIndex, STAGING_HEAP_PAGE_SIZE);

		auto& Buckets = ResourceTypeHeaps[TypeIndex]->PageSizeBuckets;
		XVulkanPageSizeBucket Bucket0 = { STAGING_HEAP_PAGE_SIZE, STAGING_HEAP_PAGE_SIZE, XVulkanPageSizeBucket::BUCKET_MASK_IMAGE | XVulkanPageSizeBucket::BUCKET_MASK_BUFFER };
		XVulkanPageSizeBucket Bucket1 = { UINT64_MAX, 0, XVulkanPageSizeBucket::BUCKET_MASK_IMAGE | XVulkanPageSizeBucket::BUCKET_MASK_BUFFER };
		Buckets.push_back(Bucket0);
		Buckets.push_back(Bucket1);
	}
}

void XMemoryManager::AllocateBufferPooled(XVulkanAllocation& OutAllocation, XVulkanEvictable* AllocationOwner, uint32 Size, VkBufferUsageFlags BufferUsage, VkMemoryPropertyFlags MemoryPropertyFlags, EVulkanAllocationMetaType MetaType)
{
	for (int32 Index = 0; Index < UsedBufferAllocations.size(); ++Index)
	{
		XVulkanSubresourceAllocator* SubresourceAllocator = UsedBufferAllocations[Index];
		if ((SubresourceAllocator->BufferUsageFlags & BufferUsage) == BufferUsage &&
			(SubresourceAllocator->MemoryPropertyFlags & MemoryPropertyFlags) == MemoryPropertyFlags)
		{
			uint32 Alignment = 0;
			XASSERT_TEMP(false);//Align
			if (SubresourceAllocator->TryAllocate(OutAllocation, AllocationOwner, Size, Alignment, MetaType))
			{
				return;
			}
		}
	}

	uint32 BufferSize = Size;
	VkBuffer Buffer;
	VkBufferCreateInfo BufferCreateInfo = {};
	BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferCreateInfo.size = BufferSize;
	BufferCreateInfo.usage = BufferUsage;
	VULKAN_VARIFY(vkCreateBuffer(Device->GetVkDevice(), &BufferCreateInfo, nullptr, &Buffer));

	VkMemoryRequirements MemReqs;
	vkGetBufferMemoryRequirements(Device->GetVkDevice(), Buffer, &MemReqs);

	uint32 Alignment = (uint32)MemReqs.alignment;
	uint32 MemoryTypeIndex;
	VULKAN_VARIFY(Device->GetDeviceMemoryManager().GetMemoryTypeFromProperties(MemReqs.memoryTypeBits, MemoryPropertyFlags, &MemoryTypeIndex));

	XDeviceMemoryAllocation* DeviceMemoryAllocation = DeviceMemoryManager->Alloc(BufferSize, MemoryTypeIndex);
	vkBindBufferMemory(Device->GetVkDevice(), Buffer, DeviceMemoryAllocation->Handle, 0);

	XVulkanSubresourceAllocator* SubresourceAllocator = new XVulkanSubresourceAllocator(Device,EVulkanAllocationPooledBuffer, this, Buffer, BufferSize);
	UsedBufferAllocations.push_back(SubresourceAllocator);
	bool Result = SubresourceAllocator->TryAllocate(OutAllocation, AllocationOwner, Size, Alignment, MetaType);
	XASSERT(Result = true);
}

XDeviceMemoryManager::XDeviceMemoryManager()
{
}

XDeviceMemoryManager::~XDeviceMemoryManager()
{
	for (auto iter : HeapInfos)
	{
		for (auto iterHeap : iter.Allocations)
		{
			delete iterHeap;
		}
	}
}

void XDeviceMemoryManager::Init(XVulkanDevice* InDevice)
{
	Device = InDevice;
	vkGetPhysicalDeviceMemoryProperties(*Device->GetVkPhysicalDevice(), &MemoryProperties);
	HeapInfos.resize(MemoryProperties.memoryHeapCount);


}

VkResult XDeviceMemoryManager::GetMemoryTypeFromProperties(uint32 TypeBits, VkMemoryPropertyFlags Properties, uint32* OutTypeIndex)
{
	for (uint32 i = 0; i < MemoryProperties.memoryTypeCount && TypeBits; i++)
	{
		if ((TypeBits & 1) == 1)
		{
			if ((MemoryProperties.memoryTypes[i].propertyFlags & Properties) == Properties)
			{
				*OutTypeIndex = i;
				return VK_SUCCESS;
			}
		}
		TypeBits >>= 1;
	}
	return VK_ERROR_FEATURE_NOT_PRESENT;
}

XDeviceMemoryAllocation* XDeviceMemoryManager::Alloc(VkDeviceSize AllocationSize, uint32 MemoryTypeIndex)
{
	VkDeviceMemory Handle;

	VkMemoryAllocateInfo Info = {};
	Info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	Info.allocationSize = AllocationSize;
	Info.memoryTypeIndex = MemoryTypeIndex;
	VULKAN_VARIFY(vkAllocateMemory(Device->GetVkDevice(), &Info, nullptr, &Handle));

	XDeviceMemoryAllocation* NewAllocation = new XDeviceMemoryAllocation();
	NewAllocation->Handle = Handle;

	uint32 HeapIndex = MemoryProperties.memoryTypes[MemoryTypeIndex].heapIndex;
	HeapInfos[HeapIndex].Allocations.push_back(NewAllocation);

	return NewAllocation;
}

bool XMemoryManager::AllocateBufferMemory(XVulkanAllocation& OutAllocation, XVulkanEvictable* AllocationOwner, const VkMemoryRequirements& MemoryReqs, VkMemoryPropertyFlags MemoryPropertyFlags, EVulkanAllocationMetaType MetaType)
{
	uint32 TypeIndex = 0;
	VkResult Result = DeviceMemoryManager->GetMemoryTypeFromProperties(MemoryReqs.memoryTypeBits, MemoryPropertyFlags, &TypeIndex);
	ResourceTypeHeaps[TypeIndex]->AllocateResource();
	return false;
}

void XVulkanSubresourceAllocator::XRange::AllocateFromEntry(std::vector<XRange>& Ranges, int32 Index, uint32 SizeToAllocate)
{
	XRange& Entry = Ranges[Index];
	if (SizeToAllocate < Entry.Size)
	{
		Entry.Size -= SizeToAllocate;
		Entry.Offset += SizeToAllocate;
	}
}

void XVulkanAllocation::Init(XVulkanDevice* InDevice, EVulkanAllocationType InType, EVulkanAllocationMetaType InMetaType, VkBuffer InVkBuffer, uint32 AlignedOffset)
{
	Device = InDevice;

	VulkanHandle = InVkBuffer;
	Offset = AlignedOffset;

	MetaType = InMetaType;
	Type  = InType;
}

void* XVulkanAllocation::Lock(EResourceLockMode LockMode, uint32 Size, uint32 Offset)
{
	void* Data = nullptr;
	if (LockMode == EResourceLockMode::RLM_WriteOnly)
	{
		XStagingBuffer* StagingBuffer = Device->GetStagingManager().AcquireBuffer(Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		Data = StagingBuffer->GetMappedPointer();
	}
	return Data;
}

void XVulkanAllocation::Unlock()
{
}

XStagingManager::~XStagingManager()
{
	for (auto iter : UsedStagingBuffers)
	{
		delete iter;
	}
}

void XStagingManager::Init(XVulkanDevice* InDevice)
{
	Device = InDevice;
}

XStagingBuffer* XStagingManager::AcquireBuffer(uint32 Size, VkBufferUsageFlags InUsageFlags, VkMemoryPropertyFlagBits InMemoryReadFlags)
{
	if (InMemoryReadFlags == VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
	{
		XASSERT(false);
	}
	if ((InUsageFlags & (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)) != 0)
	{
		InUsageFlags |= (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	}

	XStagingBuffer* StagingBuffer = new XStagingBuffer(Device);
	VkBufferCreateInfo StagingBufferCreateInfo = {};
	StagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	StagingBufferCreateInfo.size = Size;
	StagingBufferCreateInfo.usage = InUsageFlags;
	VULKAN_VARIFY(vkCreateBuffer(Device->GetVkDevice(), &StagingBufferCreateInfo, nullptr, &StagingBuffer->Buffer));
	
	VkMemoryRequirements MemReqs;
	vkGetBufferMemoryRequirements(Device->GetVkDevice(), StagingBuffer->Buffer, &MemReqs);


	VkMemoryPropertyFlags readTypeFlags = InMemoryReadFlags;
	Device->GetMemoryManager().AllocateBufferMemory();

	UsedStagingBuffers.push_back(StagingBuffer);
	return StagingBuffer;
}

XStagingBuffer::XStagingBuffer(XVulkanDevice* InDevice)
	:Device(InDevice)
{
}

XVulkanResourceHeap::XVulkanResourceHeap(XMemoryManager* InOwner, uint32 InMemoryTypeIndex, uint32 InOverridePageSize)
	: Owner(InOwner)
{

}

bool XVulkanResourceHeap::AllocateResource(XVulkanAllocation& OutAllocation, XVulkanEvictable* AllocationOwner, EType Type, uint32 Size, uint32 Alignment, EVulkanAllocationMetaType MetaType)
{
	XDeviceMemoryManager& DeviceMemoryManager = Owner->GetDevice()->GetDeviceMemoryManager();
}
