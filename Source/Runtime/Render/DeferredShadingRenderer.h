#pragma once
#include "Runtime/RHI/RHICommandList.h"
#include "Runtime/Engine/SceneView.h"
#include "Runtime/RenderCore/VertexFactory.h"
#include "Runtime/Render/SceneRendering.h"
#include "PreDepthPassGPUCulling.h"
#include "SceneRenderTagrget.h"
#include "SkyAtmosPhere.h"

//Temps
#include "Runtime/Core/Mesh/GeomertyData.h"
//

class XDefaultVertexFactory :public XVertexFactory
{
public:
	void InitRHI()override;
	void ReleaseRHI()override;
};

class XDeferredShadingRenderer
{
public:
	void SceneTagetGen();

	void Setup();
	void Rendering(XRHICommandList& RHICmdList);
	
	//Pre Depth Pass GPU Culling
	void PreDepthPassGPUCullingSetup();

	void PreDepthPassGPUCulling(XRHICommandList& RHICmdList);
	void PreDepthPassRendering(XRHICommandList& RHICmdList);

	//Virtual Shadow Map Generate
	void VSMSetup();
	void VSMUpdate();

	void VSMTileMaskPass(XRHICommandList& RHICmdList);
	void VSMPageTableGen(XRHICommandList& RHICmdList);
	void VSMShadowCommandBuild(XRHICommandList& RHICmdList);
	void VirtualShadowMapGen(XRHICommandList& RHICmdList);

	//HZBPass
	void HZBPass(XRHICommandList& RHICmdList);

	//SkyAtmosPhere Rendering
	void SkyAtmosPhereUpdata(XRHICommandList& RHICmdList);
	void SkyAtmosPhereRendering(XRHICommandList& RHICmdList);

	//BasePass Rendering
	void BasePassRendering(XRHICommandList& RHICmdList);

	//Shadow Mask Generate
	void ShadowMaskGenerate(XRHICommandList& RHICmdList);
	void VSMTileMaskClear(XRHICommandList& RHICmdList);

	//LightPass
	void LightPass(XRHICommandList& RHICmdList);

	//SkyAtmoSphereCombine
	void SkyAtmoSphereCombine(XRHICommandList& RHICmdList);

	//PostProcessToneMapping
	void PostProcessToneMapping(XRHICommandList& RHICmdList, XRHITexture* TextureSceneColorSrc, XRHITexture* TextureSceneColorDest);


	inline std::shared_ptr<XRHITexture2D> TempGetTextureSceneColorDeffered()
	{
		return SceneTargets.TextureSceneColorDeffered;
	}

	inline std::shared_ptr<XRHITexture2D> TempGetTextureSceneColorDefferedPingPong()
	{
		return SceneTargets.TextureSceneColorDefferedPingPong;
	}
private:
	XPreDepthPassResource PreDepthPassResource;
	XSceneRenderTarget SceneTargets;
	XSkyAtmosphereParams cbSkyAtmosphereIns;
public:
	XMatrix LightViewMat;
	XMatrix LightProjMat;
	XMatrix LightViewProjMat;

	XBoundSphere SceneBoundingSphere;
	XVector3 ShadowLightDir;

	XVector3 MainLightColor;
	float LightIntensity;

	std::vector<std::shared_ptr<GGeomertry>>RenderGeos;
	std::shared_ptr<XRHIShaderResourceView>GlobalObjectStructBufferSRV;
	
	std::shared_ptr<XRHIConstantBuffer>cbCullingParameters;
	
	XDefaultVertexFactory DefaultVertexFactory;
	RendererViewInfo RViewInfo;
};