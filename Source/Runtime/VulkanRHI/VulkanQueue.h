#pragma once
class XVulkanDevice;
class XVulkanQueue
{
public:
	XVulkanQueue(XVulkanDevice* VulkanDevice, uint32 QueueFamilyIndex);
	uint32 GetFamilyIndex() { return FamilyIndex; }
	VkQueue GetVkQueue() 
	{
		return Queue;
	};

private:
	friend class VkHack;

	VkQueue Queue;
	uint32 FamilyIndex;
	XVulkanDevice* Device;
};