#pragma once
#include <vector>
#include <vulkan\vulkan_core.h>
#include "Runtime\HAL\PlatformTypes.h"
#include "VulkanUtil.h"
#include "VulkanState.h"
#include "VulkanResource.h"
#include <map>
#include <string>

class XVulkanDevice;
class XVulkanShader;

// This container holds the actual VkWriteDescriptorSet structures; a Compute pipeline uses the arrays 'as-is', whereas a 
// Gfx PSO will have one big array and chunk it depending on the stage (eg Vertex, Pixel).
struct XVulkanDescriptorSetWriteContainer
{
	//std::vector<FVulkanHashableDescriptorInfo> HashableDescriptorInfo;
	std::vector<VkDescriptorImageInfo> DescriptorImageInfo;
	std::vector<VkDescriptorBufferInfo> DescriptorBufferInfo;
	std::vector<VkWriteDescriptorSet> DescriptorWrites;
	//std::vector<uint8> BindingToDynamicOffsetMap;
};

// Information for remapping descriptor sets when combining layouts
struct XDescriptorSetRemappingInfo
{
	struct XSetInfo
	{
		std::vector<VkDescriptorType>	Types;
		uint16						NumImageInfos = 0;
		uint16						NumBufferInfos = 0;
	};
	XSetInfo SetInfo;
};

// This class encapsulates updating VkWriteDescriptorSet structures (but doesn't own them), and their flags for dirty ranges; it is intended
// to be used to access a sub-region of a long array of VkWriteDescriptorSet (ie FVulkanDescriptorSetWriteContainer)
class XVulkanDescriptorSetWriter
{
public:
	bool WriteImage(uint32 DescriptorIndex, const XVulkanTextureView& TextureView, VkImageLayout Layout)
	{
		return WriteTextureView<VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE>(DescriptorIndex, TextureView, Layout);
	}
	uint32 SetupDescriptorWrites(
		const std::vector<VkDescriptorType>& Types, VkWriteDescriptorSet* InWriteDescriptors,
		VkDescriptorImageInfo* InImageInfo, VkDescriptorBufferInfo* InBufferInfo,
		const XVulkanSamplerState& DefaultSampler, const XVulkanTextureView& DefaultImageView);
protected:
	template <VkDescriptorType DescriptorType>
	bool WriteTextureView(uint32 DescriptorIndex, const XVulkanTextureView& TextureView, VkImageLayout Layout)
	{

		VkDescriptorImageInfo* ImageInfo = const_cast<VkDescriptorImageInfo*>(WriteDescriptors[DescriptorIndex].pImageInfo);
		bool bChanged = false;
		bChanged = CopyAndReturnNotEqual(ImageInfo->imageView, TextureView.View);
		bChanged |= CopyAndReturnNotEqual(ImageInfo->imageLayout, Layout);

		return bChanged;
	}

	
	uint32 NumWrites;

	// A view into someone else's descriptors
	VkWriteDescriptorSet* WriteDescriptors;
};

class XVulkanGfxPipelineDescriptorInfo
{

};

// Information for the layout of descriptor sets; does not hold runtime objects
class XVulkanDescriptorSetsLayoutInfo
{
public:
	XVulkanDescriptorSetsLayoutInfo()
	{
		for (uint32 i = VK_DESCRIPTOR_TYPE_SAMPLER; i <= VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; ++i)
		{
			LayoutTypes[i] = 0;
		}
	}

	inline uint32 GetTypesUsed(VkDescriptorType Type) const
	{
		return LayoutTypes.find(static_cast<uint32>(Type))->second;
	}

	inline uint32 GetHash() const
	{
		return Hash;
	}

	struct XSetLayout
	{
		std::vector<VkDescriptorSetLayoutBinding> LayoutBindings;

		inline void GenerateHash()
		{
			Hash = std::hash<std::string>{}(std::string((char*)LayoutBindings.data(), sizeof(VkDescriptorSetLayoutBinding) * LayoutBindings.size()));

		}

		uint32 GetHash()
		{
			return Hash;
		}

		uint32 Hash;
	};

	void FinalizeBindings_Gfx(XVulkanShader* VertexShader, XVulkanShader* PixelShader);
	XDescriptorSetRemappingInfo	RemappingInfo;
protected:
	std::map<uint32, uint32> LayoutTypes;

