#pragma once
#include "RHIResource.h"
#include "RHICommandList.h"

extern void SetGraphicsPipelineStateFromPSOInit(XRHICommandList& RHICmdList, const XGraphicsPSOInitializer& OriginalInitializer);
extern void SetComputePipelineStateFromCS(XRHICommandList& RHICmdList, const XRHIComputeShader* CSShader);
namespace PipelineStateCache
{
	std::shared_ptr <XRHIComputePSO> GetAndOrCreateComputePipelineState(XRHICommandList& RHICmdList,const XRHIComputeShader* ComputeShader);
	std::shared_ptr <XRHIGraphicsPSO> GetAndOrCreateGraphicsPipelineState(XRHICommandList& RHICmdList, const XGraphicsPSOInitializer& OriginalInitializer);
}