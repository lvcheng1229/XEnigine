#pragma once
#include "Runtime/RHI/RHICommandList.h"

class XDeferredShadingRenderer
{
public:
	void Setup();
	void Rendering(XRHICommandList& RHICmdList);
	
	void PreDepthPassGPUCullingSetup();
	void PreDepthPassGPUCullingRendering(XRHICommandList& RHICmdList);

	void PreDepthPassSetup();
	void PreDepthPassRendering(XRHICommandList& RHICmdList);

private:

	//PreDepthPassGPUCulling
	std::shared_ptr<XRHICommandSignature> RHIDepthCommandSignature;
	std::shared_ptr<XRHIStructBuffer> DepthCmdBufferNoCulling;
	std::shared_ptr<XRHIStructBuffer> DepthCmdBufferCulled;
	uint64 DepthCmdBufferOffset;
	uint64 DepthCounterOffset;
};