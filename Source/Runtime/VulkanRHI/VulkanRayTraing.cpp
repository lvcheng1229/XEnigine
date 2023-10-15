#include "VulkanPlatformRHI.h"
#include "VulkanRayTraing.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "Runtime\RHI\RHICommandList.h"

VkDeviceAddress XVulkanResourceMultiBuffer::GetDeviceAddress() const
{
	VkBufferDeviceAddressInfoKHR DeviceAddressInfo = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	DeviceAddressInfo.buffer = Buffer.VulkanHandle;
	return VulkanExtension::vkGetBufferDeviceAddressKHR(Device->GetVkDevice(), &DeviceAddressInfo);
}

static ERayTracingAccelerationStructureFlags GetRayTracingAccelerationStructureBuildFlags(const XRayTracingGeometryInitializer& Initializer)
{
	ERayTracingAccelerationStructureFlags BuildFlags = ERayTracingAccelerationStructureFlags::None;
	if (Initializer.bPreferBuild)
	{
		BuildFlags = ERayTracingAccelerationStructureFlags::PreferBuild;
	}
	else
	{
		BuildFlags = ERayTracingAccelerationStructureFlags::PreferTrace;
	}

	if (Initializer.bAllowUpdate)
	{
		BuildFlags = ERayTracingAccelerationStructureFlags(uint32(BuildFlags) | uint32(ERayTracingAccelerationStructureFlags::AllowUpdate));
	}

	if(!Initializer.bPreferBuild && !Initializer.bAllowUpdate && Initializer.bAllowCompaction )
	{
		BuildFlags = ERayTracingAccelerationStructureFlags(uint32(BuildFlags) | uint32(ERayTracingAccelerationStructureFlags::AllowCompaction));
	}

	return BuildFlags;
}

static void GetBLASBuildData(
	const VkDevice Device, 
	const std::vector<XRayTracingGeometrySegment>& Segments, 

	std::shared_ptr<XRHIBuffer>IndexBuffer,
	const uint32 IndexBufferOffset,
	const uint32 IndexStrideInBytes,

	ERayTracingAccelerationStructureFlags BuildFlag, 
	EAccelerationStructureBuildMode BuildMode,
	XVkRTBLASBuildData& BuildData)
{
	VkDeviceOrHostAddressConstKHR IndexBufferDeviceAddress = {};
	IndexBufferDeviceAddress.deviceAddress = (IndexBuffer.get() != nullptr) ? static_cast<XVulkanResourceMultiBuffer*>(IndexBuffer.get())->GetDeviceAddress() + IndexBufferOffset : 0;

	std::vector<uint32>PrimitiveCount;
	for (int32 Index = 0; Index < Segments.size(); Index++)
	{
		const XRayTracingGeometrySegment& Segment = Segments[Index];

		XVulkanResourceMultiBuffer* const VertexBuffer = static_cast<XVulkanResourceMultiBuffer*>(Segment.VertexBuffer.get());

		VkDeviceOrHostAddressConstKHR VertexBufferDeviceAddress = {};
		VertexBufferDeviceAddress.deviceAddress = VertexBuffer->GetDeviceAddress() + Segment.VertexBufferOffset;

		VkAccelerationStructureGeometryKHR SegmentGeometry = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		
		if (Segment.bForceOpaque)
		{
			SegmentGeometry.flags |= VK_GEOMETRY_OPAQUE_BIT_KHR;
		}

		SegmentGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;

		SegmentGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		SegmentGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		SegmentGeometry.geometry.triangles.vertexData = VertexBufferDeviceAddress;
		SegmentGeometry.geometry.triangles.maxVertex = Segment.MaxVertices;
		SegmentGeometry.geometry.triangles.vertexStride = Segment.VertexBufferStride;
		SegmentGeometry.geometry.triangles.indexData = IndexBufferDeviceAddress;

		// No support for segment transform
		SegmentGeometry.geometry.triangles.transformData.deviceAddress = 0;
		SegmentGeometry.geometry.triangles.transformData.hostAddress = nullptr;

		uint32 PrimitiveOffset = 0;
		if (IndexBuffer.get() != nullptr)
		{
			SegmentGeometry.geometry.triangles.indexType = (IndexStrideInBytes == 2) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
			PrimitiveOffset = Segment.FirstPrimitive * 3 * IndexStrideInBytes;
		}
		else
		{
			XASSERT(false);
		}

		BuildData.Segments.push_back(SegmentGeometry);

		VkAccelerationStructureBuildRangeInfoKHR RangeInfo = {};
		RangeInfo.firstVertex = 0;
		RangeInfo.primitiveCount = (Segment.bEnabled) ? Segment.NumPrimitives : 0;
		RangeInfo.transformOffset = 0;
		BuildData.Ranges.push_back(RangeInfo);

		PrimitiveCount.push_back(Segment.NumPrimitives);
	}
	
	BuildData.GeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	BuildData.GeometryInfo.flags = (uint32(BuildFlag) & uint32(ERayTracingAccelerationStructureFlags::PreferBuild)) != 0 ? VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR : VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	BuildData.GeometryInfo.mode = (BuildMode == EAccelerationStructureBuildMode::Build) ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
	BuildData.GeometryInfo.geometryCount = BuildData.Segments.size();
	BuildData.GeometryInfo.pGeometries = BuildData.Segments.data();

	VulkanExtension::vkGetAccelerationStructureBuildSizesKHR(Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &BuildData.GeometryInfo, PrimitiveCount.data(), &BuildData.SizesInfo);
}