	XSetLayout SetLayout;//r.Vulkan.DescriptorSetLayoutMode

	uint32 Hash = 0;


	friend class XVulkanLayout;
	friend class XVulkanPipelineStateCacheManager;
};

using XVulkanDescriptorSetLayoutMap = std::map<uint32, VkDescriptorSetLayout>;


// The actual run-time descriptor set layouts
class XVulkanDescriptorSetsLayout : public XVulkanDescriptorSetsLayoutInfo
{
public:
	XVulkanDescriptorSetsLayout(XVulkanDevice* InDevice);

	void Compile(XVulkanDescriptorSetLayoutMap& LayoutMap);
	
	inline const VkDescriptorSetAllocateInfo& GetAllocateInfo() const
	{
		return DescriptorSetAllocateInfo;
	}

	VkDescriptorSetLayout LayoutHandle;
private:
	XVulkanDevice* Device;
	VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo;
};


class XVulkanLayout
{
public:
	XVulkanLayout(XVulkanDevice* InDevice):DescriptorSetLayout(InDevice), Device(InDevice){}

	inline uint32 GetDescriptorSetLayoutHash() const
	{
		return DescriptorSetLayout.GetHash();
	}

	void Compile(XVulkanDescriptorSetLayoutMap& InDSetLayoutMap);

	const XVulkanDescriptorSetsLayout* GetDescriptorSetsLayout()
	{
		return &DescriptorSetLayout;
	}
protected:
	XVulkanDescriptorSetsLayout	DescriptorSetLayout;
	XVulkanDevice* Device;
	VkPipelineLayout PipelineLayout;

	friend class XVulkanPipelineStateCacheManager;
};



class XVulkanGfxLayout : public XVulkanLayout
{
public:
	XVulkanGfxLayout(XVulkanDevice* InDevice) :XVulkanLayout(InDevice) {}
};



















class XVulkanDescriptorPool
{
public:
	XVulkanDescriptorPool(XVulkanDevice* InDevice, const XVulkanDescriptorSetsLayout* Layout, uint32 MaxSetsAllocations);
	bool AllocateDescriptorSets(const VkDescriptorSetAllocateInfo& InDescriptorSetAllocateInfo, VkDescriptorSet* OutSets);
private:
	XVulkanDevice* Device;
	const XVulkanDescriptorSetsLayout* Layout;
	VkDescriptorPool DescriptorPool;

	uint32 MaxDescriptorSets;
};



class XVulkanTypedDescriptorPoolSet
{
	
	XVulkanDescriptorPool* PushNewPool();
public:

	XVulkanTypedDescriptorPoolSet(XVulkanDevice* InDevice, const XVulkanDescriptorSetsLayout* InLayout)
		: Device(InDevice)
		, Layout(InLayout)
	{
		PushNewPool();
	};

	~XVulkanTypedDescriptorPoolSet();

	bool AllocateDescriptorSets(const XVulkanDescriptorSetsLayout* Layout, VkDescriptorSet* OutSets);
private:
	XVulkanDevice* Device;
	const XVulkanDescriptorSetsLayout* Layout;
	std::vector<XVulkanDescriptorPool*> PoolArray;
};


class XVulkanDescriptorPoolSetContainer
{
public:
	XVulkanDescriptorPoolSetContainer(XVulkanDevice* InDevice)
		:Device(InDevice)
		, bUsed(false)
	{

	}

	~XVulkanDescriptorPoolSetContainer();

	inline void SetUsed(bool bInUsed)
	{
		bUsed = bInUsed;
	}

	inline bool IsUnused() const
	{
		return !bUsed;
	}

	XVulkanTypedDescriptorPoolSet* AcquireTypedPoolSet(const XVulkanDescriptorSetsLayout* Layout);

private:
	bool bUsed;
	XVulkanDevice* Device;

	std::map<uint32, XVulkanTypedDescriptorPoolSet*>TypedDescriptorPools;
};

class XVulkanDescriptorPoolsManager
{
public:
	XVulkanDescriptorPoolsManager(XVulkanDevice* InDevice)
		:Device(InDevice)
	{
		
	}

	XVulkanDescriptorPoolSetContainer& AcquirePoolSetContainer();
private:
	std::vector<XVulkanDescriptorPoolSetContainer*>PoolSets;
	XVulkanDevice* Device;
};