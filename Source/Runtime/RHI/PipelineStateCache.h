#pragma once
#include "RHIResource.h"
#include "RHICommandList.h"
extern void SetGraphicsPipelineStateFromPSOInit(XRHICommandList& RHICmdList, const XGraphicsPSOInitializer& OriginalInitializer);
namespace PipelineStateCache
{
	std::shared_ptr <XRHIGraphicsPSO> GetAndOrCreateGraphicsPipelineState(XRHICommandList& RHICmdList, const XGraphicsPSOInitializer& OriginalInitializer);
	//std::shared_ptr<XRHIVertexLayout> GetOrCreateVertexDeclaration(const XRHIVertexLayoutArray& Elements);
}