#include "RayTracingInstanceBufferUtil.h"
#include "Runtime\RHI\RHICommandList.h"

XRayTracingSceneWithGeometryInstance CreateRayTracingSceneWithGeometryInstance(std::vector<XRayTracingGeometryInstance>& Instances, uint32 NumShaderSlotsPerGeometrySegment, uint32 NumMissShaderSlots, uint32 NumCallableShaderSlots)
{
	const uint32 NumSceneInstances = Instances.size();

	XRayTracingSceneWithGeometryInstance Output;
	XRayTracingSceneInitializer Initializer;
	//Initializer.NumNativeInstance = ;
	
	for (uint32 Index = 0; Index < NumSceneInstances; Index++)
	{
		const XRayTracingGeometryInstance& InstanceDesc = Instances[Index];
		Initializer.NumTotalSegments += InstanceDesc.GeometryRHI->GetNumSegments();
		Initializer.NumNativeInstance += InstanceDesc.NumTransforms;
	}

	Output.Scene = RHICreateRayTracingScene(Initializer);
	return Output;
}