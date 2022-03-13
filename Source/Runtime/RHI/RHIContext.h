#pragma once
#include "Runtime/HAL/PlatformTypes.h"
#include "RHIResource.h"
#include "RHIDefines.h"
class IRHIContext
{
public:
	virtual void OpenCmdList() = 0;
	virtual void CloseCmdList() = 0;

	virtual void RHISetShaderUAV(XRHIComputeShader* ShaderRHI, uint32 TextureIndex, XRHIUnorderedAcessView* UAV) = 0;
	virtual void RHISetShaderTexture(XRHIGraphicsShader* ShaderRHI, uint32 TextureIndex, XRHITexture* NewTextureRHI) = 0;;
	virtual void RHISetViewport(float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ) = 0;
	//virtual void RHISetScissorRect(uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY) = 0;
};