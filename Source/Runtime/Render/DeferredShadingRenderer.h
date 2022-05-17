#pragma once
#include "Runtime/RHI/RHICommandList.h"
#include "Runtime/Engine/SceneView.h"
#include "Runtime/RenderCore/VertexFactory.h"
#include "Runtime/Render/SceneRendering.h"

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
	void Setup();
	void Rendering(XRHICommandList& RHICmdList);
	
	void PreDepthPassGPUCullingSetup();
	void PreDepthPassGPUCulling(XRHICommandList& RHICmdList);
	void PreDepthPassRendering(XRHICommandList& RHICmdList);

	std::vector<std::shared_ptr<GGeomertry>>RenderGeos;
private:
	//PreDepthPassGPUCulling
	std::shared_ptr<XRHIStructBuffer> DepthCmdBufferCulled;//+
	uint64 DepthCounterOffset;//+

	std::shared_ptr<XRHICommandSignature> RHIDepthCommandSignature;//+
	std::shared_ptr<XRHIStructBuffer> DepthCmdBufferNoCulling;//+
	uint64 DepthCmdBufferOffset;//+

	std::shared_ptr<XRHIShaderResourceView>CmdBufferShaderResourceView;//+
	std::shared_ptr<XRHIUnorderedAcessView> CmdBufferUnorderedAcessView;//+
public:
	std::shared_ptr<XRHIShaderResourceView>GlobalObjectStructBufferSRV;
	
	std::shared_ptr<XRHIConstantBuffer>cbCullingParameters;
	std::shared_ptr<XRHITexture2D>TextureDepthStencil;
	XDefaultVertexFactory DefaultVertexFactory;
	RendererViewInfo RViewInfo;
};