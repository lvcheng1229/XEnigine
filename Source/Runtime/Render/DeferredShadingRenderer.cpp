#pragma once
#include "DeferredShadingRenderer.h"

void XDefaultVertexFactory::InitRHI()
{
	XRHIVertexLayoutArray LayoutArray;
	LayoutArray.push_back(XVertexElement(0, EVertexElementType::VET_Float4, 0, 0));
	LayoutArray.push_back(XVertexElement(1, EVertexElementType::VET_Float3, 0, 0 + sizeof(XVector4)));
	LayoutArray.push_back(XVertexElement(2, EVertexElementType::VET_Float4, 0, 0 + sizeof(XVector4) + sizeof(XVector3)));
	LayoutArray.push_back(XVertexElement(3, EVertexElementType::VET_Float2, 0, 0 + sizeof(XVector4) + sizeof(XVector3) + sizeof(XVector4)));

	InitLayout(LayoutArray, ELayoutType::Layout_Default);
}

void XDefaultVertexFactory::ReleaseRHI()
{
	DefaultLayout.reset();
}

void XDeferredShadingRenderer::Setup()
{
	//Temp
	BeginInitResource(&DefaultVertexFactory);

	PreDepthPassGPUCullingSetup();
}

void XDeferredShadingRenderer::Rendering(XRHICommandList& RHICmdList)
{
	RHICmdList.RHIBeginFrame();
	PreDepthPassGPUCulling(RHICmdList);
	PreDepthPassRendering(RHICmdList);
}


