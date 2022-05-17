//***************************************************************************************
// CrateApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include <memory>
#include <windows.h>
#include "UnitTest/d3dApp.h"


#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb_image.h"

#include "Runtime/RenderCore/GlobalShader.h"
#include "Runtime/RenderCore/ShaderParameter.h"
#include "Runtime/RenderCore/CommonRenderRresource.h"
#include "Runtime/RenderCore/Shader.h"
#include "Runtime/RenderCore/VertexFactory.h"

#include "Runtime/Core/XMath.h"
#include "Runtime/Render/SceneRendering.h"
#include "Runtime/Render/MeshPassProcessor.h"
#include "Runtime/Render/MaterialShader.h"
#include "Runtime/Render/BasePassRendering.h"

#include "Runtime/Engine/SceneView.h"
#include "Runtime/Engine/ShaderCompiler/ShaderCompiler.h"
#include "Runtime/Engine/Material/MaterialShared.h"

#include "Runtime/RHI/PipelineStateCache.h"
#include "Runtime/RHI/RHIStaticStates.h"

#include "Runtime/D3D12RHI/D3D12Shader.h"
#include "Runtime/D3D12RHI/D3D12OnlineDescArray.h"
#include "Runtime/D3D12RHI/D3D12OnlineDescArrayManager.h"
#include "Runtime/D3D12RHI/D3D12PlatformRHI.h"
#include "Runtime/D3D12RHI/D3D12Texture.h"

#include "Runtime/Core/Math/Math.h"


//ImGUI Begin
#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>

#include "Editor/EditorUI.h"
//#include "Runtime/Engine/UIBackend.h"
//ImGUI End


#include "Runtime/Core/MainInit.h"
#include "Runtime/Engine/Classes/Texture.h"
#include "Runtime/HAL/FileManagerGeneric.h"
#include "Runtime/Core/Misc/Path.h"

#include "Runtime/Core/ComponentNode/Camera.h"

#include "Runtime/Engine/Classes/Material.h"
#include "Runtime/Engine/ResourcecConverter.h"

#include "Runtime/Render/DeferredShadingRenderer.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

class XLocalVertexFactory :public XVertexFactory
{
public:
	void InitRHI()override;
	void ReleaseRHI()override;
};

void XLocalVertexFactory::InitRHI()
{
	XRHIVertexLayoutArray LayoutArray;
	LayoutArray.push_back(XVertexElement(0, EVertexElementType::VET_Float4, 0, 0));
	LayoutArray.push_back(XVertexElement(1, EVertexElementType::VET_Float3, 0, 0 + sizeof(XVector4)));
	LayoutArray.push_back(XVertexElement(2, EVertexElementType::VET_Float4, 0, 0 + sizeof(XVector4) + sizeof(XVector3)));
	LayoutArray.push_back(XVertexElement(3, EVertexElementType::VET_Float2, 0, 0 + sizeof(XVector4) + sizeof(XVector3) + sizeof(XVector4)));

	InitLayout(LayoutArray, ELayoutType::Layout_Default);
}

void XLocalVertexFactory::ReleaseRHI()
{
	DefaultLayout.reset();
}


template<>
TBasePassVS<false>::ShaderInfos TBasePassVS<false>::StaticShaderInfos(
	"TBasePassVS<false>", L"E:/XEngine/XEnigine/Source/Shaders/BasePassVertexShader.hlsl",
	"VS", EShaderType::SV_Vertex, TBasePassVS<false>::CustomConstrucFunc,
	TBasePassVS<false>::ModifyShaderCompileSettings);

template<>
TBasePassPS<false>::ShaderInfos TBasePassPS<false>::StaticShaderInfos(
	"TBasePassPS<false>", L"E:/XEngine/XEnigine/Source/Shaders/BasePassPixelShader_1.hlsl",
	"PS", EShaderType::SV_Pixel, TBasePassPS<false>::CustomConstrucFunc,
	TBasePassPS<false>::ModifyShaderCompileSettings);




//ToneMapping PS
class XToneMappingPassPS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XToneMappingPassPS(Initializer);
	}

	static ShaderInfos StaticShaderInfos;

	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}

public:
	XToneMappingPassPS(const XShaderInitlizer& Initializer)
		:XGloablShader(Initializer)
	{
		FullScreenMap.Bind(Initializer.ShaderParameterMap, "FullScreenMap");
	}

	void SetParameter(
		XRHICommandList& RHICommandList,
		XRHITexture* InTexture)
	{
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, FullScreenMap, InTexture);
	}
	TextureParameterType    FullScreenMap;
};
XToneMappingPassPS::ShaderInfos XToneMappingPassPS::StaticShaderInfos(
	"XToneMappingPassPS", L"E:/XEngine/XEnigine/Source/Shaders/ToneMapping.hlsl",
	"ToneMapping_PS", EShaderType::SV_Pixel, XToneMappingPassPS::CustomConstrucFunc,
	XToneMappingPassPS::ModifyShaderCompileSettings);

class XFinalPassPS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XFinalPassPS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	XFinalPassPS(const XShaderInitlizer& Initializer)
		:XGloablShader(Initializer) 
	{
		FullScreenMap.Bind(Initializer.ShaderParameterMap, "FullScreenMap");
	}

	void SetParameter(
		XRHICommandList& RHICommandList,
		XRHITexture* InTexture)
	{
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, FullScreenMap, InTexture);
	}
	TextureParameterType    FullScreenMap;
};
XFinalPassPS::ShaderInfos XFinalPassPS::StaticShaderInfos(
	"XFinalPassPS", L"E:/XEngine/XEnigine/Source/Shaders/FinalPassShader.hlsl",
	"PS", EShaderType::SV_Pixel, XFinalPassPS::CustomConstrucFunc,
	XFinalPassPS::ModifyShaderCompileSettings);







//XLightPass
class XLightPassVS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XLightPassVS(Initializer);
	}

	static ShaderInfos StaticShaderInfos;

	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}

public:
	XLightPassVS(const XShaderInitlizer& Initializer)
		:XGloablShader(Initializer)
	{
		CBV_View.Bind(Initializer.ShaderParameterMap, "cbView");
	}

	void SetParameter(
		XRHICommandList& RHICommandList,
		XRHIConstantBuffer* ViewCB)
	{
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Vertex, CBV_View, ViewCB);
	}
	CBVParameterType CBV_View;
};

class XLightPassPS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XLightPassPS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}

public:
	XLightPassPS(const XShaderInitlizer& Initializer)
		:XGloablShader(Initializer)
	{
		CBV_View.Bind(Initializer.ShaderParameterMap, "cbView");
		CBV_DefferedLight.Bind(Initializer.ShaderParameterMap, "cbLightPass");

		GBufferATexture.Bind(Initializer.ShaderParameterMap, "SceneTexturesStruct_GBufferATexture");
		GBufferBTexture.Bind(Initializer.ShaderParameterMap, "SceneTexturesStruct_GBufferBTexture");
		GBufferCTexture.Bind(Initializer.ShaderParameterMap, "SceneTexturesStruct_GBufferCTexture");
		GBufferDTexture.Bind(Initializer.ShaderParameterMap, "SceneTexturesStruct_GBufferDTexture");
		SceneDepthTexture.Bind(Initializer.ShaderParameterMap, "SceneTexturesStruct_SceneDepthTexture");
		ShadowMakTex.Bind(Initializer.ShaderParameterMap, "ShadowMaskTex");
	}

	void SetParameter(
		XRHICommandList& RHICommandList,
		XRHIConstantBuffer* InCBV_View,
		XRHIConstantBuffer* InCBV_DefferedLight,

		XRHITexture* InGBufferATexture,
		XRHITexture* InGBufferBTexture,
		XRHITexture* InGBufferCTexture,
		XRHITexture* InGBufferDTexture,
		XRHITexture* InSceneDepthTexture,
		XRHITexture* InLightAttenuationTexture)
	{
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Pixel, CBV_View, InCBV_View);
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Pixel, CBV_DefferedLight, InCBV_DefferedLight);

		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, GBufferATexture, InGBufferATexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, GBufferBTexture, InGBufferBTexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, GBufferCTexture, InGBufferCTexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, GBufferDTexture, InGBufferDTexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, SceneDepthTexture, InSceneDepthTexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, ShadowMakTex, InLightAttenuationTexture);
	}

	CBVParameterType CBV_View;
	CBVParameterType CBV_DefferedLight;

	TextureParameterType GBufferATexture;
	TextureParameterType GBufferBTexture;
	TextureParameterType GBufferCTexture;
	TextureParameterType GBufferDTexture;
	TextureParameterType SceneDepthTexture;
	TextureParameterType ShadowMakTex;

};

XLightPassVS::ShaderInfos XLightPassVS::StaticShaderInfos(
	"XLightPassVS", L"E:/XEngine/XEnigine/Source/Shaders/DeferredLightVertexShaders.hlsl",
	"DeferredLightVertexMain", EShaderType::SV_Vertex, XLightPassVS::CustomConstrucFunc,
	XLightPassVS::ModifyShaderCompileSettings);
XLightPassPS::ShaderInfos XLightPassPS::StaticShaderInfos(
	"XLightPassPS", L"E:/XEngine/XEnigine/Source/Shaders/DeferredLightPixelShaders.hlsl",
	"DeferredLightPixelMain", EShaderType::SV_Pixel, XLightPassPS::CustomConstrucFunc,
	XLightPassPS::ModifyShaderCompileSettings);


//XHZBPassCS
//class XHZBPassCS :public XGloablShader
//{
//public:
//	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
//	{
//		return new XHZBPassCS(Initializer);
//	}
//	static ShaderInfos StaticShaderInfos;
//	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
//public:
//	XHZBPassCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
//	{
//		DispatchThreadIdToBufferUV.Bind(Initializer.ShaderParameterMap, "DispatchThreadIdToBufferUV");
//
//		TextureSampledInput.Bind(Initializer.ShaderParameterMap, "TextureSampledInput");
//		FurthestHZBOutput_0.Bind(Initializer.ShaderParameterMap, "FurthestHZBOutput_0");
//		FurthestHZBOutput_1.Bind(Initializer.ShaderParameterMap, "FurthestHZBOutput_1");
//		FurthestHZBOutput_2.Bind(Initializer.ShaderParameterMap, "FurthestHZBOutput_2");
//		FurthestHZBOutput_3.Bind(Initializer.ShaderParameterMap, "FurthestHZBOutput_3");
//		FurthestHZBOutput_4.Bind(Initializer.ShaderParameterMap, "FurthestHZBOutput_4");
//
//		//VirtualSMFlags.Bind(Initializer.ShaderParameterMap, "VirtualSMFlags");
//	}
//
//	void SetParameters(
//		XRHICommandList& RHICommandList,
//		XVector4 InDispatchThreadIdToBufferUV,
//
//		XRHITexture* InTextureSampledInput,
//		XRHIUnorderedAcessView* InFurthestHZBOutput_0,
//		XRHIUnorderedAcessView* InFurthestHZBOutput_1,
//		XRHIUnorderedAcessView* InFurthestHZBOutput_2,
//		XRHIUnorderedAcessView* InFurthestHZBOutput_3,
//		XRHIUnorderedAcessView* InFurthestHZBOutput_4//,
//
//		//XRHIUnorderedAcessView* VirtualSMFlagsIn
//	)
//	{
//		SetShaderValue(RHICommandList, EShaderType::SV_Compute, DispatchThreadIdToBufferUV, InDispatchThreadIdToBufferUV);
//
//		SetTextureParameter(RHICommandList, EShaderType::SV_Compute, TextureSampledInput, InTextureSampledInput);
//		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, FurthestHZBOutput_0, InFurthestHZBOutput_0);
//		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, FurthestHZBOutput_1, InFurthestHZBOutput_1);
//		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, FurthestHZBOutput_2, InFurthestHZBOutput_2);
//		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, FurthestHZBOutput_3, InFurthestHZBOutput_3);
//		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, FurthestHZBOutput_4, InFurthestHZBOutput_4);
//		
//		//SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, VirtualSMFlags, VirtualSMFlagsIn);
//	}
//
//	XShaderVariableParameter DispatchThreadIdToBufferUV;
//
//	UAVParameterType TextureSampledInput;
//	UAVParameterType FurthestHZBOutput_0;
//	UAVParameterType FurthestHZBOutput_1;
//	UAVParameterType FurthestHZBOutput_2;
//	UAVParameterType FurthestHZBOutput_3;
//	UAVParameterType FurthestHZBOutput_4;
//
//	//UAVParameterType VirtualSMFlags;
//};
//
//XHZBPassCS::ShaderInfos XHZBPassCS::StaticShaderInfos(
//	"HZBBuildCS", L"E:/XEngine/XEnigine/Source/Shaders/HZB.hlsl",
//	"HZBBuildCS", EShaderType::SV_Compute, XHZBPassCS::CustomConstrucFunc,
//	XHZBPassCS::ModifyShaderCompileSettings);


