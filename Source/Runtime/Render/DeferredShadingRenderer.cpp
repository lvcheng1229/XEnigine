#pragma once
#include "DeferredShadingRenderer.h"

void XDeferredShadingRenderer::Setup()
{
	PreDepthPassGPUCullingSetup();
}

void XDeferredShadingRenderer::Rendering(XRHICommandList& RHICmdList)
{
	RHICmdList.RHIBeginFrame();

	PreDepthPassGPUCullingRendering(RHICmdList);
}


