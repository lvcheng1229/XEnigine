#pragma once


class XVulkanDescriptorSetsLayoutInfo
{
protected:
	uint32 Hash = 0;
};

class XVulkanDescriptorSetsLayout : public XVulkanDescriptorSetsLayoutInfo
{
public:
	inline uint32 GetHash() const
	{
		return Hash;
	}
};

class XVulkanLayout
{
public:
	inline uint32 GetDescriptorSetLayoutHash() const
	{
		return DescriptorSetLayout.GetHash();
	}
protected:
	XVulkanDescriptorSetsLayout	DescriptorSetLayout;
};