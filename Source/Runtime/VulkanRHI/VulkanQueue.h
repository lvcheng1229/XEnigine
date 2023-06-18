#pragma once
class XVulkanDevice;
class XVulkanCmdBuffer;
class XVulkanQueue
{
public:
	XVulkanQueue(XVulkanDevice* VulkanDevice, uint32 QueueFamilyIndex);
	uint32 GetFamilyIndex() { return FamilyIndex; }
	VkQueue GetVkQueue() 
	{
		return Queue;
	};

	void Submit(XVulkanCmdBuffer* CmdBuffer, uint32 NumSignalSemaphores = 0, VkSemaphore* SignalSemaphores = nullptr);
private:
	friend class VkHack;

	VkQueue Queue;
	uint32 FamilyIndex;
	XVulkanDevice* Device;
};