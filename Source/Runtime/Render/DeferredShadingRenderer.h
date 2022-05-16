#pragma once
#include "Runtime/RHI/RHICommandList.h"

class XDeferredShadingRenderer
{
public:
	void Rendering(XRHICommandList& RHICmdList);

	void PreDepthSetup();
	void PreDepthRendering(XRHICommandList& RHICmdList);
};