XRayTracingAccelerationStructSize XVulkanPlatformRHI::RHICalcRayTracingGeometrySize(const XRayTracingGeometryInitializer& Initializer)
{
	const uint32 IndexStrideInBytes = Initializer.IndexBuffer ? Initializer.IndexBuffer->GetStride() : 0;

	XVkRTBLASBuildData BuildData;
	GetBLASBuildData(GetDevice()->GetVkDevice(), Initializer.Segments, Initializer.IndexBuffer, Initializer.IndexBufferOffset, IndexStrideInBytes, GetRayTracingAccelerationStructureBuildFlags(Initializer), EAccelerationStructureBuildMode::Build, BuildData);
	
	XRayTracingAccelerationStructSize Result;
	Result.ResultSize = AlignArbitrary(BuildData.SizesInfo.accelerationStructureSize, GRHIRayTracingAccelerationStructureAlignment);
	Result.BuildScratahSize = AlignArbitrary(BuildData.SizesInfo.buildScratchSize, GRHIRayTracingScratchBufferAlignment);
	Result.UpdateScratchSize = AlignArbitrary(BuildData.SizesInfo.updateScratchSize, GRHIRayTracingScratchBufferAlignment);
	return Result;
}

std::shared_ptr<XRHIRayTracingGeometry> XVulkanPlatformRHI::RHICreateRayTracingGeometry(const XRayTracingGeometryInitializer& Initializer)
{
	return std::make_shared<XVulkanRayTracingGeometry>(Initializer, GetDevice());
}

void XVulkanCommandListContext::RHIBuildAccelerationStructures(const std::span<const XRayTracingGeometryBuildParams> Params, const XRHIBufferRange& ScratchBufferRange)
{
	for (const XRayTracingGeometryBuildParams& P : Params)
	{

	}
}

XVulkanRayTracingGeometry::XVulkanRayTracingGeometry(const XRayTracingGeometryInitializer& Initializer, XVulkanDevice* InDevice)
	:XRHIRayTracingGeometry(Initializer)
	,Device(InDevice)
{
	SizeInfo = RHICalcRayTracingGeometrySize(Initializer);

	XRHIResourceCreateData ResourceCreateData;
	AccelerationStructureBuffer = RHICreateBuffer(0, SizeInfo.ResultSize, EBufferUsage::BUF_AccelerationStructure, ResourceCreateData);
	
	VkAccelerationStructureCreateInfoKHR CreateInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
	CreateInfo.buffer = AccelerationStructureBuffer.get()->Buffer.VulkanHandle;
	CreateInfo.offset = AccelerationStructureBuffer.get()->Buffer.Offset;
	CreateInfo.size = SizeInfo.ResultSize;
	CreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

	VulkanExtension::vkCreateAccelerationStructureKHR(Device->GetVkDevice(), &CreateInfo, nullptr, &Handle);

	VkAccelerationStructureDeviceAddressInfoKHR DeviceAddressInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
	DeviceAddressInfo.accelerationStructure = Handle;
	Address = VulkanExtension::vkGetAccelerationStructureDeviceAddressKHR(Device->GetVkDevice(), &DeviceAddressInfo);
}
