#pragma once
#include "VulkanRHIPrivate.h"
#include "VulkanResource.h"

struct XVkRTBLASBuildData
{
	std::vector<VkAccelerationStructureGeometryKHR> Segments;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR>Ranges;
	VkAccelerationStructureBuildGeometryInfoKHR GeometryInfo;
	VkAccelerationStructureBuildSizesInfoKHR SizesInfo;
};

class XVulkanRayTracingGeometry : public XRHIRayTracingGeometry
{
public:
	XVulkanRayTracingGeometry(const XRayTracingGeometryInitializer& Initializer, XVulkanDevice* InDevice);

	std::shared_ptr<XVulkanResourceMultiBuffer>AccelerationStructureBuffer;

	VkAccelerationStructureKHR Handle = nullptr;
	VkDeviceAddress Address = 0;

private:
	XVulkanDevice* const Device = nullptr;
};
