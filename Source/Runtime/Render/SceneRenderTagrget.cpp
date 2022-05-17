#include "DeferredShadingRenderer.h"
#include "VirtualShadowMap.h"

void XDeferredShadingRenderer::SceneTagetGen()
{
	SceneTargets.PhysicalShadowDepthTexture = RHICreateTexture2D(PhysicalShadowDepthTextureSize, PhysicalShadowDepthTextureSize, 1, false, false,
		EPixelFormat::FT_R32_UINT, ETextureCreateFlags(TexCreate_UAV | TexCreate_ShaderResource), 1, nullptr);

	SceneTargets.PagetableInfos = RHICreateTexture2D(VirtualTileWidthNum, VirtualTileWidthNum, 1, false, false, EPixelFormat::FT_R32G32B32A32_UINT
		, ETextureCreateFlags(TexCreate_UAV | TexCreate_ShaderResource), 1, nullptr);
	
	SceneTargets.TextureDepthStencil = RHICreateTexture2D(RViewInfo.ViewWidth, RViewInfo.ViewHeight, 1, false, false,
		EPixelFormat::FT_R24G8_TYPELESS, ETextureCreateFlags(TexCreate_DepthStencilTargetable | TexCreate_ShaderResource)
		, 1, nullptr);

	SceneTargets.FurthestHZBOutput = RHICreateTexture2D(512, 512, 1, false, false, EPixelFormat::FT_R16_FLOAT
		, ETextureCreateFlags(TexCreate_UAV | TexCreate_ShaderResource), 5, nullptr);
}