class VSMTileMaskClearCS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new VSMTileMaskClearCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	VSMTileMaskClearCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		VirtualSMFlags.Bind(Initializer.ShaderParameterMap, "VirtualSMFlags");
		PhysicalShadowDepthTexture.Bind(Initializer.ShaderParameterMap, "PhysicalShadowDepthTexture");
	}

	void SetParameters(
		XRHICommandList& RHICommandList,
		XRHIUnorderedAcessView* VirtualSMFlagsIn,
		XRHIUnorderedAcessView* PhysicalShadowDepthTextureIn)
	{
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, VirtualSMFlags, VirtualSMFlagsIn);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, PhysicalShadowDepthTexture, PhysicalShadowDepthTextureIn);
	}
	UAVParameterType VirtualSMFlags;
	UAVParameterType PhysicalShadowDepthTexture;
};
VSMTileMaskClearCS::ShaderInfos VSMTileMaskClearCS::StaticShaderInfos(
	"VSMTileMaskClearCS", L"E:/XEngine/XEnigine/Source/Shaders/VSMTileMaskClearCS.hlsl",
	"VSMTileMaskClearCS", EShaderType::SV_Compute, VSMTileMaskClearCS::CustomConstrucFunc,
	VSMTileMaskClearCS::ModifyShaderCompileSettings);


class ShadowMaskGenCS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new ShadowMaskGenCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	ShadowMaskGenCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		cbView.Bind(Initializer.ShaderParameterMap, "cbView");
		cbShadowViewInfo.Bind(Initializer.ShaderParameterMap, "cbShadowViewInfo");
		
		VsmShadowMaskTex.Bind(Initializer.ShaderParameterMap, "VsmShadowMaskTex");
		
		GBufferNormal.Bind(Initializer.ShaderParameterMap, "GBufferNormal");
		SceneDepthInput.Bind(Initializer.ShaderParameterMap, "SceneDepthInput");
		PagetableInfos.Bind(Initializer.ShaderParameterMap, "PagetableInfos");
		PhysicalShadowDepthTexture.Bind(Initializer.ShaderParameterMap, "PhysicalShadowDepthTexture");
	}

	void SetParameters(
		XRHICommandList& RHICommandList,
		XRHIConstantBuffer* IncbView,
		XRHIConstantBuffer* IncbShadowViewInfo,
		
		XRHIUnorderedAcessView* InVsmShadowMaskTex,

		XRHIShaderResourceView* InGBufferNormal,
		XRHIShaderResourceView* InSceneDepthInput,
		XRHIShaderResourceView* InPagetableInfos,
		XRHIShaderResourceView* InPhysicalShadowDepthTexture)
	{
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbView, IncbView);
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbShadowViewInfo, IncbShadowViewInfo);

		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, VsmShadowMaskTex, InVsmShadowMaskTex);

		SetShaderSRVParameter(RHICommandList, EShaderType::SV_Compute, GBufferNormal, InGBufferNormal);
		SetShaderSRVParameter(RHICommandList, EShaderType::SV_Compute, SceneDepthInput, InSceneDepthInput);
		SetShaderSRVParameter(RHICommandList, EShaderType::SV_Compute, PagetableInfos, InPagetableInfos);
		SetShaderSRVParameter(RHICommandList, EShaderType::SV_Compute, PhysicalShadowDepthTexture, InPhysicalShadowDepthTexture);
	}

	CBVParameterType cbView;
	CBVParameterType cbShadowViewInfo;

	UAVParameterType VsmShadowMaskTex;

	SRVParameterType GBufferNormal;
	SRVParameterType SceneDepthInput;
	SRVParameterType PagetableInfos;
	SRVParameterType PhysicalShadowDepthTexture;
};
ShadowMaskGenCS::ShaderInfos ShadowMaskGenCS::StaticShaderInfos(
	"ShadowMaskGenCS", L"E:/XEngine/XEnigine/Source/Shaders/ShadowMaskGenCS.hlsl",
	"ShadowMaskGenCS", EShaderType::SV_Compute, ShadowMaskGenCS::CustomConstrucFunc,
	ShadowMaskGenCS::ModifyShaderCompileSettings);


//XRenderTransmittanceLutCS
//class XRenderTransmittanceLutCS :public XGloablShader
//{
//public:
//	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
//	{
//		return new XRenderTransmittanceLutCS(Initializer);
//	}
//	static ShaderInfos StaticShaderInfos;
//
//	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings)
//	{
//		OutSettings.SetDefines("THREADGROUP_SIZE", "8");
//	}
//public:
//
//	XRenderTransmittanceLutCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
//	{
//		TransmittanceLutUAV.Bind(Initializer.ShaderParameterMap, "TransmittanceLutUAV");
//		cbSkyAtmosphere.Bind(Initializer.ShaderParameterMap, "SkyAtmosphere");
//	}
//
//	void SetParameters(
//		XRHICommandList& RHICommandList,
//		XRHIUnorderedAcessView* InUAV,
//		XRHIConstantBuffer* IncbSkyAtmosphere)
//	{
//		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, TransmittanceLutUAV, InUAV);
//		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbSkyAtmosphere, IncbSkyAtmosphere);
//	}
//
//	CBVParameterType cbSkyAtmosphere;
//	TextureParameterType TransmittanceLutUAV;
//};
//
//XRenderTransmittanceLutCS::ShaderInfos XRenderTransmittanceLutCS::StaticShaderInfos(
//	"RenderTransmittanceLutCS", L"E:/XEngine/XEnigine/Source/Shaders/SkyAtmosphere.hlsl",
//	"RenderTransmittanceLutCS", EShaderType::SV_Compute, XRenderTransmittanceLutCS::CustomConstrucFunc,
//	XRenderTransmittanceLutCS::ModifyShaderCompileSettings);


//XRenderMultiScatteredLuminanceLutCS
//class XRenderMultiScatteredLuminanceLutCS : public XGloablShader
//{
//public:
//	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
//	{
//		return new XRenderMultiScatteredLuminanceLutCS(Initializer);
//	}
//	static ShaderInfos StaticShaderInfos;
//
//	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings)
//	{
//		OutSettings.SetDefines("THREADGROUP_SIZE", "8");
//	}
//public:
//	XRenderMultiScatteredLuminanceLutCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
//	{
//		MultiScatteredLuminanceLutUAV.Bind(Initializer.ShaderParameterMap, "MultiScatteredLuminanceLutUAV");
//		TransmittanceLutTexture.Bind(Initializer.ShaderParameterMap, "TransmittanceLutTexture");
//		cbSkyAtmosphere.Bind(Initializer.ShaderParameterMap, "SkyAtmosphere");
//	}
//
//	void SetParameters(
//		XRHICommandList& RHICommandList,
//		XRHIUnorderedAcessView* InUAV,
//		XRHIConstantBuffer* IncbSkyAtmosphere,
//		XRHITexture* InTransmittanceLutTexture)
//	{
//		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, MultiScatteredLuminanceLutUAV, InUAV);
//		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbSkyAtmosphere, IncbSkyAtmosphere);
//		SetTextureParameter(RHICommandList, EShaderType::SV_Compute, TransmittanceLutTexture, InTransmittanceLutTexture);
//	}
//
//	UAVParameterType MultiScatteredLuminanceLutUAV;
//	CBVParameterType cbSkyAtmosphere;
//	TextureParameterType TransmittanceLutTexture;
//};
//
//XRenderMultiScatteredLuminanceLutCS::ShaderInfos XRenderMultiScatteredLuminanceLutCS::StaticShaderInfos(
//	"RenderMultiScatteredLuminanceLutCS", L"E:/XEngine/XEnigine/Source/Shaders/SkyAtmosphere.hlsl",
//	"RenderMultiScatteredLuminanceLutCS", EShaderType::SV_Compute, XRenderMultiScatteredLuminanceLutCS::CustomConstrucFunc,
//	XRenderMultiScatteredLuminanceLutCS::ModifyShaderCompileSettings);


//XRenderSkyViewLutCS
//class XRenderSkyViewLutCS :public XGloablShader
//{
//public:
//	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
//	{
//		return new XRenderSkyViewLutCS(Initializer);
//	}
//	static ShaderInfos StaticShaderInfos;
//
//	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings)
//	{
//		OutSettings.SetDefines("THREADGROUP_SIZE", "8");
//	}
//public:
//	XRenderSkyViewLutCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
//	{
//		cbView.Bind(Initializer.ShaderParameterMap, "cbView");
//		cbSkyAtmosphere.Bind(Initializer.ShaderParameterMap, "SkyAtmosphere");
//
//		SkyViewLutUAV.Bind(Initializer.ShaderParameterMap, "SkyViewLutUAV");
//
//		MultiScatteredLuminanceLutTexture.Bind(Initializer.ShaderParameterMap, "MultiScatteredLuminanceLutTexture");
//		TransmittanceLutTexture.Bind(Initializer.ShaderParameterMap, "TransmittanceLutTexture");
//	}
//
//	void SetParameters(
//		XRHICommandList& RHICommandList,
//		XRHIConstantBuffer* IncbView,
//		XRHIConstantBuffer* IncbSkyAtmosphere,
//
//		XRHIUnorderedAcessView* InSkyViewLutUAV,
//		XRHITexture* InTransmittanceLutTexture,
//		XRHITexture* InMultiScatteredLuminanceLutTexture
//	)
//	{
//		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbView, IncbView);
//		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbSkyAtmosphere, IncbSkyAtmosphere);
//
//		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, SkyViewLutUAV, InSkyViewLutUAV);
//
//		SetTextureParameter(RHICommandList, EShaderType::SV_Compute, TransmittanceLutTexture, InTransmittanceLutTexture);
//		SetTextureParameter(RHICommandList, EShaderType::SV_Compute, MultiScatteredLuminanceLutTexture, InMultiScatteredLuminanceLutTexture);
//	}
//
//	CBVParameterType cbView;
//	CBVParameterType cbSkyAtmosphere;
//
//	UAVParameterType SkyViewLutUAV;
//
//	TextureParameterType MultiScatteredLuminanceLutTexture;
//	TextureParameterType TransmittanceLutTexture;
//};
//
//XRenderSkyViewLutCS::ShaderInfos XRenderSkyViewLutCS::StaticShaderInfos(
//	"RenderSkyViewLutCS", L"E:/XEngine/XEnigine/Source/Shaders/SkyAtmosphere.hlsl",
//	"RenderSkyViewLutCS", EShaderType::SV_Compute, XRenderSkyViewLutCS::CustomConstrucFunc,
//	XRenderSkyViewLutCS::ModifyShaderCompileSettings);




