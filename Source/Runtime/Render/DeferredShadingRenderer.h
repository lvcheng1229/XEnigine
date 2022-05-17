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

	//Shadow Mask Generate
	


	std::shared_ptr <XRHITexture2D> TempGetVirtualSMFlags();
	std::shared_ptr<XRHITexture2D> TempGetPagetableInfos();
	std::shared_ptr<XRHIConstantBuffer> TempGetVSMTileMaskConstantBuffer();
	uint32 TempGetShadowCmdBufferOffset();
	uint32 TempGetShadowCounterOffset();
	std::shared_ptr<XRHIStructBuffer> TempGetShadowCmdBufferCulled();
	std::shared_ptr<XRHITexture2D> TempGetPhysicalDepth();
	inline std::shared_ptr <XRHITexture2D> TempGetTextureDepthStencil()
	{
		return SceneTargets.TextureDepthStencil;
	}

	std::shared_ptr<XRHITexture2D> TempGetTransmittanceLutUAV();
	std::shared_ptr<XRHITexture2D> TempGetSkyViewLutUAV();
	std::shared_ptr<XRHIConstantBuffer> TempGetRHICbSkyAtmosphere();
private:
	XPreDepthPassResource PreDepthPassResource;
	XSceneRenderTarget SceneTargets;
	XSkyAtmosphereParams cbSkyAtmosphereIns;
	//std::shared_ptr <XRHITexture2D> PhysicalShadowDepthTexture;//++
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