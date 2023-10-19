#pragma once
#include "VulkanRHIPrivate.h"
#include "VulkanResource.h"

struct XVkRTBLASBuildData
{
	std::vector<VkAccelerationStructureBuildRangeInfoKHR>Ranges;

	std::vector<VkAccelerationStructureGeometryKHR> Segments;
	VkAccelerationStructureBuildGeometryInfoKHR GeometryInfo;
	VkAccelerationStructureBuildSizesInfoKHR SizesInfo;
};

struct XVkRTTLASBuildData
{
	XVkRTTLASBuildData()
	{
		Geometry = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		GeometryInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		SizesInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
	}

	VkAccelerationStructureGeometryKHR Geometry;
	VkAccelerationStructureBuildGeometryInfoKHR GeometryInfo;
	VkAccelerationStructureBuildSizesInfoKHR SizesInfo;
};

class XVulkanRayTracingGeometry : public XRHIRayTracingGeometry
{
public:
	XVulkanRayTracingGeometry(const XRayTracingGeometryInitializer& Initializer, XVulkanDevice* InDevice);

	std::shared_ptr<XRHIBuffer>AccelerationStructureBuffer;

	VkAccelerationStructureKHR Handle = nullptr;
	VkDeviceAddress Address = 0;

private:
	XVulkanDevice* const Device = nullptr;
};

class XVulkanRayTracingScene : public XRHIRayTracingScene
{
public:
	XVulkanRayTracingScene(XRayTracingSceneInitializer Initializer, XVulkanDevice* InDevice);

	//assign in bind buffer
	std::shared_ptr<XVulkanShaderResourceView> ShaderResourceView;

	void BindBuffer(std::shared_ptr<XRHIBuffer> InBuffer, uint32 InBufferOffset);
	
	void BuildAccelerationStructure(XVulkanCommandListContext& CmdContext, XVulkanResourceMultiBuffer* ScratchBuffer, uint32 ScratchOffset, XVulkanResourceMultiBuffer* InstanceBuffer, uint32 InstanceOffset);
	void BuildPerInstanceGeometryParameterBuffer(XVulkanCommandListContext& CmdContext);

	//layer
	std::shared_ptr<XVulkanShaderResourceView>PerInstanceGeometryParameterSRV;
	std::shared_ptr<XVulkanResourceMultiBuffer> PerInstanceGeometryParameterBuffer;
	std::shared_ptr<XRHIBuffer>AccelerationStructureBuffer;
	
private:
	const XRayTracingSceneInitializer Initializer;
	XVulkanDevice* const Device = nullptr;
};
