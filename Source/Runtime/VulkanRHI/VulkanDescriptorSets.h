#pragma once
#include <vector>
#include <vulkan\vulkan_core.h>
#include "Runtime\HAL\PlatformTypes.h"
#include <map>
#include <string>

class XVulkanDevice;
class XVulkanShader;

// Information for the layout of descriptor sets; does not hold runtime objects
class XVulkanDescriptorSetsLayoutInfo
{
public:
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
protected:
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
	
	VkDescriptorSetLayout LayoutHandle;
private:
	XVulkanDevice* Device;
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

class XVulkanDescriptorPoolSetContainer
{
public:
};

class XVulkanDescriptorPoolsManager
{
public:
	XVulkanDescriptorPoolSetContainer& AcquirePoolSetContainer();
private:
};