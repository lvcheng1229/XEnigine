#pragma once
#include "Runtime/RenderCore/RenderResource.h"

#define VoxelDimension 512
#define OctreeRealBufferSize 1024
#define OctreeHeight 4

#define ApproxVoxelDimension (512*512*16)

#define MinBoundX -20
#define MinBoundY -20
#define MinBoundZ -20

#define MaxBoundX 20
#define MaxBoundY 20
#define MaxBoundZ 20

class XSVOGIResourece :public XRenderResource
{
public:

	//Voxel Scene
	std::shared_ptr<XRHIConstantBuffer>VoxelSceneVSCBuffer;
	std::shared_ptr<XRHIConstantBuffer>VoxelScenePSCBuffer;

	std::shared_ptr <XRHITexture2D> VoxelSceneRTPlaceHolder;

	std::shared_ptr <XRHITexture2D> NodeCountAndOffsetBuffer;

	std::shared_ptr<XRHIStructBuffer>VoxelArrayRW;
	std::shared_ptr<XRHIUnorderedAcessView> VoxelArrayUnorderedAcessView;
	std::shared_ptr<XRHIShaderResourceView> VoxelArrayShaderResourceView;

	//SVO Build
	std::shared_ptr <XRHITexture2D> SpaseVoxelOctree;
	std::shared_ptr<XRHIConstantBuffer>cbSVOBuildBufferLevel0;
	std::shared_ptr<XRHIConstantBuffer>cbSVOBuildBufferLevel1;
	std::shared_ptr<XRHIConstantBuffer>cbSVOBuildBufferLevel2;
	std::shared_ptr<XRHIConstantBuffer>cbSVOBuildBufferLevel3;

	std::shared_ptr<XRHIStructBuffer>SVOCounterBuffer;
	std::shared_ptr<XRHIUnorderedAcessView> SVOCounterBufferUnorderedAcessView;

	void InitRHI()override;
	void ReleaseRHI()override;
};

struct cbSVOBuildBufferStruct
{
	uint32 CurrentLevel;
};

struct VoxelScenePSBufferStruct
{
	XVector3 MinBound;
	float VoxelBufferDimension;
	XVector3 MaxBound;
	float Pad;
};

extern TGlobalResource<XSVOGIResourece>SVOGIResourece;;
extern bool VoxelSceneInitialize;