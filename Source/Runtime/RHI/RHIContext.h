#pragma once
#include "Runtime/HAL/PlatformTypes.h"
#include "RHIResource.h"
#include "RHIDefines.h"
class IRHIContext
{
public:
	virtual void OpenCmdList() = 0;
	virtual void CloseCmdList() = 0;

	//SetShaderParameter
	virtual void RHISetShaderUAV(EShaderType ShaderType, uint32 TextureIndex, XRHIUnorderedAcessView* UAV) = 0;
	virtual void RHISetShaderTexture(EShaderType ShaderType, uint32 TextureIndex, XRHITexture* NewTextureRHI) = 0;
	virtual void RHISetShaderConstantBuffer(EShaderType ShaderType, uint32 BufferIndex, XRHIConstantBuffer* RHIConstantBuffer) = 0;
	virtual void SetShaderValue(EShaderType ShaderType, uint32 BufferIndex, uint32 VariableOffsetInBuffer, uint32 NumBytes, const void* NewValue) = 0;

	//DrawCall/DisPatch
	virtual void RHIDispatchComputeShader(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ) = 0;
	virtual void RHIDrawIndexedPrimitive() = 0;

	//SetPSO
	virtual void RHISetGraphicsPipelineState(XRHIGraphicsPSO* GraphicsState) = 0;
	virtual void RHISetComputePipelineState(XRHIComputePSO* ComputeState) = 0;


	virtual void RHISetViewport(float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ) = 0;
	virtual void RHIBeginRenderPass(const XRHIRenderPassInfo& InInfo, const wchar_t* InName) = 0;
	virtual void SetRenderTargetsAndViewPort(uint32 NumRTs, const XRHIRenderTargetView* RTViews,const XRHIDepthStencilView* DSView) = 0;
	
};