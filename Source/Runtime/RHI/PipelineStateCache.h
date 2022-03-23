#pragma once
#include "RHIResource.h"
#include "RHICommandList.h"
extern void SetGraphicsPipelineState(XRHICommandList& RHICmdList, const XGraphicsPSOInitializer& OriginalInitializer);
namespace PipelineStateCache
{
	XRHIGraphicsPSO* GetAndOrCreateGraphicsPipelineState(XRHICommandList& RHICmdList, const XGraphicsPSOInitializer& OriginalInitializer);
	std::shared_ptr<XRHIVertexLayout> GetOrCreateVertexDeclaration(const XRHIVertexLayoutArray& Elements);
}