//XRenderCameraAerialPerspectiveVolumeCS
class XRenderCameraAerialPerspectiveVolumeCS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XRenderCameraAerialPerspectiveVolumeCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;

	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings)
	{
		OutSettings.SetDefines("THREADGROUP_SIZE", "8");
	}
public:
	XRenderCameraAerialPerspectiveVolumeCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		cbView.Bind(Initializer.ShaderParameterMap, "cbView");
		cbSkyAtmosphere.Bind(Initializer.ShaderParameterMap, "SkyAtmosphere");

		CameraAerialPerspectiveVolume.Bind(Initializer.ShaderParameterMap, "CameraAerialPerspectiveVolumeUAV");

		MultiScatteredLuminanceLutTexture.Bind(Initializer.ShaderParameterMap, "MultiScatteredLuminanceLutTexture");
		TransmittanceLutTexture.Bind(Initializer.ShaderParameterMap, "TransmittanceLutTexture");
	}

	void SetParameters(
		XRHICommandList& RHICommandList,
		XRHIConstantBuffer* IncbView,
		XRHIConstantBuffer* IncbSkyAtmosphere,

		XRHIUnorderedAcessView* InCameraAerialPerspectiveVolumeUAV,

		XRHITexture* InTransmittanceLutTexture,
		XRHITexture* InMultiScatteredLuminanceLutTexture
	)
	{
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbView, IncbView);
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbSkyAtmosphere, IncbSkyAtmosphere);

		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, CameraAerialPerspectiveVolume, InCameraAerialPerspectiveVolumeUAV);

		SetTextureParameter(RHICommandList, EShaderType::SV_Compute, TransmittanceLutTexture, InTransmittanceLutTexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Compute, MultiScatteredLuminanceLutTexture, InMultiScatteredLuminanceLutTexture);
	}

	CBVParameterType cbView;
	CBVParameterType cbSkyAtmosphere;

	UAVParameterType CameraAerialPerspectiveVolume;

	TextureParameterType MultiScatteredLuminanceLutTexture;
	TextureParameterType TransmittanceLutTexture;
};

XRenderCameraAerialPerspectiveVolumeCS::ShaderInfos XRenderCameraAerialPerspectiveVolumeCS::StaticShaderInfos(
	"RenderCameraAerialPerspectiveVolumeCS", L"E:/XEngine/XEnigine/Source/Shaders/SkyAtmosphere.hlsl",
	"RenderCameraAerialPerspectiveVolumeCS", EShaderType::SV_Compute, XRenderCameraAerialPerspectiveVolumeCS::CustomConstrucFunc,
	XRenderCameraAerialPerspectiveVolumeCS::ModifyShaderCompileSettings);


class XSSRPassPS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XSSRPassPS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}

public:
	XSSRPassPS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{

		SSRParams.Bind(Initializer.ShaderParameterMap, "cbSSR_SSRParams");
		CBV_View.Bind(Initializer.ShaderParameterMap, "cbView");

		SceneColor.Bind(Initializer.ShaderParameterMap, "SceneColor");
		GBufferATexture.Bind(Initializer.ShaderParameterMap, "SceneTexturesStruct_GBufferATexture");
		GBufferBTexture.Bind(Initializer.ShaderParameterMap, "SceneTexturesStruct_GBufferBTexture");
		GBufferCTexture.Bind(Initializer.ShaderParameterMap, "SceneTexturesStruct_GBufferCTexture");
		GBufferDTexture.Bind(Initializer.ShaderParameterMap, "SceneTexturesStruct_GBufferDTexture");
		SceneDepthTexture.Bind(Initializer.ShaderParameterMap, "SceneTexturesStruct_SceneDepthTexture");
		HZBTexture.Bind(Initializer.ShaderParameterMap, "HZBTexture");
	}

	void SetParameter(
		XRHICommandList& RHICommandList,
		DirectX::XMFLOAT4 InSSRParams,
		XRHIConstantBuffer* ViewCB,
		XRHITexture* InSceneColor,
		XRHITexture* InGBufferATexture,
		XRHITexture* InGBufferBTexture,
		XRHITexture* InGBufferCTexture,
		XRHITexture* InGBufferDTexture,
		XRHITexture* InSceneDepthTexture,
		XRHITexture* InHZBTexture)
	{
		SetShaderValue(RHICommandList, EShaderType::SV_Pixel, SSRParams, InSSRParams);
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Pixel, CBV_View, ViewCB);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, SceneColor, InSceneColor);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, GBufferATexture, InGBufferATexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, GBufferBTexture, InGBufferBTexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, GBufferCTexture, InGBufferCTexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, GBufferDTexture, InGBufferDTexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, SceneDepthTexture, InSceneDepthTexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, HZBTexture, InHZBTexture);
	}


	XShaderVariableParameter SSRParams;

	CBVParameterType CBV_View;

	TextureParameterType SceneColor;
	TextureParameterType GBufferATexture;
	TextureParameterType GBufferBTexture;
	TextureParameterType GBufferCTexture;
	TextureParameterType GBufferDTexture;
	TextureParameterType SceneDepthTexture;
	TextureParameterType HZBTexture;
};
XSSRPassPS::ShaderInfos XSSRPassPS::StaticShaderInfos(
	"XSSRPassPS", L"E:/XEngine/XEnigine/Source/Shaders/ScreenSpaceReflection.hlsl",
	"PS", EShaderType::SV_Pixel, XSSRPassPS::CustomConstrucFunc, XSSRPassPS::ModifyShaderCompileSettings);






class XReflectionEnvironmentPS : public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XReflectionEnvironmentPS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}

public:
	XReflectionEnvironmentPS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		SSRMap.Bind(Initializer.ShaderParameterMap, "SSRMap");
	}

	void SetParameter(
		XRHICommandList& RHICommandList,
		XRHITexture* InSSRMap)
	{
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, SSRMap, InSSRMap);
	}

	TextureParameterType SSRMap;
};
XReflectionEnvironmentPS::ShaderInfos XReflectionEnvironmentPS::StaticShaderInfos(
	"XReflectionEnvironmentPS", L"E:/XEngine/XEnigine/Source/Shaders/ReflectionEnvironmentShader.hlsl",
	"PS", EShaderType::SV_Pixel, XReflectionEnvironmentPS::CustomConstrucFunc,
	XReflectionEnvironmentPS::ModifyShaderCompileSettings);






class XRenderSkyAtmosphereRayMarchingPS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XRenderSkyAtmosphereRayMarchingPS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}

public:
	XRenderSkyAtmosphereRayMarchingPS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		cbView.Bind(Initializer.ShaderParameterMap, "cbView");
		SkyAtmosphere.Bind(Initializer.ShaderParameterMap, "SkyAtmosphere");

		SkyViewLutTexture.Bind(Initializer.ShaderParameterMap, "SkyViewLutTexture");
		SceneDepthTexture.Bind(Initializer.ShaderParameterMap, "SceneTexturesStruct_SceneDepthTexture");
		TransmittanceLutTexture_Combine.Bind(Initializer.ShaderParameterMap, "TransmittanceLutTexture_Combine");
	}

	void SetParameter(
		XRHICommandList& RHICommandList,
		XRHIConstantBuffer* IncbView,
		XRHIConstantBuffer* InSkyAtmosphere,
		XRHITexture* InSkyViewLutTexture,
		XRHITexture* InSceneDepthTexture,
		XRHITexture* InTransmittanceLutTexture_Combine
	)
	{
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Pixel, cbView, IncbView);
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Pixel, SkyAtmosphere, InSkyAtmosphere);

		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, SkyViewLutTexture, InSkyViewLutTexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, SceneDepthTexture, InSceneDepthTexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, TransmittanceLutTexture_Combine, InTransmittanceLutTexture_Combine);
	}

	CBVParameterType cbView;
	CBVParameterType SkyAtmosphere;

	TextureParameterType SkyViewLutTexture;
	TextureParameterType SceneDepthTexture;
	TextureParameterType TransmittanceLutTexture_Combine;
};
XRenderSkyAtmosphereRayMarchingPS::ShaderInfos XRenderSkyAtmosphereRayMarchingPS::StaticShaderInfos(
	"XRenderSkyAtmosphereRayMarchingPS", L"E:/XEngine/XEnigine/Source/Shaders/SkyAtmosphere.hlsl",
	"RenderSkyAtmosphereRayMarchingPS", EShaderType::SV_Pixel, XRenderSkyAtmosphereRayMarchingPS::CustomConstrucFunc,
	XRenderSkyAtmosphereRayMarchingPS::ModifyShaderCompileSettings);





struct BoundSphere
{
	XVector3 Center;
	float Radius;
};


class CrateApp : public D3DApp
{
public:
	CrateApp();
	CrateApp(const CrateApp& rhs) = delete;
	CrateApp& operator=(const CrateApp& rhs) = delete;
	~CrateApp();

