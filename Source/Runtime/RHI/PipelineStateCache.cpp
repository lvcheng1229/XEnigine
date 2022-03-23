#include "PipelineStateCache.h"

void SetGraphicsPipelineState(XRHICommandList& RHICmdList, const XGraphicsPSOInitializer& Initializer)
{
	XRHIGraphicsPSO* RHIGraphicsPSO = PipelineStateCache::GetAndOrCreateGraphicsPipelineState(RHICmdList, Initializer);
	RHICmdList.SetGraphicsPipelineState(PipelineState, Initializer.BoundShaderState);
}
namespace PipelineStateCache
{
	XRHIGraphicsPSO* GetAndOrCreateGraphicsPipelineState(XRHICommandList& RHICmdList, const XGraphicsPSOInitializer& OriginalInitializer)
	{
		const XGraphicsPSOInitializer* Initializer = &OriginalInitializer;
		bool bFound = false;
		if (bFound == false)
		{

		}
	}

	std::shared_ptr<XRHIVertexLayout> GetOrCreateVertexDeclaration(const XRHIVertexLayoutArray& Elements)
	{

		return std::shared_ptr<XRHIVertexLayout>();
	}
}

