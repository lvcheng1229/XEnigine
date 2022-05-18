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

XDeferredShadingRenderer::~XDeferredShadingRenderer()
{
	EditorUI.ImGui_Impl_RHI_Shutdown();
}

void XDeferredShadingRenderer::Setup()
{
	//Temp
	BeginInitResource(&DefaultVertexFactory);
	SceneTagetGen();
	PreDepthPassGPUCullingSetup();
	VSMSetup();

	EditorUI.SetDefaltStyle();
	EditorUI.InitIOInfo(RViewInfo.ViewWidth, RViewInfo.ViewHeight);
	EditorUI.ImGui_Impl_RHI_Init();
}

void XDeferredShadingRenderer::Rendering(XRHICommandList& RHICmdList)
{
	LightViewProjMat = LightViewMat * LightProjMat;
	SkyAtmosPhereUpdata(RHICmdList);


	RHICmdList.RHIBeginFrame();
	PreDepthPassGPUCulling(RHICmdList);
	PreDepthPassRendering(RHICmdList);

	VSMUpdate();
	VSMTileMaskPass(RHICmdList);
	VSMPageTableGen(RHICmdList);
	VSMShadowCommandBuild(RHICmdList);
	VirtualShadowMapGen(RHICmdList);

	HZBPass(RHICmdList);
	
	SkyAtmosPhereRendering(RHICmdList);

	BasePassRendering(RHICmdList);

	ShadowMaskGenerate(RHICmdList);
	VSMTileMaskClear(RHICmdList);

	LightPass(RHICmdList);

	SkyAtmoSphereCombine(RHICmdList);

	PostProcessToneMapping(RHICmdList, SceneTargets.TextureSceneColorDeffered.get(), SceneTargets.TextureSceneColorDefferedPingPong.get());
	TempUIRenderer(RHICmdList, SceneTargets.TextureSceneColorDefferedPingPong.get());
	PresentPass(RHICmdList, SceneTargets.TextureSceneColorDefferedPingPong.get());

	RHICmdList.RHIEndFrame();
}