	virtual bool Initialize()override;

private:
	void TempDelete()override;
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Renderer(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	void LoadTextures();
	void BuildPSOs();

	void TestExecute();
	void VirtualShadow();

	BoundSphere BoundSphere0;
	

	GCamera CamIns;
	RendererViewInfo RViewInfo;

	XVector3 LightDir = { -1 / sqrtf(3.0f),1/ sqrtf(3.0f),1/ sqrtf(3.0f) };
	XVector3 LightColor = { 1,1,1 };
	float LightIntensity = 7.0f;

	struct ObjectConstants
	{
		XMatrix World;
		DirectX::XMFLOAT3 BoundingBoxCenter;
		float padding0 = 1.0f;
		DirectX::XMFLOAT3 BoundingBoxExtent;
		float padding1 = 1.0f;
	};

	XDeferredShadingRenderer DeferredShadingRenderer;
private:
	//GPU Driven New
	std::shared_ptr<XRHICommandSignature> RHIDepthCommandSignature;
	std::shared_ptr<XRHIStructBuffer> DepthCmdBufferNoCulling;
	std::shared_ptr<XRHIStructBuffer> DepthCmdBufferCulled;
	uint64 DepthCmdBufferOffset;
	uint64 DepthCounterOffset;

	std::shared_ptr<XRHIStructBuffer>GlobalObjectStructBuffer;
	std::shared_ptr<XRHIShaderResourceView>GlobalObjectStructBufferSRV;

	struct cbCullingParametersStruct
	{
		XMatrix  ShdowViewProject;
		XPlane Planes[(int)ECameraPlane::CP_MAX];
		float commandCount;
	};
	cbCullingParametersStruct CullingParametersIns;
	std::shared_ptr<XRHIConstantBuffer>cbCullingParameters;
private:
	//UI
	uint32 IMGUI_IndexOfDescInHeap;
	uint32 IMGUI_IndexOfHeap;
	XEditorUI EditorUI;
	//BasePass
private:
	//std::shared_ptr <XRHITexture2D> VirtualSMFlags;
	//std::shared_ptr <XRHITexture2D> PagetableInfos;
	//std::shared_ptr <XRHITexture2D> VirtualSMFlagsEmptyCopySrc;
	//std::shared_ptr <XRHITexture2D> PhysicalShadowDepthTexture;
	XLocalVertexFactory LocalVertexFactory;

	std::vector<std::shared_ptr<GGeomertry>>RenderGeos;
private:
	uint64 FrameNum = 0;
	float Far = 1000.0f;
	float Near = 1.0f;
	float FoVAngleY = 0.25f * X_PI;

	std::shared_ptr<XRHITexture2D>TextureMetalBaseColor;
	std::shared_ptr<XRHITexture2D>TextureMetalNormal;
	std::shared_ptr<XRHITexture2D>TextureRoughness;

	GTexture2D TextureWoodTexture;
	std::shared_ptr<XRHITexture2D>TextureWoodBaseColor;
	std::shared_ptr<XRHITexture2D>TextureWoodNormal;


	std::shared_ptr<XRHITexture2D>TextureGBufferA;
	std::shared_ptr<XRHITexture2D>TextureGBufferB;
	std::shared_ptr<XRHITexture2D>TextureGBufferC;
	std::shared_ptr<XRHITexture2D>TextureGBufferD;
	std::shared_ptr<XRHITexture2D>TextureSceneColorDeffered;
	std::shared_ptr<XRHITexture2D>TextureSceneColorDefferedPingPong;

	XRHIRenderTargetView* RTViews[8];
private:
	uint32 TileNumWidthPerPhysicalTex = 8;
	uint32 PhysicalTexNumWidthPerVirtualTex = 8;
	uint32 TileNumWidthPerVirtualTex = 8 * 8;
	
	//struct TiledInfoStruct
	//{
	//	XMatrix ViewProjMatrix;
	//	uint32 IndexX;
	//	uint32 IndexY;
	//	uint32 Padding0;
	//	uint32 Padding1;
	//};
	//std::shared_ptr<XRHIConstantBuffer> GlobalShadowViewProjMatrix;


private://deffered light pass

	struct cbDefferedLight
	{
		XVector3 LightDir;
		float padding0 = 0;
		XMFLOAT4 LightColorAndIntensityInLux;
	};

	cbDefferedLight cbDefferedLightIns;
	std::shared_ptr<XRHIConstantBuffer> RHIcbDefferedLight;

private:
	//Full Screen Pass
	//XViewMatrices ViewMatrix;

private://Shadow Pass

	struct ShadowPassConstants
	{
		XMatrix ViewProject;
	};

	ShadowPassConstants ShadowPassConstant;
	std::shared_ptr<XRHIConstantBuffer>ShadowPassConstantBuffer;

	float ShadowMapHeight = 1024;
	float ShadowMapWidth = 1024;
	float ShadowViewportWidth = 1024;

private: //HZBPass 
	//std::shared_ptr<XRHITexture2D> FurthestHZBOutput0;

private://sky atmosphere PreCompute

	//struct cbSkyAtmosphere
	//{
	//	XVector4 TransmittanceLutSizeAndInvSize;
	//	XVector4 MultiScatteredLuminanceLutSizeAndInvSize;
	//	XVector4 SkyViewLutSizeAndInvSize;
	//	XVector4 CameraAerialPerspectiveVolumeSizeAndInvSize;
	//
	//	XVector4 RayleighScattering;
	//	XVector4 MieScattering;
	//	XVector4 MieAbsorption;
	//	XVector4 MieExtinction;
	//
	//	XVector4 GroundAlbedo;
	//
	//	float TopRadiusKm;
	//	float BottomRadiusKm;
	//	float MieDensityExpScale;
	//	float RayleighDensityExpScale;
	//
	//	float TransmittanceSampleCount;
	//	float MultiScatteringSampleCount;
	//	float MiePhaseG;
	//	float padding0 = 0;
	//
	//	XVector4 Light0Illuminance;
	//
	//	float CameraAerialPerspectiveVolumeDepthResolution;
	//	float CameraAerialPerspectiveVolumeDepthResolutionInv;
	//	float CameraAerialPerspectiveVolumeDepthSliceLengthKm;
	//	float padding1;
	//};
	//cbSkyAtmosphere cbSkyAtmosphereIns;
	//std::shared_ptr<XRHIConstantBuffer>RHICbSkyAtmosphere;

	//std::shared_ptr <XRHITexture2D> TransmittanceLutUAV;
	//std::shared_ptr <XRHITexture2D> MultiScatteredLuminanceLutUAV;
	//std::shared_ptr <XRHITexture2D> SkyViewLutUAV;
	//std::shared_ptr <XRHITexture3D> CameraAerialPerspectiveVolumeUAV;

	// Shadow Mask Pass
private:
	//struct VSMTileMaskStruct
	//{
	//	XMatrix LighgViewProject;
	//
	//	float ClientWidth;
	//	float ClientHeight;
	//	float padding0;
	//	float padding1;
	//};
	//std::shared_ptr<XRHIConstantBuffer>VSMTileMaskConstantBuffer;
	std::shared_ptr<XRHITexture2D>VSMShadowMaskTexture;

private:
	std::shared_ptr<XRHITexture2D> SSROutput;


private:
	float clear_color[4] = { 0, 0, 0, 0 };



	XMatrix mLightProj = XMatrix::Identity;
	XMatrix mLightView = XMatrix::Identity;
	POINT mLastMousePos;
};

CrateApp::CrateApp()
{
	BeginInitResource(&LocalVertexFactory);
}

CrateApp::~CrateApp()
{
	if (md3dDevice != nullptr)direct_cmd_queue->CommandQueueWaitFlush();
}


//-------------------
bool CrateApp::Initialize()
{

	if (!D3DApp::Initialize())
		return false;

	direct_ctx->OpenCmdList();

	LoadTextures();

	RViewInfo.ViewConstantBuffer = abstrtact_device.CreateUniformBuffer(sizeof(ViewConstantBufferData));
	RViewInfo.ViewWidth = mClientWidth;
	RViewInfo.ViewHeight = mClientHeight;

	ShadowPassConstantBuffer = abstrtact_device.CreateUniformBuffer(sizeof(ShadowPassConstants));
	//VSMTileMaskConstantBuffer = abstrtact_device.CreateUniformBuffer(sizeof(VSMTileMaskStruct));

	cbCullingParameters = RHICreateConstantBuffer(sizeof(cbCullingParametersStruct));
	//RHICbSkyAtmosphere = abstrtact_device.CreateUniformBuffer(sizeof(cbSkyAtmosphere));
	RHIcbDefferedLight = abstrtact_device.CreateUniformBuffer(sizeof(cbDefferedLight));

	BuildPSOs();
	TestExecute();
	VirtualShadow();

	BoundSphere0.Center = XVector3(0, 0, 0);
	BoundSphere0.Radius = 96.0f;
	DeferredShadingRenderer.SceneBoundingSphere.Center = BoundSphere0.Center;
	DeferredShadingRenderer.SceneBoundingSphere.Radius = BoundSphere0.Radius;
	DeferredShadingRenderer.ShadowLightDir = LightDir;
	DeferredShadingRenderer.MainLightColor = LightColor;
	DeferredShadingRenderer.LightIntensity = LightIntensity;
	DeferredShadingRenderer.RenderGeos = RenderGeos;
	DeferredShadingRenderer.RViewInfo.ViewWidth = mClientWidth;
	DeferredShadingRenderer.RViewInfo.ViewHeight = mClientHeight;
	DeferredShadingRenderer.Setup();


	OutputDebugString(L"1111\n");

	direct_ctx->CloseCmdList();
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	direct_cmd_queue->CommandQueueWaitFlush();
	OutputDebugString(L"2222\n");

	//ImGUI Begin
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingAlwaysTabBar = true;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding = ImVec2(1.0, 0);
	style.FramePadding = ImVec2(14.0, 2.0f);
	style.ChildBorderSize = 0.0f;
	style.FrameRounding = 5.0f;
	style.FrameBorderSize = 1.5f;

	//ImGui::StyleColorsDark();
	EditorUI.SetDefaltStyle();
	EditorUI.InitIOInfo(mClientWidth, mClientHeight);

	ImGui_ImplWin32_Init(mhMainWnd);
	EditorUI.ImGui_Impl_RHI_Init();
	//ImGUI End

	return true;
}


void CrateApp::OnResize()
{
	D3DApp::OnResize();

	CamIns.SetPerspective(FoVAngleY, AspectRatio(), Near, Far);
	XMatrix mProj = CamIns.GetProjectMatrix();
	RViewInfo.ViewMats.Create(mProj, CamIns.GetEyePosition(), CamIns.GetTargetPosition());

	
}

void CrateApp::Update(const GameTimer& gt)
{
	OnKeyboardInput(gt);
	UpdateCamera(gt);
	UpdateMainPassCB(gt);
}


void CrateApp::VirtualShadow()
{

}
void CrateApp::TestExecute()
{
		
	//ObjectConstants
	CullingParametersIns.commandCount = RenderGeos.size();
	
	uint32 ObjConstVecSize = sizeof(ObjectConstants) * RenderGeos.size();
	ObjectConstants* ConstantArray = (ObjectConstants*)std::malloc(ObjConstVecSize);
	
	for (int i = 0; i < RenderGeos.size(); i++)
	{
		auto& RG = RenderGeos[i];
	
		XBoundingBox BoudingBoxTans = RG->GetBoudingBoxWithTrans();
		ConstantArray[i].BoundingBoxCenter = BoudingBoxTans.Center;
		ConstantArray[i].BoundingBoxExtent = BoudingBoxTans.Extent;
		ConstantArray[i].World = RG->GetWorldTransform().GetCombineMatrix();
	}
	
	{
	
		FResourceVectorUint8 ObjectStructBufferData;
		ObjectStructBufferData.Data = ConstantArray;
		ObjectStructBufferData.SetResourceDataSize(ObjConstVecSize);
		XRHIResourceCreateData ObjectStructBufferResourceData(&ObjectStructBufferData);
		
		GlobalObjectStructBuffer = RHIcreateStructBuffer(
			sizeof(ObjectConstants),
			ObjConstVecSize,
			EBufferUsage((int)EBufferUsage::BUF_ShaderResource),
			ObjectStructBufferResourceData);
	
		GlobalObjectStructBufferSRV = RHICreateShaderResourceView(GlobalObjectStructBuffer.get());
	}
}

void CrateApp::Renderer(const GameTimer& gt)
{
	DeferredShadingRenderer.Rendering(RHICmdList);

	RViewInfo = DeferredShadingRenderer.RViewInfo;
	RViewInfo.ViewConstantBuffer.get()->UpdateData(&RViewInfo.ViewCBCPUData, sizeof(ViewConstantBufferData), 0);
	auto TextureDepthStencil = DeferredShadingRenderer.TempGetTextureDepthStencil();
	
	
	//Pass2.5 SkyAtmosPhere PreCompute
	//{
	//	{
	//		RHICmdList.RHIEventBegin(1, "RenderTransmittanceLutCS", sizeof("RenderTransmittanceLutCS"));
	//		TShaderReference<XRenderTransmittanceLutCS> Shader = GetGlobalShaderMapping()->GetShader<XRenderTransmittanceLutCS>();
	//		XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
	//		SetComputePipelineStateFromCS(RHICmdList, ComputeShader);
	//		Shader->SetParameters(RHICmdList, GetRHIUAVFromTexture(TransmittanceLutUAV.get()), RHICbSkyAtmosphere.get());
	//		RHICmdList.RHIDispatchComputeShader(256 / 8, 64 / 8, 1);
	//		RHICmdList.RHIEventEnd();
	//	}
	//
	//	{
	//		RHICmdList.RHIEventBegin(1, "XRenderMultiScatteredLuminanceLutCS", sizeof("XRenderMultiScatteredLuminanceLutCS"));
	//		TShaderReference<XRenderMultiScatteredLuminanceLutCS> Shader = GetGlobalShaderMapping()->GetShader<XRenderMultiScatteredLuminanceLutCS>();
	//		XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
	//		SetComputePipelineStateFromCS(RHICmdList, ComputeShader);
	//		Shader->SetParameters(RHICmdList, GetRHIUAVFromTexture(MultiScatteredLuminanceLutUAV.get()), RHICbSkyAtmosphere.get(), TransmittanceLutUAV.get());
	//		RHICmdList.RHIDispatchComputeShader(32 / 8, 32 / 8, 1);
	//		RHICmdList.RHIEventEnd();
	//	}
	//
	//	//RenderSkyViewLutCS
	//	{
	//		RHICmdList.RHIEventBegin(1, "XRenderSkyViewLutCS", sizeof("XRenderSkyViewLutCS"));
	//		TShaderReference<XRenderSkyViewLutCS> Shader = GetGlobalShaderMapping()->GetShader<XRenderSkyViewLutCS>();
	//		XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
	//		SetComputePipelineStateFromCS(RHICmdList, ComputeShader);
	//		Shader->SetParameters(RHICmdList, RViewInfo.ViewConstantBuffer.get(), RHICbSkyAtmosphere.get(),
	//			GetRHIUAVFromTexture(SkyViewLutUAV.get()), TransmittanceLutUAV.get(), MultiScatteredLuminanceLutUAV.get());
	//		RHICmdList.RHIDispatchComputeShader(192 / 8, 104 / 8, 1);
	//		RHICmdList.RHIEventEnd();
	//	}
	//}
	

	//Pass4 GBufferPass BasePass
	{
		XRHITexture* RTTextures[4];
		RTTextures[0] = TextureGBufferA.get();
		RTTextures[1] = TextureGBufferB.get();
		RTTextures[2] = TextureGBufferC.get();
		RTTextures[3] = TextureGBufferD.get();
		XRHIRenderPassInfo RTInfos(4, RTTextures, ERenderTargetLoadAction::EClear, TextureDepthStencil.get(), EDepthStencilLoadAction::ELoad);
		RHICmdList.RHIBeginRenderPass(RTInfos,"GBufferPass",sizeof("GBufferPass"));
		RHICmdList.CacheActiveRenderTargets(RTInfos);

		for (int i = 0; i < RenderGeos.size(); i++)
		{
			std::shared_ptr<GGeomertry>& GeoInsPtr = RenderGeos[i];
			std::shared_ptr<GMaterialInstance>& MaterialInstancePtr = GeoInsPtr->GetMaterialInstance();

			{
				XGraphicsPSOInitializer_WithoutRT PassState;
				{
					XRHIBoundShaderStateInput_WithoutRT PassShaders;
					{
						XMaterialShaderInfo_Set ShaderInfos;
						XMaterialShader_Set XShaders;

						ShaderInfos.ShaderInfoSet[(int)EShaderType::SV_Compute] = nullptr;
						ShaderInfos.ShaderInfoSet[(int)EShaderType::SV_Vertex] = &TBasePassVS<false>::StaticShaderInfos;
						ShaderInfos.ShaderInfoSet[(int)EShaderType::SV_Pixel] = &TBasePassPS<false>::StaticShaderInfos;

						MaterialInstancePtr->MaterialPtr->RMaterialPtr->GetShaderInfos(ShaderInfos, XShaders);

						TShaderReference<TBasePassVS<false>> BaseVertexShader = TShaderReference<TBasePassVS<false>>(
							static_cast<TBasePassVS<false>*>(XShaders.XShaderSet[(int32)EShaderType::SV_Vertex]), XShaders.ShaderMap);

						TShaderReference<TBasePassPS<false>> BasePixelShader = TShaderReference<TBasePassPS<false>>(
							static_cast<TBasePassPS<false>*>(XShaders.XShaderSet[(int32)EShaderType::SV_Pixel]), XShaders.ShaderMap);

						//SetParameter
						{
							BaseVertexShader->SetParameter(RHICmdList, RViewInfo.ViewConstantBuffer.get(), GeoInsPtr->GetPerObjectVertexCBuffer().get());
							BasePixelShader->SetParameter(RHICmdList, MaterialInstancePtr);
						}

						std::shared_ptr<XRHIVertexLayout> RefVertexLayout = LocalVertexFactory.GetLayout(ELayoutType::Layout_Default);
						PassShaders.RHIVertexLayout = RefVertexLayout.get();

						PassShaders.MappingRHIVertexShader = BaseVertexShader.GetShaderMappingFileUnit()->GetRefShaderMapStoreRHIShaders();
						PassShaders.IndexRHIVertexShader = BaseVertexShader->GetRHIShaderIndex();

						PassShaders.MappingRHIPixelShader = BasePixelShader.GetShaderMappingFileUnit()->GetRefShaderMapStoreRHIShaders();
						PassShaders.IndexRHIPixelShader = BasePixelShader->GetRHIShaderIndex();
					}
					PassState.BoundShaderState = PassShaders;
					PassState.BlendState = TStaticBlendState<>::GetRHI();
					PassState.DepthStencilState = TStaticDepthStencilState<true, ECompareFunction::CF_GreaterEqual>::GetRHI();
				}

				{
					XGraphicsPSOInitializer PSOInitializer = PassState.TransToGraphicsPSOInitializer();
					RHICmdList.ApplyCachedRenderTargets(PSOInitializer);

					SetGraphicsPipelineStateFromPSOInit(RHICmdList, PSOInitializer);
				}
			}
		
			RHICmdList.SetVertexBuffer(GeoInsPtr->GetGVertexBuffer()->GetRHIVertexBuffer().get(), 0, 0);
			RHICmdList.RHIDrawIndexedPrimitive(GeoInsPtr->GetGIndexBuffer()->GetRHIIndexBuffer().get(), GeoInsPtr->GetIndexCount(), 1, 0, 0, 0);
		}

		RHICmdList.RHIEndRenderPass();
	}
	
	auto VirtualSMFlags = DeferredShadingRenderer.TempGetVirtualSMFlags();
	auto VSMTileMaskConstantBuffer = DeferredShadingRenderer.TempGetVSMTileMaskConstantBuffer();
	auto PagetableInfos = DeferredShadingRenderer.TempGetPagetableInfos();
	auto PhysicalShadowDepthTexture = DeferredShadingRenderer.TempGetPhysicalDepth();
	{
		RHICmdList.RHIEventBegin(1, "ShadowMaskGenCS", sizeof("ShadowMaskGenCS"));
		TShaderReference<ShadowMaskGenCS> Shader = GetGlobalShaderMapping()->GetShader<ShadowMaskGenCS>();
		XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
		SetComputePipelineStateFromCS(RHICmdList, ComputeShader);
		
		Shader->SetParameters(RHICmdList,RViewInfo.ViewConstantBuffer.get(),VSMTileMaskConstantBuffer.get(),
			GetRHIUAVFromTexture(VSMShadowMaskTexture.get()),
			GetRHISRVFromTexture(TextureGBufferA.get()),
			GetRHISRVFromTexture(TextureDepthStencil.get()),
			GetRHISRVFromTexture(PagetableInfos.get()),
			GetRHISRVFromTexture(PhysicalShadowDepthTexture.get()));
		
		RHICmdList.RHIDispatchComputeShader(static_cast<uint32>(ceil(mClientWidth / 16)), static_cast<uint32>(ceil(mClientHeight / 16)), 1);
		mCommandList->EndEvent();
	}

	{
		RHICmdList.RHIEventBegin(1, "VSMTileMaskClearCS", sizeof("VSMTileMaskClearCS"));
		TShaderReference<VSMTileMaskClearCS> Shader = GetGlobalShaderMapping()->GetShader<VSMTileMaskClearCS>();
		XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
		SetComputePipelineStateFromCS(RHICmdList, ComputeShader);
		Shader->SetParameters(RHICmdList, GetRHIUAVFromTexture(VirtualSMFlags.get()), GetRHIUAVFromTexture(PhysicalShadowDepthTexture.get()));
		RHICmdList.RHIDispatchComputeShader(8 * 8 / 16, 8 * 8 / 16, 1);
		RHICmdList.RHIEventEnd();
	}

	//LightPass
	{
		XRHITexture* SceneColorRTs = TextureSceneColorDeffered.get();
		XRHIRenderPassInfo RPInfos(1, &SceneColorRTs, ERenderTargetLoadAction::EClear, nullptr, EDepthStencilLoadAction::ENoAction);
		RHICmdList.RHIBeginRenderPass(RPInfos, "LightPass", sizeof("LightPass"));
		RHICmdList.CacheActiveRenderTargets(RPInfos);

		XGraphicsPSOInitializer GraphicsPSOInit;
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();;
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();

		TShaderReference<XLightPassVS> VertexShader = GetGlobalShaderMapping()->GetShader<XLightPassVS>();
		TShaderReference<XLightPassPS> PixelShader = GetGlobalShaderMapping()->GetShader<XLightPassPS>();
		GraphicsPSOInit.BoundShaderState.RHIVertexShader = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.RHIPixelShader = PixelShader.GetPixelShader();
		GraphicsPSOInit.BoundShaderState.RHIVertexLayout = GFullScreenLayout.RHIVertexLayout.get();

		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		SetGraphicsPipelineStateFromPSOInit(RHICmdList, GraphicsPSOInit);

		VertexShader->SetParameter(RHICmdList, RViewInfo.ViewConstantBuffer.get());
		PixelShader->SetParameter(RHICmdList, RViewInfo.ViewConstantBuffer.get(), RHIcbDefferedLight.get(),
			TextureGBufferA.get(), TextureGBufferB.get(), TextureGBufferC.get(), TextureGBufferD.get(),
			TextureDepthStencil.get(), VSMShadowMaskTexture.get());

		RHICmdList.SetVertexBuffer(GFullScreenVertexRHI.RHIVertexBuffer.get(), 0, 0);
		RHICmdList.RHIDrawIndexedPrimitive(GFullScreenIndexRHI.RHIIndexBuffer.get(), 6, 1, 0, 0, 0);
		RHICmdList.RHIEndRenderPass();
	}

	//if (false)
	//{
	//	//SSRPass
	//	{
	//		XRHITexture* SSRRTs = SSROutput.get();
	//		XRHIRenderPassInfo RPInfos(1, &SSRRTs, ERenderTargetLoadAction::EClear, nullptr, EDepthStencilLoadAction::ENoAction);
	//		RHICmdList.RHIBeginRenderPass(RPInfos, "SSRPassPS",sizeof("SSRPassPS"));
	//		RHICmdList.CacheActiveRenderTargets(RPInfos);
	//
	//		XGraphicsPSOInitializer GraphicsPSOInit;
	//		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();;
	//		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
	//
	//		TShaderReference<RFullScreenQuadVS> SSRVertexShader = GetGlobalShaderMapping()->GetShader<RFullScreenQuadVS>();
	//		TShaderReference<XSSRPassPS> SSRPixelShader = GetGlobalShaderMapping()->GetShader<XSSRPassPS>();
	//		GraphicsPSOInit.BoundShaderState.RHIVertexShader = SSRVertexShader.GetVertexShader();
	//		GraphicsPSOInit.BoundShaderState.RHIPixelShader = SSRPixelShader.GetPixelShader();
	//		GraphicsPSOInit.BoundShaderState.RHIVertexLayout = GFullScreenLayout.RHIVertexLayout.get();
	//
	//		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	//		SetGraphicsPipelineStateFromPSOInit(RHICmdList, GraphicsPSOInit);
	//		SSRPixelShader->SetParameter(RHICmdList,
	//			XVector4(1.0f, 1.0f, 1.0f, 1.0f),
	//			RViewInfo.ViewConstantBuffer.get(),
	//			TextureSceneColorDeffered.get(),
	//			TextureGBufferA.get(),
	//			TextureGBufferB.get(),
	//			TextureGBufferC.get(),
	//			TextureGBufferD.get(),
	//			TextureDepthStencil.get(),
	//			FurthestHZBOutput0.get());
	//
	//		RHICmdList.SetVertexBuffer(GFullScreenVertexRHI.RHIVertexBuffer.get(), 0, 0);
	//		RHICmdList.RHIDrawIndexedPrimitive(GFullScreenIndexRHI.RHIIndexBuffer.get(), 6, 1, 0, 0, 0);
	//		RHICmdList.RHIEndRenderPass();
	//	}
	//
	//
	//
	//	//ReflectionEnvironment Pass
	//	{
	//		XRHITexture* TextureSceneColor = TextureSceneColorDeffered.get();
	//		XRHIRenderPassInfo RPInfos(1, &TextureSceneColor, ERenderTargetLoadAction::ELoad, nullptr, EDepthStencilLoadAction::ENoAction);
	//		RHICmdList.RHIBeginRenderPass(RPInfos, "ReflectionEnvironmentPass",sizeof("ReflectionEnvironmentPass"));
	//		RHICmdList.CacheActiveRenderTargets(RPInfos);
	//
	//		XGraphicsPSOInitializer GraphicsPSOInit;
	//		GraphicsPSOInit.BlendState = TStaticBlendState<true, EBlendOperation::BO_Add, EBlendFactor::BF_One, EBlendFactor::BF_One>::GetRHI();
	//		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
	//
	//		TShaderReference<RFullScreenQuadVS> VertexShader = GetGlobalShaderMapping()->GetShader<RFullScreenQuadVS>();
	//		TShaderReference<XReflectionEnvironmentPS> PixelShader = GetGlobalShaderMapping()->GetShader<XReflectionEnvironmentPS>();
	//		GraphicsPSOInit.BoundShaderState.RHIVertexShader = VertexShader.GetVertexShader();
	//		GraphicsPSOInit.BoundShaderState.RHIPixelShader = PixelShader.GetPixelShader();
	//		GraphicsPSOInit.BoundShaderState.RHIVertexLayout = GFullScreenLayout.RHIVertexLayout.get();
	//
	//		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	//		SetGraphicsPipelineStateFromPSOInit(RHICmdList, GraphicsPSOInit);
	//		PixelShader->SetParameter(RHICmdList,SSROutput.get());
	//		RHICmdList.SetVertexBuffer(GFullScreenVertexRHI.RHIVertexBuffer.get(), 0, 0);
	//		RHICmdList.RHIDrawIndexedPrimitive(GFullScreenIndexRHI.RHIIndexBuffer.get(), 6, 1, 0, 0, 0);
	//		RHICmdList.RHIEndRenderPass();
	//	}
	//}
	
	auto TransmittanceLutUAV= DeferredShadingRenderer.TempGetTransmittanceLutUAV();
	auto SkyViewLutUAV = DeferredShadingRenderer.TempGetSkyViewLutUAV();
	auto RHICbSkyAtmosphere = DeferredShadingRenderer.TempGetRHICbSkyAtmosphere();

	//SkyAtmosphere Combine Pass
	{
		XRHITexture* TextureSceneColor = TextureSceneColorDeffered.get();
		XRHIRenderPassInfo RPInfos(1, &TextureSceneColor, ERenderTargetLoadAction::ELoad, nullptr, EDepthStencilLoadAction::ENoAction);
		RHICmdList.RHIBeginRenderPass(RPInfos, "SkyAtmosphere Combine Pass",sizeof("SkyAtmosphere Combine Pass"));
		RHICmdList.CacheActiveRenderTargets(RPInfos);

		XGraphicsPSOInitializer GraphicsPSOInit;
		GraphicsPSOInit.BlendState = TStaticBlendState<true,
			EBlendOperation::BO_Add, EBlendFactor::BF_One, EBlendFactor::BF_One,
			EBlendOperation::BO_Add, EBlendFactor::BF_One, EBlendFactor::BF_One>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();

		TShaderReference<RFullScreenQuadVS> VertexShader = GetGlobalShaderMapping()->GetShader<RFullScreenQuadVS>();
		TShaderReference<XRenderSkyAtmosphereRayMarchingPS> PixelShader = GetGlobalShaderMapping()->GetShader<XRenderSkyAtmosphereRayMarchingPS>();
		GraphicsPSOInit.BoundShaderState.RHIVertexShader = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.RHIPixelShader = PixelShader.GetPixelShader();
		GraphicsPSOInit.BoundShaderState.RHIVertexLayout = GFullScreenLayout.RHIVertexLayout.get();

		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		SetGraphicsPipelineStateFromPSOInit(RHICmdList, GraphicsPSOInit);
		PixelShader->SetParameter(RHICmdList, RViewInfo.ViewConstantBuffer.get(), RHICbSkyAtmosphere.get(),
			SkyViewLutUAV.get(), TextureDepthStencil.get(), TransmittanceLutUAV.get());

		RHICmdList.SetVertexBuffer(GFullScreenVertexRHI.RHIVertexBuffer.get(), 0, 0);
		RHICmdList.RHIDrawIndexedPrimitive(GFullScreenIndexRHI.RHIIndexBuffer.get(), 6, 1, 0, 0, 0);
		RHICmdList.RHIEndRenderPass();
	}

	//ToneMapping
	{
		XRHITexture* TextureSceneColor = TextureSceneColorDefferedPingPong.get();
		XRHIRenderPassInfo RPInfos(1, &TextureSceneColor, ERenderTargetLoadAction::ELoad, nullptr, EDepthStencilLoadAction::ENoAction);
		RHICmdList.RHIBeginRenderPass(RPInfos, "PostProcess_ToneMapping",sizeof("PostProcess_ToneMapping"));
		RHICmdList.CacheActiveRenderTargets(RPInfos);

		XGraphicsPSOInitializer GraphicsPSOInit;
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();

		TShaderReference<RFullScreenQuadVS> VertexShader = GetGlobalShaderMapping()->GetShader<RFullScreenQuadVS>();
		TShaderReference<XToneMappingPassPS> PixelShader = GetGlobalShaderMapping()->GetShader<XToneMappingPassPS>();
		GraphicsPSOInit.BoundShaderState.RHIVertexShader = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.RHIPixelShader = PixelShader.GetPixelShader();
		GraphicsPSOInit.BoundShaderState.RHIVertexLayout = GFullScreenLayout.RHIVertexLayout.get();

		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		SetGraphicsPipelineStateFromPSOInit(RHICmdList, GraphicsPSOInit);
		PixelShader->SetParameter(RHICmdList, TextureSceneColorDeffered.get());

		RHICmdList.SetVertexBuffer(GFullScreenVertexRHI.RHIVertexBuffer.get(), 0, 0);
		RHICmdList.RHIDrawIndexedPrimitive(GFullScreenIndexRHI.RHIIndexBuffer.get(), 6, 1, 0, 0, 0);
		RHICmdList.RHIEndRenderPass();
	}

	XRHITexture* CurrentPingPongTextureSceneColorTarget = TextureSceneColorDefferedPingPong.get();
	{
		EditorUI.ImGui_Impl_RHI_NewFrame(&RHICmdList);
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		EditorUI.OnTick();
		ImGui::Render();
		EditorUI.ImGui_Impl_RHI_RenderDrawData(ImGui::GetDrawData(), &RHICmdList, CurrentPingPongTextureSceneColorTarget);
	}

	//Pass9 FinalPass
	{
		XRHITexture* BackTex = viewport.GetCurrentBackTexture();
		XRHIRenderPassInfo RPInfos(1, &BackTex, ERenderTargetLoadAction::EClear, nullptr, EDepthStencilLoadAction::ENoAction);
		RHICmdList.RHIBeginRenderPass(RPInfos, "FinalPass",sizeof("FinalPass"));
		RHICmdList.CacheActiveRenderTargets(RPInfos);

		XGraphicsPSOInitializer GraphicsPSOInit;
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();;
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();

		TShaderReference<RFullScreenQuadVS> SSRVertexShader = GetGlobalShaderMapping()->GetShader<RFullScreenQuadVS>();
		TShaderReference<XFinalPassPS> SSRPixelShader = GetGlobalShaderMapping()->GetShader<XFinalPassPS>();
		GraphicsPSOInit.BoundShaderState.RHIVertexShader = SSRVertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.RHIPixelShader = SSRPixelShader.GetPixelShader();
		GraphicsPSOInit.BoundShaderState.RHIVertexLayout = GFullScreenLayout.RHIVertexLayout.get();

		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		SetGraphicsPipelineStateFromPSOInit(RHICmdList, GraphicsPSOInit);
		SSRPixelShader->SetParameter(RHICmdList, TextureSceneColorDefferedPingPong.get());

		RHICmdList.SetVertexBuffer(GFullScreenVertexRHI.RHIVertexBuffer.get(), 0, 0);
		RHICmdList.RHIDrawIndexedPrimitive(GFullScreenIndexRHI.RHIIndexBuffer.get(), 6, 1, 0, 0, 0);
		RHICmdList.RHIEndRenderPass();
	}


	{
		XD3D12PlatformRHI::TransitionResource(
			*direct_ctx->GetCmdList(),
			viewport.GetCurrentBackTexture()->GetRenderTargetView(),
			D3D12_RESOURCE_STATE_PRESENT);

		direct_ctx->GetCmdList()->CmdListFlushBarrier();
		direct_ctx->CloseCmdList();
		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		direct_cmd_queue->CommandQueueWaitFlush();
		viewport.Present();
	}

}

void CrateApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void CrateApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CrateApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		CamIns.ProcessMouseMove(static_cast<float>(x - mLastMousePos.x), static_cast<float>(y - mLastMousePos.y));
	}


	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void CrateApp::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();

	if (GetAsyncKeyState('W') & 0x8000)
		CamIns.WalkWS(10.0f * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		CamIns.WalkWS(-10.0f * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		CamIns.WalkAD(-10.0f * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		CamIns.WalkAD(10.0f * dt);
}


void CrateApp::UpdateCamera(const GameTimer& gt)
{
	RViewInfo.ViewMats.UpdateViewMatrix(CamIns.GetEyePosition(), CamIns.GetTargetPosition());

	{
		//BoundSphere BoundSphere0;


		//Compute Project Matrix
		float l = BoundSphere0.Center.x - BoundSphere0.Radius;
		float r = BoundSphere0.Center.x + BoundSphere0.Radius;
		float t = BoundSphere0.Center.y + BoundSphere0.Radius;
		float b = BoundSphere0.Center.y - BoundSphere0.Radius;
		float f = BoundSphere0.Radius * 2;
		float n = 0.1f;
		
		XMMATRIX lightProj = DirectX::XMMatrixOrthographicOffCenterLH(l, r, b, t, f, n);
		XMStoreFloat4x4(&mLightProj, lightProj);
		
		//Compute Light Pos
		XMVECTOR LightDirCom = DirectX::XMVector3Normalize(XMLoadFloat3(&LightDir));
		XMVECTOR lightPos = LightDirCom * BoundSphere0.Radius * 1.0;
		XVector3 lightPosStore; XMStoreFloat3(&lightPosStore, lightPos);
		
		XVector3 upTempp(0, 1, 0);
		XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, BoundSphere0.Center, upTempp);
		
		XMStoreFloat4x4(&mLightView, lightView);
	}
	
}

void CrateApp::UpdateMainPassCB(const GameTimer& gt)
{

	//for deffered light pass
	cbDefferedLightIns.LightDir = LightDir;
	cbDefferedLightIns.LightColorAndIntensityInLux = XMFLOAT4(LightColor.x, LightColor.y, LightColor.z, LightIntensity);
	RHIcbDefferedLight->UpdateData(&cbDefferedLightIns, sizeof(cbDefferedLight), 0);


	FrameNum++;
	RViewInfo.ViewCBCPUData.StateFrameIndexMod8 = FrameNum % 8;
	RViewInfo.ViewCBCPUData.ViewSizeAndInvSize = XMFLOAT4(mClientWidth, mClientHeight, 1.0 / mClientWidth, 1.0 / mClientHeight);
	float LenSqrt = sqrt(LightDir.x * LightDir.x + LightDir.y * LightDir.y + LightDir.z * LightDir.z);
	RViewInfo.ViewCBCPUData.AtmosphereLightDirection = XMFLOAT4(LightDir.x / LenSqrt, LightDir.y / LenSqrt, LightDir.z / LenSqrt, 1.0f);
	RViewInfo.ViewCBCPUData.ViewToClip = RViewInfo.ViewMats.GetProjectionMatrix();
	RViewInfo.ViewCBCPUData.TranslatedViewProjectionMatrix = RViewInfo.ViewMats.GetTranslatedViewProjectionMatrix();
	RViewInfo.ViewCBCPUData.ScreenToWorld = RViewInfo.ViewMats.GetScreenToWorld();
	RViewInfo.ViewCBCPUData.ScreenToTranslatedWorld = RViewInfo.ViewMats.GetScreenToTranslatedWorld();
	RViewInfo.ViewCBCPUData.InvDeviceZToWorldZTransform = CreateInvDeviceZToWorldZTransform(RViewInfo.ViewMats.GetProjectionMatrix());
	RViewInfo.ViewCBCPUData.WorldCameraOrigin = RViewInfo.ViewMats.GetViewOrigin();
	RViewInfo.ViewCBCPUData.ViewProjectionMatrix = RViewInfo.ViewMats.GetViewProjectionMatrix();
	RViewInfo.ViewCBCPUData.ViewProjectionMatrixInverse = RViewInfo.ViewMats.GetViewProjectionMatrixInverse();
	RViewInfo.ViewCBCPUData.BufferSizeAndInvSize = XMFLOAT4(mClientWidth, mClientHeight, 1.0f / mClientWidth, 1.0f / mClientHeight);
	
	DeferredShadingRenderer.LightViewMat = mLightView;
	DeferredShadingRenderer.LightProjMat = mLightProj;

	XMatrix mLightViewProj = mLightView * mLightProj;
	memcpy(&ShadowPassConstant.ViewProject, &mLightViewProj, sizeof(XMatrix));
	ShadowPassConstantBuffer.get()->UpdateData(&ShadowPassConstant, sizeof(ShadowPassConstants), 0);

	
	//cbSkyAtmosphere
	//{
	//	// All distance here are in kilometer and scattering/absorptions coefficient in 1/kilometers.
	//	const float EarthBottomRadius = 6360.0f;
	//	const float EarthTopRadius = 6420.0f;
	//
	//	//SkyComponent SkyAtmosphereCommonData.cpp
	//	XMFLOAT4 RayleighScattering = XMFLOAT4(0.175287, 0.409607, 1, 1);
	//	float RayleighScatteringScale = 0.0331;
	//
	//	float RayleighExponentialDistribution = 8.0f;
	//	float RayleighDensityExpScale = -1.0 / RayleighExponentialDistribution;
	//
	//	XMFLOAT4 MieScattering = XMFLOAT4(1, 1, 1, 1);
	//	XMFLOAT4 MieAbsorption = XMFLOAT4(1, 1, 1, 1);
	//
	//	// The altitude in kilometer at which Mie effects are reduced to 40%.
	//	float MieExponentialDistribution = 1.2;
	//	float MieScatteringScale = 0.003996;
	//	float MieAbsorptionScale = 0.000444;
	//
	//	float MieDensityExpScale = -1.0f / MieExponentialDistribution;
	//	float TransmittanceSampleCount = 10.0f;
	//	float MultiScatteringSampleCount = 15.0f;
	//
	//
	//	//For RenderSkyViewLutCS
	//	const float CmToSkyUnit = 0.00001f;			// Centimeters to Kilometers
	//	const float SkyUnitToCm = 1.0f / 0.00001f;	// Kilometers to Centimeters
	//
	//	XVector3 CametaWorldOrigin = CamIns.GetEyePosition();
	//	XVector3 CametaTargetPos = CamIns.GetTargetPosition();
	//
	//
	//	XMVECTOR Forward = XMLoadFloat3(GetRValuePtr(XVector3(CametaTargetPos.x - CametaWorldOrigin.x,
	//		CametaTargetPos.y - CametaWorldOrigin.y, CametaTargetPos.z - CametaWorldOrigin.z)));
	//
	//	XMVECTOR PlanetCenterKm = XMLoadFloat3(GetRValuePtr(XVector3(0, -EarthBottomRadius, 0)));
	//	const float PlanetRadiusOffset = 0.005f;
	//	const float Offset = PlanetRadiusOffset * SkyUnitToCm;
	//	const float BottomRadiusWorld = EarthBottomRadius * SkyUnitToCm;
	//	const XMVECTOR PlanetCenterWorld = PlanetCenterKm * SkyUnitToCm;
	//
	//	XMVECTOR CametaWorldOriginCom = XMLoadFloat3(&CametaWorldOrigin);
	//	const XMVECTOR PlanetCenterToCameraWorld = CametaWorldOriginCom - PlanetCenterWorld;
	//
	//	XMFLOAT3 LengthCamToCenter;
	//	XMStoreFloat3(&LengthCamToCenter, XMVector3Length(PlanetCenterToCameraWorld));
	//	const float DistanceToPlanetCenterWorld = LengthCamToCenter.x;
	//
	//	//X_Assert(DistanceToPlanetCenterWorld > (BottomRadiusWorld + Offset));
	//	// If the camera is below the planet surface, we snap it back onto the surface.
	//	XMVECTOR SkyWorldCameraOrigin = DistanceToPlanetCenterWorld < (BottomRadiusWorld + Offset)
	//		? PlanetCenterWorld + (BottomRadiusWorld + Offset) * (PlanetCenterToCameraWorld / DistanceToPlanetCenterWorld) :
	//		//XMLoadFloat3(&mEyePos) * SkyUnitToCm;
	//		XMLoadFloat3(&CametaWorldOrigin);
	//
	//	XMFLOAT3 SkyPlanetCenter; XMStoreFloat3(&SkyPlanetCenter, PlanetCenterWorld);
	//	XMFLOAT3 SkyViewHeight; XMStoreFloat3(&SkyViewHeight, XMVector3Length(SkyWorldCameraOrigin - PlanetCenterWorld));
	//
	//	XMStoreFloat3(&RViewInfo.ViewCBCPUData.SkyWorldCameraOrigin, SkyWorldCameraOrigin);
	//	RViewInfo.ViewCBCPUData.SkyPlanetCenterAndViewHeight = XMFLOAT4(SkyPlanetCenter.x, SkyPlanetCenter.y, SkyPlanetCenter.z, SkyViewHeight.x);
	//
	//	XMVECTOR SkyUp = (SkyWorldCameraOrigin - PlanetCenterWorld) * CmToSkyUnit;
	//	SkyUp = XMVector3Normalize(SkyUp);
	//
	//	//TODO
	//	//XMVECTOR SkyLeft = XMVector3Cross(Forward, SkyUp);
	//	XMVECTOR SkyLeft = XMVector3Cross(SkyUp, Forward);
	//	SkyLeft = XMVector3Normalize(SkyLeft);
	//
	//	XVector3 DotMainDir;
	//	XMStoreFloat3(&DotMainDir, XMVectorAbs(XMVector3Dot(SkyUp, Forward)));
	//	if (DotMainDir.x > 0.999f)
	//	{
	//		X_Assert(false);
	//		XVector3 UpStore; XMStoreFloat3(&UpStore, SkyUp);
	//		const float Sign = UpStore.z >= 0.0f ? 1.0f : -1.0f;
	//		const float a = -1.0f / (Sign + UpStore.z);
	//		const float b = UpStore.x * UpStore.y * a;
	//		Forward = XMLoadFloat3(GetRValuePtr(XVector3(1 + Sign * a * pow(UpStore.x, 2.0f), Sign * b, -Sign * UpStore.x)));
	//		SkyLeft = XMLoadFloat3(GetRValuePtr(XVector3(b, Sign + a * pow(UpStore.y, 2.0f), -UpStore.y)));
	//	}
	//	else
	//	{
	//		Forward = XMVector3Cross(SkyUp, SkyLeft);
	//		Forward = XMVector3Normalize(Forward);
	//	}
	//
	//	XMFLOAT4 SkyViewRow0; XMStoreFloat4(&SkyViewRow0, SkyLeft);
	//
	//	XMFLOAT4 SkyViewRow1; XMStoreFloat4(&SkyViewRow1, SkyUp);
	//	XMFLOAT4 SkyViewRow2; XMStoreFloat4(&SkyViewRow2, Forward);
	//
	//	XMFLOAT4X4 SkyViewLutReferential(
	//		SkyViewRow0.x, SkyViewRow0.y, SkyViewRow0.z, 0,
	//		SkyViewRow1.x, SkyViewRow1.y, SkyViewRow1.z, 0,
	//		SkyViewRow2.x, SkyViewRow2.y, SkyViewRow2.z, 0,
	//		0, 0, 0, 0);
	//
	//	RViewInfo.ViewCBCPUData.SkyViewLutReferential = SkyViewLutReferential;
	//
	//
	//	//SkyAtmosphere Precompute
	//	cbSkyAtmosphereIns.TransmittanceLutSizeAndInvSize = XMFLOAT4(256.0, 64.0, 1.0 / 256.0, 1.0 / 64.0);
	//	cbSkyAtmosphereIns.MultiScatteredLuminanceLutSizeAndInvSize = XMFLOAT4(32.0, 32.0, 1.0 / 32.0, 1.0 / 32.0);
	//	cbSkyAtmosphereIns.SkyViewLutSizeAndInvSize = XMFLOAT4(192.0, 104.0, 1.0 / 192.0, 1.0 / 104.0);
	//	cbSkyAtmosphereIns.CameraAerialPerspectiveVolumeSizeAndInvSize = XMFLOAT4(32.0, 32.0, 1.0 / 32.0, 1.0 / 32.0);
	//
	//	cbSkyAtmosphereIns.RayleighScattering = XMFLOAT4(
	//		RayleighScattering.x * RayleighScatteringScale,
	//		RayleighScattering.y * RayleighScatteringScale,
	//		RayleighScattering.z * RayleighScatteringScale,
	//		RayleighScattering.w * RayleighScatteringScale);
	//
	//	cbSkyAtmosphereIns.MieScattering = XMFLOAT4(
	//		MieScattering.x * MieScatteringScale,
	//		MieScattering.y * MieScatteringScale,
	//		MieScattering.z * MieScatteringScale,
	//		MieScattering.w * MieScatteringScale);
	//
	//	cbSkyAtmosphereIns.MieAbsorption = XMFLOAT4(
	//		MieAbsorption.x * MieAbsorptionScale,
	//		MieAbsorption.y * MieAbsorptionScale,
	//		MieAbsorption.z * MieAbsorptionScale,
	//		MieAbsorption.w * MieAbsorptionScale);
	//
	//	cbSkyAtmosphereIns.MieExtinction = XMFLOAT4(
	//		cbSkyAtmosphereIns.MieScattering.x + cbSkyAtmosphereIns.MieAbsorption.x,
	//		cbSkyAtmosphereIns.MieScattering.y + cbSkyAtmosphereIns.MieAbsorption.y,
	//		cbSkyAtmosphereIns.MieScattering.z + cbSkyAtmosphereIns.MieAbsorption.z,
	//		cbSkyAtmosphereIns.MieScattering.w + cbSkyAtmosphereIns.MieAbsorption.w);
	//
	//	cbSkyAtmosphereIns.BottomRadiusKm = EarthBottomRadius;
	//	cbSkyAtmosphereIns.TopRadiusKm = EarthTopRadius;
	//	cbSkyAtmosphereIns.MieDensityExpScale = MieDensityExpScale;
	//	cbSkyAtmosphereIns.RayleighDensityExpScale = RayleighDensityExpScale;
	//	cbSkyAtmosphereIns.TransmittanceSampleCount = TransmittanceSampleCount;
	//	cbSkyAtmosphereIns.MultiScatteringSampleCount = MultiScatteringSampleCount;
	//	cbSkyAtmosphereIns.GroundAlbedo = XMFLOAT4(0.66, 0.66, 0.66, 1.0);
	//	cbSkyAtmosphereIns.MiePhaseG = 0.8f;
	//
	//	cbSkyAtmosphereIns.Light0Illuminance = XMFLOAT4(
	//		LightColor.x * LightIntensity, LightColor.y * LightIntensity, LightColor.z * LightIntensity, 0);
	//
	//	cbSkyAtmosphereIns.CameraAerialPerspectiveVolumeDepthResolution = 16;
	//	cbSkyAtmosphereIns.CameraAerialPerspectiveVolumeDepthResolutionInv = 1.0 / 16.0;
	//	cbSkyAtmosphereIns.CameraAerialPerspectiveVolumeDepthSliceLengthKm = 96 / 16;
	//
	//	RHICbSkyAtmosphere->UpdateData(&cbSkyAtmosphereIns, sizeof(cbSkyAtmosphere), 0);
	//}


	//RViewInfo.ViewConstantBuffer.get()->UpdateData(&RViewInfo.ViewCBCPUData, sizeof(ViewConstantBufferData), 0);

	RViewInfo.ViewMats.GetPlanes(CullingParametersIns.Planes);
	CullingParametersIns.ShdowViewProject = mLightViewProj;
	cbCullingParameters->UpdateData(&CullingParametersIns, sizeof(cbCullingParametersStruct), 0);


	{
		DeferredShadingRenderer.GlobalObjectStructBufferSRV = GlobalObjectStructBufferSRV;
		DeferredShadingRenderer.cbCullingParameters = cbCullingParameters;
		DeferredShadingRenderer.RViewInfo = RViewInfo;
	}
}


void CrateApp::LoadTextures()
{
	std::shared_ptr<GGeomertry> DefaultSphere = TempCreateSphereGeoWithMat();
	DefaultSphere->GetGVertexBuffer()->CreateRHIBufferChecked();
	DefaultSphere->GetGIndexBuffer()->CreateRHIBufferChecked();
	DefaultSphere->SetWorldTranslate(XVector3(0, -0.5, 0));

	std::shared_ptr<GGeomertry> SphereInsUp = DefaultSphere->CreateGeoInstancewithMat();
	SphereInsUp->SetWorldTranslate(XVector3(0, 1.5, 0));

	std::shared_ptr<GGeomertry> SphereInsRight = DefaultSphere->CreateGeoInstancewithMat();
	SphereInsRight->SetWorldTranslate(XVector3(2, 1.5, 0));

	std::shared_ptr<GGeomertry> DefaultQuad = TempCreateQuadGeoWithMat();
	DefaultQuad->GetGVertexBuffer()->CreateRHIBufferChecked();
	DefaultQuad->GetGIndexBuffer()->CreateRHIBufferChecked();
	DefaultQuad->SetWorldTranslate(XVector3(0.0, 1.01, 0.0));

	RenderGeos.push_back(DefaultSphere);
	RenderGeos.push_back(SphereInsUp);
	RenderGeos.push_back(DefaultQuad);
	RenderGeos.push_back(SphereInsRight);

	MainInit::TempInit2();

	{
		TextureGBufferA = RHICreateTexture2D(mClientWidth, mClientHeight, 1, false, false,
			EPixelFormat::FT_R16G16B16A16_FLOAT
			, ETextureCreateFlags(TexCreate_RenderTargetable), 1
			, nullptr);

		TextureGBufferB = RHICreateTexture2D(mClientWidth, mClientHeight, 1, false, false,
			EPixelFormat::FT_R16G16B16A16_FLOAT
			, ETextureCreateFlags(TexCreate_RenderTargetable), 1
			, nullptr);

		TextureGBufferC = RHICreateTexture2D(mClientWidth, mClientHeight, 1, false, false,
			EPixelFormat::FT_R16G16B16A16_FLOAT
			, ETextureCreateFlags(TexCreate_RenderTargetable), 1
			, nullptr);

		TextureGBufferD = RHICreateTexture2D(mClientWidth, mClientHeight, 1, false, false,
			EPixelFormat::FT_R8G8B8A8_UNORM
			, ETextureCreateFlags(TexCreate_RenderTargetable), 1
			, nullptr);

		TextureSceneColorDeffered = RHICreateTexture2D(mClientWidth, mClientHeight, 1, false, false,
			EPixelFormat::FT_R16G16B16A16_FLOAT
			, ETextureCreateFlags(TexCreate_RenderTargetable), 1
			, nullptr);

		TextureSceneColorDefferedPingPong = RHICreateTexture2D(mClientWidth, mClientHeight, 1, false, false,
			EPixelFormat::FT_R16G16B16A16_FLOAT
			, ETextureCreateFlags(TexCreate_RenderTargetable), 1
			, nullptr);

		SSROutput = RHICreateTexture2D(mClientWidth, mClientHeight, 1, false, false,
			EPixelFormat::FT_R8G8B8A8_UNORM
			, ETextureCreateFlags(TexCreate_RenderTargetable | TexCreate_ShaderResource), 1
			, nullptr);

		//TransmittanceLutUAV = RHICreateTexture2D(256, 64, 1, false, false,
		//	EPixelFormat::FT_R11G11B10_FLOAT
		//	, ETextureCreateFlags(TexCreate_UAV | TexCreate_ShaderResource), 1
		//	, nullptr);
		//
		//MultiScatteredLuminanceLutUAV = RHICreateTexture2D(32, 32, 1, false, false,
		//	EPixelFormat::FT_R11G11B10_FLOAT
		//	, ETextureCreateFlags(TexCreate_UAV | TexCreate_ShaderResource), 1
		//	, nullptr);

		VSMShadowMaskTexture = RHICreateTexture2D(mClientWidth, mClientHeight, 1, false, false,
			EPixelFormat::FT_R16_FLOAT
			, ETextureCreateFlags(TexCreate_UAV | TexCreate_ShaderResource), 1
			, nullptr);


		//SkyViewLutUAV = RHICreateTexture2D(192, 104, 1, false, false,
		//	EPixelFormat::FT_R11G11B10_FLOAT
		//	, ETextureCreateFlags(TexCreate_UAV | TexCreate_ShaderResource), 1
		//	, nullptr);
		//
		//CameraAerialPerspectiveVolumeUAV = RHICreateTexture3D(32, 32, 16,
		//	EPixelFormat::FT_R16G16B16A16_FLOAT
		//	, ETextureCreateFlags(TexCreate_UAV | TexCreate_ShaderResource), 1
		//	, nullptr);
	}

}

void CrateApp::BuildPSOs()
{
	CompileGlobalShaderMap();
}

void CrateApp::TempDelete()
{
	if (GPlatformRHI)
		delete GPlatformRHI;
	if (GGlobalShaderMapping)
		delete GGlobalShaderMapping;

	//ImGUI Begin
	EditorUI.ImGui_Impl_RHI_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	//ImGUI End
}




void TestReflectAndArchive()
{
	std::cout << GTexture::StaticReflectionInfo.GetProperty(0)->PropertyName << std::endl;
	std::wstring FileName = XPath::ProjectResourceSavedDir() + L"/T_Rock_Sandstone_D111.xasset";
	std::shared_ptr<XArchiveBase>ArchiveWriterTex = XFileManagerGeneric::CreateFileWriter(FileName.c_str());
	GTexture2D Texture;
	Texture.LoadTextureFromImage("E:/XEngine/XEnigine/Source/Shaders/T_Rock_Sandstone_D.TGA");
	Texture.ArchiveImpl(*ArchiveWriterTex);
	ArchiveWriterTex->Close();

	std::shared_ptr<XArchiveBase>ArchiveReaderTex = XFileManagerGeneric::CreateFileReader(FileName.c_str());
	GTexture2D Texture2;
	Texture2.ArchiveImpl(*ArchiveReaderTex);
	X_Assert(Texture2.SizeX == Texture.SizeX);
}

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(304);
	int* a = new int(5);

	{
		MainInit::Init();
		TestReflectAndArchive();

		CrateApp theApp;
		if (!theApp.Initialize())return 0;
		return theApp.Run();
	}

	_CrtDumpMemoryLeaks();
}


