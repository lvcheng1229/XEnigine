#include "VulkanExtensions.h"

template <typename ExistingChainType, typename NewStructType>
static void AddToPNext(ExistingChainType& Existing, NewStructType& Added)
{
	Added.pNext = (void*)Existing.pNext;
	Existing.pNext = (void*)&Added;
}


class XVulkanAccelerationExtensionStructExtension : public XVulkanDeviceExtension
{
public:
	XVulkanAccelerationExtensionStructExtension(XVulkanDevice* InDevice)
		:XVulkanDeviceExtension(InDevice, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)
	{

	}

	virtual void PrePhysicalDeviceProperties(VkPhysicalDeviceProperties2KHR& PhysicalDeviceProperties2)final override 
	{
#if RHI_RAYTRACING
		const XRayTracingProperties& RayTracingProperties = Device->GetRayTracingProperties();
		VkPhysicalDeviceAccelerationStructurePropertiesKHR& AccelerationStructure = const_cast<VkPhysicalDeviceAccelerationStructurePropertiesKHR&>(RayTracingProperties.AccelerationStructure);
		AccelerationStructure = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR };
		AddToPNext(PhysicalDeviceProperties2, AccelerationStructure);
#endif	
	}

	virtual void PrePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2KHR& PhysicalDeviceFeatures2)final override 
	{
		AccelerationStructureFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
		AddToPNext(PhysicalDeviceFeatures2, AccelerationStructureFeatures);
	}

	virtual void PreCreateDevice(VkDeviceCreateInfo& DeviceCreateInfo) final override
	{
		AddToPNext(DeviceCreateInfo, AccelerationStructureFeatures);
	}

private:
	VkPhysicalDeviceAccelerationStructureFeaturesKHR AccelerationStructureFeatures;
};

class XVulkanayTracingPipelineExtension : public XVulkanDeviceExtension
{
public:
	XVulkanayTracingPipelineExtension(XVulkanDevice* InDevice)
		:XVulkanDeviceExtension(InDevice, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
	{

	}

	virtual void PrePhysicalDeviceProperties(VkPhysicalDeviceProperties2KHR& PhysicalDeviceProperties2)final override
	{
#if RHI_RAYTRACING
		const XRayTracingProperties& RayTracingProperties = Device->GetRayTracingProperties();
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR& RayTracingPipeline = const_cast<VkPhysicalDeviceRayTracingPipelinePropertiesKHR&>(RayTracingProperties.RayTracingPipeline);
		RayTracingPipeline = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR };
		AddToPNext(PhysicalDeviceProperties2, RayTracingPipeline);
#endif
	}

	virtual void PrePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2KHR& PhysicalDeviceFeatures2)final override
	{
		RayTracingPipelineFeature = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
		AddToPNext(PhysicalDeviceFeatures2, RayTracingPipelineFeature);
	}

	virtual void PreCreateDevice(VkDeviceCreateInfo& DeviceCreateInfo) final override
	{
		AddToPNext(DeviceCreateInfo, RayTracingPipelineFeature);
	}

private:
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR RayTracingPipelineFeature;
};

static std::vector<XVulkanDeviceExtension> vkDeviceExtensions;
static bool bInit = false;
std::vector<XVulkanDeviceExtension>& XVulkanDeviceExtension::GetSupportedDeviceExtensions(XVulkanDevice* InDevice)
{
	if (bInit == false)
	{
		vkDeviceExtensions.push_back(XVulkanAccelerationExtensionStructExtension(InDevice));
		vkDeviceExtensions.push_back(XVulkanayTracingPipelineExtension(InDevice));
	}
	return vkDeviceExtensions;
};