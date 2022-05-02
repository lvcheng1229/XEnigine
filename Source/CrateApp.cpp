//***************************************************************************************
// CrateApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include <memory>
#include <windows.h>
#include "UnitTest/d3dApp.h"
#include "UnitTest/MathHelper.h"
#include "UnitTest/GeometryGenerator.h"
#include "FrameResource.h"


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


#include "Runtime/Engine/Classes/Material.h"

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
		LightAttenuationTexture.Bind(Initializer.ShaderParameterMap, "LightAttenuationTexture");
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
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, LightAttenuationTexture, InLightAttenuationTexture);
	}

	CBVParameterType CBV_View;
	CBVParameterType CBV_DefferedLight;

	TextureParameterType GBufferATexture;
	TextureParameterType GBufferBTexture;
	TextureParameterType GBufferCTexture;
	TextureParameterType GBufferDTexture;
	TextureParameterType SceneDepthTexture;
	TextureParameterType LightAttenuationTexture;

};

XLightPassVS::ShaderInfos XLightPassVS::StaticShaderInfos(
	"XLightPassVS", L"E:/XEngine/XEnigine/Source/Shaders/DeferredLightVertexShaders.hlsl", 
	"DeferredLightVertexMain", EShaderType::SV_Vertex, XLightPassVS::CustomConstrucFunc,
	XLightPassVS::ModifyShaderCompileSettings);
XLightPassPS::ShaderInfos XLightPassPS::StaticShaderInfos(
	"XLightPassPS", L"E:/XEngine/XEnigine/Source/Shaders/DeferredLightPixelShaders.hlsl", 
	"DeferredLightPixelMain",EShaderType::SV_Pixel, XLightPassPS::CustomConstrucFunc,
	XLightPassPS::ModifyShaderCompileSettings);



//XShadowMaskPassPS
class XShadowMaskPassPS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XShadowMaskPassPS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}

public:
	XShadowMaskPassPS(const XShaderInitlizer& Initializer)
		:XGloablShader(Initializer)
	{
		ShadowMatrixBuffer.Bind(Initializer.ShaderParameterMap, "cbShadowMaskNoCommon");
		ViewCB.Bind(Initializer.ShaderParameterMap, "cbView");

		ShadowDepthTexture.Bind(Initializer.ShaderParameterMap, "ShadowDepthTexture");
		GBufferATexture.Bind(Initializer.ShaderParameterMap, "SceneTexturesStruct_GBufferATexture");
		SceneDepthTexture.Bind(Initializer.ShaderParameterMap, "SceneTexturesStruct_SceneDepthTexture");
	}

	void SetParameter(
		XRHICommandList& RHICommandList,
		XRHIConstantBuffer* InViewCB,
		XRHITexture* InShadowDepthTexture,
		XRHITexture* InGBufferATexture,
		XRHITexture* InSceneDepthTexture
	)
	{
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Pixel, ViewCB, InViewCB);

		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, ShadowDepthTexture, InShadowDepthTexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, GBufferATexture, InGBufferATexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, SceneDepthTexture, InSceneDepthTexture);
	}


	void SetShaderShadowMatrixBuffer(
		XRHICommandList& RHICommandList,
		XRHIConstantBuffer* InShadowMatrixBuffer)
	{
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Pixel, ShadowMatrixBuffer, InShadowMatrixBuffer);
	}

	CBVParameterType ShadowMatrixBuffer;
	CBVParameterType ViewCB;

	TextureParameterType ShadowDepthTexture;
	TextureParameterType GBufferATexture;
	TextureParameterType SceneDepthTexture;
};
XShadowMaskPassPS::ShaderInfos XShadowMaskPassPS::StaticShaderInfos(
	"XShadowMaskPassPS", L"E:/XEngine/XEnigine/Source/Shaders/ShadowProjectionShader.hlsl",
	"PS", EShaderType::SV_Pixel, XShadowMaskPassPS::CustomConstrucFunc,
	XShadowMaskPassPS::ModifyShaderCompileSettings);


//XHZBPassCS
class XHZBPassCS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XHZBPassCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	XHZBPassCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		DispatchThreadIdToBufferUV.Bind(Initializer.ShaderParameterMap, "DispatchThreadIdToBufferUV");
		
		TextureSampledInput.Bind(Initializer.ShaderParameterMap, "TextureSampledInput");
		FurthestHZBOutput_0.Bind(Initializer.ShaderParameterMap, "FurthestHZBOutput_0");
		FurthestHZBOutput_1.Bind(Initializer.ShaderParameterMap, "FurthestHZBOutput_1");
		FurthestHZBOutput_2.Bind(Initializer.ShaderParameterMap, "FurthestHZBOutput_2");
		FurthestHZBOutput_3.Bind(Initializer.ShaderParameterMap, "FurthestHZBOutput_3");
		FurthestHZBOutput_4.Bind(Initializer.ShaderParameterMap, "FurthestHZBOutput_4");
	}

	void SetParameters(
		XRHICommandList& RHICommandList,
		XVector4 InDispatchThreadIdToBufferUV,

		XRHITexture* InTextureSampledInput,
		XRHIUnorderedAcessView* InFurthestHZBOutput_0,
		XRHIUnorderedAcessView* InFurthestHZBOutput_1,
		XRHIUnorderedAcessView* InFurthestHZBOutput_2,
		XRHIUnorderedAcessView* InFurthestHZBOutput_3,
		XRHIUnorderedAcessView* InFurthestHZBOutput_4
	)
	{
		SetShaderValue(RHICommandList, EShaderType::SV_Compute, DispatchThreadIdToBufferUV, InDispatchThreadIdToBufferUV);

		SetTextureParameter(RHICommandList, EShaderType::SV_Compute, TextureSampledInput, InTextureSampledInput);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, FurthestHZBOutput_0, InFurthestHZBOutput_0);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, FurthestHZBOutput_1, InFurthestHZBOutput_1);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, FurthestHZBOutput_2, InFurthestHZBOutput_2);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, FurthestHZBOutput_3, InFurthestHZBOutput_3);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, FurthestHZBOutput_4, InFurthestHZBOutput_4);
	}

	XShaderVariableParameter DispatchThreadIdToBufferUV;

	UAVParameterType TextureSampledInput;
	UAVParameterType FurthestHZBOutput_0;
	UAVParameterType FurthestHZBOutput_1;
	UAVParameterType FurthestHZBOutput_2;
	UAVParameterType FurthestHZBOutput_3;
	UAVParameterType FurthestHZBOutput_4;
};

XHZBPassCS::ShaderInfos XHZBPassCS::StaticShaderInfos(
	"HZBBuildCS", L"E:/XEngine/XEnigine/Source/Shaders/HZB.hlsl",
	"HZBBuildCS", EShaderType::SV_Compute, XHZBPassCS::CustomConstrucFunc,
	XHZBPassCS::ModifyShaderCompileSettings);

//XRenderTransmittanceLutCS
class XRenderTransmittanceLutCS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XRenderTransmittanceLutCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;

	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) 
	{
		OutSettings.SetDefines("THREADGROUP_SIZE", "8");
	}
public:

	XRenderTransmittanceLutCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		TransmittanceLutUAV.Bind(Initializer.ShaderParameterMap, "TransmittanceLutUAV");
		cbSkyAtmosphere.Bind(Initializer.ShaderParameterMap, "SkyAtmosphere");
	}

	void SetParameters(
		XRHICommandList& RHICommandList,
		XRHIUnorderedAcessView* InUAV,
		XRHIConstantBuffer* IncbSkyAtmosphere)
	{
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, TransmittanceLutUAV, InUAV);
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbSkyAtmosphere, IncbSkyAtmosphere);
	}

	CBVParameterType cbSkyAtmosphere;
	TextureParameterType TransmittanceLutUAV;
};

XRenderTransmittanceLutCS::ShaderInfos XRenderTransmittanceLutCS::StaticShaderInfos(
	"RenderTransmittanceLutCS", L"E:/XEngine/XEnigine/Source/Shaders/SkyAtmosphere.hlsl",
	"RenderTransmittanceLutCS", EShaderType::SV_Compute, XRenderTransmittanceLutCS::CustomConstrucFunc,
	XRenderTransmittanceLutCS::ModifyShaderCompileSettings);


//XRenderMultiScatteredLuminanceLutCS
class XRenderMultiScatteredLuminanceLutCS : public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XRenderMultiScatteredLuminanceLutCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;

	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings)
	{
		OutSettings.SetDefines("THREADGROUP_SIZE", "8");
	}
public:
	XRenderMultiScatteredLuminanceLutCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		MultiScatteredLuminanceLutUAV.Bind(Initializer.ShaderParameterMap, "MultiScatteredLuminanceLutUAV");
		TransmittanceLutTexture.Bind(Initializer.ShaderParameterMap, "TransmittanceLutTexture");
		cbSkyAtmosphere.Bind(Initializer.ShaderParameterMap, "SkyAtmosphere");
	}

	void SetParameters(
		XRHICommandList& RHICommandList,
		XRHIUnorderedAcessView* InUAV,
		XRHIConstantBuffer* IncbSkyAtmosphere,
		XRHITexture* InTransmittanceLutTexture)
	{
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, MultiScatteredLuminanceLutUAV, InUAV);
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbSkyAtmosphere, IncbSkyAtmosphere);
		SetTextureParameter(RHICommandList, EShaderType::SV_Compute, TransmittanceLutTexture, InTransmittanceLutTexture);
	}

	UAVParameterType MultiScatteredLuminanceLutUAV;
	CBVParameterType cbSkyAtmosphere;
	TextureParameterType TransmittanceLutTexture;
};

XRenderMultiScatteredLuminanceLutCS::ShaderInfos XRenderMultiScatteredLuminanceLutCS::StaticShaderInfos(
	"RenderMultiScatteredLuminanceLutCS", L"E:/XEngine/XEnigine/Source/Shaders/SkyAtmosphere.hlsl",
	"RenderMultiScatteredLuminanceLutCS", EShaderType::SV_Compute, XRenderMultiScatteredLuminanceLutCS::CustomConstrucFunc,
	XRenderMultiScatteredLuminanceLutCS::ModifyShaderCompileSettings);


//XRenderSkyViewLutCS
class XRenderSkyViewLutCS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XRenderSkyViewLutCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;

	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings)
	{
		OutSettings.SetDefines("THREADGROUP_SIZE", "8");
	}
public:
	XRenderSkyViewLutCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		cbView.Bind(Initializer.ShaderParameterMap, "cbView");
		cbSkyAtmosphere.Bind(Initializer.ShaderParameterMap, "SkyAtmosphere");

		SkyViewLutUAV.Bind(Initializer.ShaderParameterMap, "SkyViewLutUAV");

		MultiScatteredLuminanceLutTexture.Bind(Initializer.ShaderParameterMap, "MultiScatteredLuminanceLutTexture");
		TransmittanceLutTexture.Bind(Initializer.ShaderParameterMap, "TransmittanceLutTexture");
	}

	void SetParameters(
		XRHICommandList& RHICommandList,
		XRHIConstantBuffer* IncbView,
		XRHIConstantBuffer* IncbSkyAtmosphere,

		XRHIUnorderedAcessView* InSkyViewLutUAV,
		XRHITexture* InTransmittanceLutTexture,
		XRHITexture* InMultiScatteredLuminanceLutTexture
		)
	{
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbView, IncbView);
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbSkyAtmosphere, IncbSkyAtmosphere);

		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, SkyViewLutUAV, InSkyViewLutUAV);

		SetTextureParameter(RHICommandList, EShaderType::SV_Compute, TransmittanceLutTexture, InTransmittanceLutTexture);
		SetTextureParameter(RHICommandList, EShaderType::SV_Compute, MultiScatteredLuminanceLutTexture, InMultiScatteredLuminanceLutTexture);
	}
	
	CBVParameterType cbView;
	CBVParameterType cbSkyAtmosphere;

	UAVParameterType SkyViewLutUAV;

	TextureParameterType MultiScatteredLuminanceLutTexture;
	TextureParameterType TransmittanceLutTexture;
};

XRenderSkyViewLutCS::ShaderInfos XRenderSkyViewLutCS::StaticShaderInfos(
	"RenderSkyViewLutCS", L"E:/XEngine/XEnigine/Source/Shaders/SkyAtmosphere.hlsl",
	"RenderSkyViewLutCS", EShaderType::SV_Compute, XRenderSkyViewLutCS::CustomConstrucFunc,
	XRenderSkyViewLutCS::ModifyShaderCompileSettings);




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



struct RenderItem
{
	RenderItem() = default; 
	XMatrix World = XMatrix::Identity;
	XMatrix TexTransform = XMatrix::Identity;
	//XMFLOAT4X4 World = MathHelper::Identity4x4();
	//XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

    // DrawIndexedInstanced parameters.
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;
};

struct BoundSphere
{
	XVector3 Center;
	float Radius;
};

class Camera
{
public:
	void RotateY(float angle)
	{
		// Rotate the basis vectors about the world y-axis.

		XMMATRIX R = XMMatrixRotationY(angle);
		XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
		XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
		XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
	}

	void Pitch(float angle)
	{
		// Rotate up and look vector about the right vector.
		XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mRight), angle);
		XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
		XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
	}

	void Strafe(float d)
	{
		XMVECTOR s = XMVectorReplicate(d);
		XMVECTOR r = XMLoadFloat3(&mRight);
		XMVECTOR p = XMLoadFloat3(&mPosition);
		XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, r, p));
	}
	void Walk(float d)
	{
		// mPosition += d*mLook
		XMVECTOR s = XMVectorReplicate(d);
		XMVECTOR l = XMLoadFloat3(&mLook);
		XMVECTOR p = XMLoadFloat3(&mPosition);
		XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, l, p));
	}

	XVector3 GetEyePosition()
	{
		return mPosition;
	}

	XVector3 GetTargetPosition()
	{
		//return XVector3(mPosition.x + mLook.x, mPosition.y + mLook.y, mPosition.z + mLook.z);
		return mPosition + mLook;
	}
private:
	XVector3 mPosition = { 0.0f, 3.0f, -3.0f };
	XVector3 mRight = { 1.0f, 0.0f, 0.0f };
	XVector3 mUp = { 0.0f, 1.0f, 0.0f };
	XVector3 mLook = { 0.0f, 0.0f, 1.0f };
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
	void UpdateShadowTransform(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	void LoadTextures();
    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    void BuildShapeGeometry();
    void BuildPSOs();
    void BuildMaterials();
    void BuildRenderItems();
	
	BoundSphere BuildBoundSphere(float FoVAngleY, float WHRatio, float SplirNear, float SplitFar);
	
	Camera cam_ins;
	RendererViewInfo RViewInfo;

	XVector3 LightDir = { -1,1,1 };
	XVector3 LightColor = { 1,1,1 };
	float LightIntensity = 7.0f;

private:
	//UI
	uint32 IMGUI_IndexOfDescInHeap;
	uint32 IMGUI_IndexOfHeap;
	XEditorUI EditorUI;
//BasePass
private:
	XLocalVertexFactory LocalVertexFactory;
	
	
	GMaterial SphereGMaterial;
	RMaterial SphereMaterial;
private:
	uint64 FrameNum = 0;
	float Far = 1000.0f;
	float Near = 1.0f;
	float FoVAngleY = 0.25f * MathHelper::Pi;

	XD3D12RootSignature d3d12_root_signature;
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
	XD3D12RootSignature FullScreenRootSig;
	ComPtr<ID3D12PipelineState>FullScreenPSO = nullptr;
	XViewMatrices ViewMatrix;

private://Shadow Pass

	struct ShadowPassConstants
	{
		DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
	};

	
	XViewMatrices LightMatrix[4];

	ShadowPassConstants ShadowPassConstant[4];
	ComPtr<ID3D12PipelineState> ShadowPSO = nullptr;
	XD3D12RootSignature ShadowPassRootSig;
	std::shared_ptr<XRHIConstantBuffer>ShadowPassConstantBuffer[4];
	std::shared_ptr<XRHITexture2D>ShadowTexture0;
	
	float ShadowMapHeight = 1024;
	float ShadowMapWidth = 1024 * 4;
	float ShadowViewportWidth = 1024;

private: //HZBPass 

	std::shared_ptr<XRHITexture2D> FurthestHZBOutput0;

private://sky atmosphere PreCompute
	
	struct cbSkyAtmosphere
	{
		XVector4 TransmittanceLutSizeAndInvSize;
		XVector4 MultiScatteredLuminanceLutSizeAndInvSize;
		XVector4 SkyViewLutSizeAndInvSize;
		XVector4 CameraAerialPerspectiveVolumeSizeAndInvSize;

		XVector4 RayleighScattering;
		XVector4 MieScattering;
		XVector4 MieAbsorption;
		XVector4 MieExtinction;

		XVector4 GroundAlbedo;
		
		float TopRadiusKm;
		float BottomRadiusKm;
		float MieDensityExpScale;
		float RayleighDensityExpScale;
		
		float TransmittanceSampleCount;
		float MultiScatteringSampleCount;
		float MiePhaseG;
		float padding0 = 0;

		XVector4 Light0Illuminance;

		float CameraAerialPerspectiveVolumeDepthResolution;
		float CameraAerialPerspectiveVolumeDepthResolutionInv;
		float CameraAerialPerspectiveVolumeDepthSliceLengthKm;
		float padding1;
	};
	cbSkyAtmosphere cbSkyAtmosphereIns;
	std::shared_ptr<XRHIConstantBuffer>RHICbSkyAtmosphere;
	
	std::shared_ptr <XRHITexture2D> TransmittanceLutUAV;
	std::shared_ptr <XRHITexture2D> MultiScatteredLuminanceLutUAV;
	std::shared_ptr <XRHITexture2D> SkyViewLutUAV;
	std::shared_ptr <XRHITexture3D> CameraAerialPerspectiveVolumeUAV;


private://Shadow Mask Pass 
	struct cbShadowMaskNoCommonBuffer
	{
		XMFLOAT4X4 ScreenToShadowMatrix;
		float x_offset;
		XVector3 padding0;
	};
	cbShadowMaskNoCommonBuffer cbShadowMaskNoCommon[4];
	std::shared_ptr<XRHIConstantBuffer>ShadowMaskNoCommonConstantBuffer[4];
	std::shared_ptr<XRHITexture2D>ShadowMaskTexture;

private://Depth Only
	XD3D12RootSignature PrePassRootSig;
	ComPtr<ID3D12PipelineState> mDepthOnlyPSO = nullptr;

private:
	std::unique_ptr<RenderItem> LightPassItem;
	std::shared_ptr<XRHITexture2D> SSROutput;
	
private:
	XD3D12RootSignature ReflectionEnvironmentRootSig;
	ComPtr<ID3D12PipelineState>ReflectionEnvironmentPassPSO = nullptr;

private:
	float clear_color[4] = { 0, 0, 0, 0 };
	std::unique_ptr<FrameResource>mFrameResource;
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, XShader> mShaders;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mFullScreenInputLayout;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mBasePassLayout;

    ComPtr<ID3D12PipelineState> mOpaquePSO = nullptr;

	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	std::vector<RenderItem*> mOpaqueRitems;
    PassConstants mMainPassCB;
	XMatrix mProj = XMatrix::Identity;
	XMatrix mLightProj = XMatrix::Identity;
    POINT mLastMousePos;
};

CrateApp::CrateApp()
{
	BeginInitResource(&LocalVertexFactory);
}

CrateApp::~CrateApp()
{
    if(md3dDevice != nullptr)direct_cmd_queue->CommandQueueWaitFlush();
}

//-------------------
bool CrateApp::Initialize()
{
	
    if(!D3DApp::Initialize())
        return false;

	direct_ctx->OpenCmdList();

	LoadTextures();
    BuildShadersAndInputLayout();
    BuildRootSignature();
    BuildShapeGeometry();
	BuildMaterials();
    BuildRenderItems();

	mFrameResource = std::make_unique<FrameResource>();
	
	mFrameResource.get()->PassConstantBuffer = abstrtact_device.CreateUniformBuffer(d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants)));
	
	for (int i = 0; i < mMaterials.size();i++)
	{
		mFrameResource.get()->MaterialConstantBuffer.push_back(abstrtact_device.CreateUniformBuffer(d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants))));
	}

	for (uint32 i = 0; i < mAllRitems.size();++i)
	{
		mFrameResource.get()->ObjectConstantBuffer.push_back(abstrtact_device.CreateUniformBuffer(d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants))));
	}

	RViewInfo.ViewConstantBuffer = abstrtact_device.CreateUniformBuffer(d3dUtil::CalcConstantBufferByteSize(sizeof(ViewConstantBufferData)));


	for (uint32 i = 0; i < 4; i++)
	{
		ShadowPassConstantBuffer[i] =abstrtact_device.CreateUniformBuffer(d3dUtil::CalcConstantBufferByteSize(sizeof(ShadowPassConstants)));
	}


	for (uint32 i = 0; i < 4; i++)
	{
		ShadowMaskNoCommonConstantBuffer[i] =abstrtact_device.CreateUniformBuffer(d3dUtil::CalcConstantBufferByteSize(sizeof(cbShadowMaskNoCommonBuffer)));
	}

	RHICbSkyAtmosphere = abstrtact_device.CreateUniformBuffer(d3dUtil::CalcConstantBufferByteSize(sizeof(cbSkyAtmosphere)));
	RHIcbDefferedLight = abstrtact_device.CreateUniformBuffer(d3dUtil::CalcConstantBufferByteSize(sizeof(cbDefferedLight)));

	BuildPSOs();

	OutputDebugString(L"1111\n");

	direct_ctx->CloseCmdList();
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	direct_cmd_queue->CommandQueueWaitFlush();
	OutputDebugString(L"2222\n");

	////ImGUI Begin
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
 
BoundSphere CrateApp::BuildBoundSphere(float FoVAngleY, float WHRatio, float SplitNear, float SplitFar)
{
	float SinVDiv2, CosVDiv2;
	XMScalarSinCos(&SinVDiv2, &CosVDiv2, 0.5f * FoVAngleY);
	float TanVDiv2 = SinVDiv2 / CosVDiv2;

	float ViewNearHeight = SplitNear * TanVDiv2;
	float ViewNearWidth = AspectRatio() * ViewNearHeight;
	float ViewFarHeight = SplitFar * TanVDiv2;
	float ViewFarWidth = AspectRatio() * ViewFarHeight;

	XVector3 NearHOffset = { 0,ViewNearHeight,0 };
	XVector3 NearVOffset = { ViewNearWidth,0,0 };
	XVector3 FarHOffset = { 0,ViewFarHeight,0 };
	XVector3 FarVOffset = { ViewFarWidth,0,0 };

	XMVECTOR NearHOffsetCom = XMLoadFloat3(&NearHOffset);
	XMVECTOR NearVOffsetCom = XMLoadFloat3(&NearVOffset);
	XMVECTOR FarHOffsetCom = XMLoadFloat3(&FarHOffset);
	XMVECTOR FarVOffsetCom = XMLoadFloat3(&FarVOffset);
	

	XMVECTOR EyePosCom = XMLoadFloat3(GetRValuePtr(cam_ins.GetEyePosition()));
	XMVECTOR TargetPosCom = XMLoadFloat3(GetRValuePtr(cam_ins.GetTargetPosition()));
	XMVECTOR CameraDirection = DirectX::XMVector3Normalize(TargetPosCom - EyePosCom);
	
	XMVECTOR CascadeFrustumVerts[8];
	CascadeFrustumVerts[0] = EyePosCom + CameraDirection * SplitNear + NearHOffsetCom + NearVOffsetCom;//Near Top Right
	CascadeFrustumVerts[1] = EyePosCom + CameraDirection * SplitNear + NearHOffsetCom - NearVOffsetCom;//Near Top Left
	CascadeFrustumVerts[2] = EyePosCom + CameraDirection * SplitNear - NearHOffsetCom + NearVOffsetCom;//Near Bottom Right
	CascadeFrustumVerts[3] = EyePosCom + CameraDirection * SplitNear - NearHOffsetCom - NearVOffsetCom;//Near Bottom Left
	CascadeFrustumVerts[4] = EyePosCom + CameraDirection * SplitFar + FarHOffsetCom + FarVOffsetCom;//Far Top Right
	CascadeFrustumVerts[5] = EyePosCom + CameraDirection * SplitFar + FarHOffsetCom - FarVOffsetCom;//Far Top Left
	CascadeFrustumVerts[6] = EyePosCom + CameraDirection * SplitFar - FarHOffsetCom + FarVOffsetCom;//Far Bottom Right
	CascadeFrustumVerts[7] = EyePosCom + CameraDirection * SplitFar - FarHOffsetCom - FarVOffsetCom;//Far Bottom Left

	float FrustumLength = SplitFar - SplitNear;
	float DiagonalNear = ViewNearHeight * ViewNearHeight + ViewNearWidth * ViewNearWidth;
	float DiagonalFar = ViewFarHeight * ViewFarHeight + ViewFarWidth * ViewFarWidth;
	float OptimalOffset = (DiagonalNear - DiagonalFar) / (2.0f * FrustumLength) + FrustumLength * 0.5f;
	float CentreZ = SplitFar - OptimalOffset;
	CentreZ = min(max(CentreZ, SplitNear), SplitFar);
	
	
	XVector3 Center;
	XMVECTOR CenterCom = EyePosCom + CameraDirection * CentreZ;
	XMStoreFloat3(&Center, CenterCom);
	BoundSphere BoundSphereRet = { Center ,0 };
	
	XVector3 Temp;
	for (uint32 i = 0; i < 8; i++)
	{
		XMStoreFloat3(&Temp, XMVector3Length(CascadeFrustumVerts[i] - CenterCom));
		BoundSphereRet.Radius = max(BoundSphereRet.Radius, Temp.x);
	}
	BoundSphereRet.Radius = max(BoundSphereRet.Radius, 1.0f);
	return BoundSphereRet;
}

void CrateApp::OnResize()
{
    D3DApp::OnResize();
	

	XMMATRIX P = XDirectx::XXMMatrixPerspectiveFovLH(FoVAngleY, AspectRatio(), Near, Far);
	
	XMStoreFloat4x4(&mProj, P);
	ViewMatrix.Create(mProj, cam_ins.GetEyePosition(), cam_ins.GetTargetPosition());
	
	
}

void CrateApp::Update(const GameTimer& gt)
{
    OnKeyboardInput(gt);
	UpdateCamera(gt);

	AnimateMaterials(gt);
	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);
}

static XGraphicsPSOInitializer static_RHIPSOINIT;
static XD3DGraphicsPSO static_pso(static_RHIPSOINIT, nullptr, nullptr);

void CrateApp::Renderer(const GameTimer& gt)
{
	pass_state_manager->ResetDescHeapIndex();
	pass_state_manager->TempResetPSO(&static_pso);
	
	//Pass1 DepthPrePass
	{
		direct_ctx->ResetCmdAlloc();
		direct_ctx->OpenCmdList();

		mCommandList->BeginEvent(1, "DepthPrePass", sizeof("DepthPrePass"));

		direct_ctx->RHISetViewport(0.0f, 0.0f, 0.0f, mClientWidth, mClientHeight, 1.0f);
		mCommandList->SetPipelineState(mDepthOnlyPSO.Get());
		pass_state_manager->SetRootSignature(&PrePassRootSig);
		pass_state_manager->SetHeapDesc();
		direct_ctx->RHISetRenderTargets(0, nullptr,
			static_cast<XD3D12Texture2D*>(TextureDepthStencil.get())->GeDepthStencilView());
		direct_ctx->RHIClearMRT(false, true, nullptr, 0.0f, 0);

		pass_state_manager->SetShader<EShaderType::SV_Vertex>(&mShaders["PrePassVS"]);
		pass_state_manager->SetShader<EShaderType::SV_Pixel> (&mShaders["PrePassPS"]);
		pass_state_manager->SetShader<EShaderType::SV_Compute>(nullptr);

		for (size_t i = 0; i < mOpaqueRitems.size(); ++i)
		{
			auto& ri = mOpaqueRitems[i];
			direct_ctx->RHISetShaderConstantBuffer(EShaderType::SV_Vertex,
				 0,
				mFrameResource.get()->ObjectConstantBuffer[ri->ObjCBIndex].get());

			direct_ctx->RHISetShaderConstantBuffer(
				EShaderType::SV_Vertex, 1,
				mFrameResource.get()->PassConstantBuffer.get());

			mCommandList.Get()->IASetVertexBuffers(0, 1, GetRValuePtr(ri->Geo->VertexBufferView()));
			mCommandList.Get()->IASetIndexBuffer(GetRValuePtr(ri->Geo->IndexBufferView()));
			pass_state_manager->ApplyCurrentStateToPipeline<ED3D12PipelineType::D3D12PT_Graphics>();

			direct_ctx->GetCmdList()->CmdListFlushBarrier();
			mCommandList.Get()->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		}
		mCommandList->EndEvent();
	}
	
	//Pass2 HZB Pass
	{
		TShaderReference<XHZBPassCS> Shader = GetGlobalShaderMapping()->GetShader<XHZBPassCS>();
		XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
		SetComputePipelineStateFromCS(RHICmdList, ComputeShader);

		XD3D12TextureBase* HZBOutTex = GetD3D12TextureFromRHITexture(FurthestHZBOutput0.get());
		Shader->SetParameters(RHICmdList,
			XMFLOAT4(1.0 / 512.0, 1.0 / 512.0, 1.0, 1.0),
			TextureDepthStencil.get(),
			HZBOutTex->GeUnorderedAcessView(0),
			HZBOutTex->GeUnorderedAcessView(1),
			HZBOutTex->GeUnorderedAcessView(2),
			HZBOutTex->GeUnorderedAcessView(3),
			HZBOutTex->GeUnorderedAcessView(4)
		);
		RHICmdList.RHIDispatchComputeShader(512 / 16, 512 / 16, 1);
	}

	//Pass2.5 SkyAtmosPhere PreCompute
	{
		{
			TShaderReference<XRenderTransmittanceLutCS> Shader = GetGlobalShaderMapping()->GetShader<XRenderTransmittanceLutCS>();
			XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
			SetComputePipelineStateFromCS(RHICmdList, ComputeShader);

			XD3D12TextureBase* TransmittanceLutUAVTex = GetD3D12TextureFromRHITexture(TransmittanceLutUAV.get());
			Shader->SetParameters(RHICmdList, TransmittanceLutUAVTex->GeUnorderedAcessView(), RHICbSkyAtmosphere.get());
			RHICmdList.RHIDispatchComputeShader(256 / 8, 64 / 8, 1);
		}

		{
			TShaderReference<XRenderMultiScatteredLuminanceLutCS> Shader = GetGlobalShaderMapping()->GetShader<XRenderMultiScatteredLuminanceLutCS>();
			XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
			SetComputePipelineStateFromCS(RHICmdList, ComputeShader);
			
			XD3D12TextureBase* MultiScatteredLuminanceLutUAVTex = GetD3D12TextureFromRHITexture(MultiScatteredLuminanceLutUAV.get());
			Shader->SetParameters(RHICmdList, 
				MultiScatteredLuminanceLutUAVTex->GeUnorderedAcessView(), 
				RHICbSkyAtmosphere.get(),
				TransmittanceLutUAV.get());
			RHICmdList.RHIDispatchComputeShader(32 / 8, 32 / 8, 1);
		}

		//RenderSkyViewLutCS
		{
			TShaderReference<XRenderSkyViewLutCS> Shader = GetGlobalShaderMapping()->GetShader<XRenderSkyViewLutCS>();
			XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
			SetComputePipelineStateFromCS(RHICmdList, ComputeShader);

			XD3D12TextureBase* SkyViewLutUAVTex = GetD3D12TextureFromRHITexture(SkyViewLutUAV.get());
			Shader->SetParameters(RHICmdList,
				RViewInfo.ViewConstantBuffer.get(),
				RHICbSkyAtmosphere.get(),
				SkyViewLutUAVTex->GeUnorderedAcessView(),
				TransmittanceLutUAV.get(),
				MultiScatteredLuminanceLutUAV.get());
			RHICmdList.RHIDispatchComputeShader(192 / 8, 104 / 8, 1);
		}

		//RenderCameraAerialPerspectiveVolumeCS
		{
			TShaderReference<XRenderCameraAerialPerspectiveVolumeCS> Shader = GetGlobalShaderMapping()->GetShader<XRenderCameraAerialPerspectiveVolumeCS>();
			XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
			SetComputePipelineStateFromCS(RHICmdList, ComputeShader);
			
			XD3D12TextureBase* CameraAerialPerspectiveVolumeUAVTex = GetD3D12TextureFromRHITexture(CameraAerialPerspectiveVolumeUAV.get());
			Shader->SetParameters(RHICmdList,
				RViewInfo.ViewConstantBuffer.get(),
				RHICbSkyAtmosphere.get(),
				CameraAerialPerspectiveVolumeUAVTex->GeUnorderedAcessView(),
				TransmittanceLutUAV.get(),
				MultiScatteredLuminanceLutUAV.get());
			RHICmdList.RHIDispatchComputeShader(32 / 8, 32 / 8, 16 / 8);
		}

	}


	//Pass3 ShadowPass
	{
		mCommandList->BeginEvent(1, "ShadowPass", sizeof("ShadowPass"));
		mCommandList->SetPipelineState(ShadowPSO.Get());
		pass_state_manager->SetRootSignature(&ShadowPassRootSig);
		pass_state_manager->SetShader<EShaderType::SV_Vertex>(&mShaders["ShadowPassVS"]);
		pass_state_manager->SetShader<EShaderType::SV_Pixel> (&mShaders["ShadowPassPS"]);
		pass_state_manager->SetShader<EShaderType::SV_Compute>(nullptr);

		direct_ctx->RHISetRenderTargets(0, nullptr,
			static_cast<XD3D12Texture2D*>(ShadowTexture0.get())->GeDepthStencilView());
		direct_ctx->RHIClearMRT(false, true, nullptr, 0.0f, 0);
		
		for (uint32 CSMIndex = 0; CSMIndex < 4; CSMIndex++)
		{
			direct_ctx->RHISetViewport(CSMIndex * ShadowViewportWidth, 0.0f, 0.0f, 
				CSMIndex * ShadowViewportWidth+ShadowViewportWidth, 
				ShadowViewportWidth, 1.0f);

			for (size_t i = 0; i < mOpaqueRitems.size(); ++i)
			{
				auto& ri = mOpaqueRitems[i];
				direct_ctx->RHISetShaderConstantBuffer(
					EShaderType::SV_Vertex, 0,
					mFrameResource.get()->ObjectConstantBuffer[ri->ObjCBIndex].get());

				direct_ctx->RHISetShaderConstantBuffer(
					EShaderType::SV_Vertex, 1,
					ShadowPassConstantBuffer[CSMIndex].get());

				mCommandList.Get()->IASetVertexBuffers(0, 1, GetRValuePtr((ri->Geo->VertexBufferView())));
				mCommandList.Get()->IASetIndexBuffer(GetRValuePtr(ri->Geo->IndexBufferView()));
				pass_state_manager->ApplyCurrentStateToPipeline<ED3D12PipelineType::D3D12PT_Graphics>();
				mCommandList.Get()->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
			}

		}
		mCommandList->EndEvent();
	}



	//Pass4 GBufferPass BasePass
	{
		XRHITexture* RTTextures[4];
		RTTextures[0] = TextureGBufferA.get();
		RTTextures[1] = TextureGBufferB.get();
		RTTextures[2] = TextureGBufferC.get();
		RTTextures[3] = TextureGBufferD.get();
		XRHIRenderPassInfo RTInfos(4, RTTextures,ERenderTargetLoadAction::EClear, TextureDepthStencil.get(), EDepthStencilLoadAction::ELoad);
		RHICmdList.RHIBeginRenderPass(RTInfos, L"GBufferPass");
		RHICmdList.CacheActiveRenderTargets(RTInfos);
		{
			XGraphicsPSOInitializer_WithoutRT PassState;
			{
				//PassShaders
				XRHIBoundShaderStateInput_WithoutRT PassShaders;
				{
					//GetBasePassShaders
					XMaterialShaderInfo_Set ShaderInfos;
					XMaterialShader_Set XShaders;

					ShaderInfos.ShaderInfoSet[(int)EShaderType::SV_Compute] = nullptr;
					ShaderInfos.ShaderInfoSet[(int)EShaderType::SV_Vertex] = &TBasePassVS<false>::StaticShaderInfos;
					ShaderInfos.ShaderInfoSet[(int)EShaderType::SV_Pixel] = &TBasePassPS<false>::StaticShaderInfos;


					SphereMaterial.GetShaderInfos(ShaderInfos, XShaders);

					TShaderReference<TBasePassVS<false>> BaseVertexShader = TShaderReference<TBasePassVS<false>>(
						static_cast<TBasePassVS<false>*>(XShaders.XShaderSet[(int32)EShaderType::SV_Vertex]), XShaders.ShaderMap);

					TShaderReference<TBasePassPS<false>> BasePixelShader = TShaderReference<TBasePassPS<false>>(
						static_cast<TBasePassPS<false>*>(XShaders.XShaderSet[(int32)EShaderType::SV_Pixel]), XShaders.ShaderMap);


					std::shared_ptr<XRHIVertexLayout> RefVertexLayout = LocalVertexFactory.GetLayout(ELayoutType::Layout_Default);
					PassShaders.RHIVertexLayout = RefVertexLayout.get();

					PassShaders.MappingRHIVertexShader = BaseVertexShader.GetShaderMappingFileUnit()->GetRefShaderMapStoreRHIShaders();
					PassShaders.IndexRHIVertexShader = BaseVertexShader->GetRHIShaderIndex();

					PassShaders.MappingRHIPixelShader = BasePixelShader.GetShaderMappingFileUnit()->GetRefShaderMapStoreRHIShaders();
					PassShaders.IndexRHIPixelShader = BasePixelShader->GetRHIShaderIndex();
				}
				PassState.BoundShaderState = PassShaders;
				PassState.BlendState = TStaticBlendState<>::GetRHI();
				PassState.DepthStencilState = TStaticDepthStencilState<true,ECompareFunction::CF_GreaterEqual>::GetRHI();
			}

			//SubmitDraw
			{
				XGraphicsPSOInitializer PSOInitializer = PassState.TransToGraphicsPSOInitializer();
				RHICmdList.ApplyCachedRenderTargets(PSOInitializer);

				SetGraphicsPipelineStateFromPSOInit(RHICmdList, PSOInitializer);
			}
		}

		for (size_t i = 0; i < mOpaqueRitems.size(); ++i)
		{
			auto& ri = mOpaqueRitems[i];

			direct_ctx->RHISetShaderConstantBuffer(
				EShaderType::SV_Vertex, 0,
				mFrameResource.get()->ObjectConstantBuffer[ri->ObjCBIndex].get());
			
			direct_ctx->RHISetShaderConstantBuffer(
				EShaderType::SV_Vertex, 1,
				mFrameResource.get()->PassConstantBuffer.get());
		

			direct_ctx->RHISetShaderTexture(EShaderType::SV_Pixel, 0,
				ri->Mat->TextureBaseColor.get());
			direct_ctx->RHISetShaderTexture(EShaderType::SV_Pixel, 1,
				ri->Mat->TextureNormal.get());
			direct_ctx->RHISetShaderTexture(EShaderType::SV_Pixel, 2,
				ri->Mat->TextureRoughness.get());
			
			direct_ctx->RHISetShaderConstantBuffer(
				EShaderType::SV_Pixel,
				0,
				mFrameResource.get()->MaterialConstantBuffer[ri->Mat->MatCBIndex].get());


			mCommandList.Get()->IASetVertexBuffers(0, 1, GetRValuePtr(ri->Geo->VertexBufferView()));
			mCommandList.Get()->IASetIndexBuffer(GetRValuePtr(ri->Geo->IndexBufferView()));
			pass_state_manager->ApplyCurrentStateToPipeline<ED3D12PipelineType::D3D12PT_Graphics>();

			mCommandList.Get()->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		}
		pass_state_manager->ResetState();
		mCommandList->EndEvent();
	}
	
	//Pass5 Shadow Mask Pass
	{
		XRHITexture* SceneColorRTs = ShadowMaskTexture.get();
		XRHIRenderPassInfo RPInfos(1, &SceneColorRTs, ERenderTargetLoadAction::EClear, nullptr, EDepthStencilLoadAction::ENoAction);
		RHICmdList.RHIBeginRenderPass(RPInfos, L"ShadowMaskPass");
		RHICmdList.CacheActiveRenderTargets(RPInfos);

		XGraphicsPSOInitializer GraphicsPSOInit;
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();;
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();

		TShaderReference<RFullScreenQuadVS> VertexShader = GetGlobalShaderMapping()->GetShader<RFullScreenQuadVS>();
		TShaderReference<XShadowMaskPassPS> PixelShader = GetGlobalShaderMapping()->GetShader<XShadowMaskPassPS>();
		GraphicsPSOInit.BoundShaderState.RHIVertexShader = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.RHIPixelShader = PixelShader.GetPixelShader();
		GraphicsPSOInit.BoundShaderState.RHIVertexLayout = GFullScreenLayout.RHIVertexLayout.get();

		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		SetGraphicsPipelineStateFromPSOInit(RHICmdList, GraphicsPSOInit);

		PixelShader->SetParameter(RHICmdList,
			RViewInfo.ViewConstantBuffer.get(),
			ShadowTexture0.get(),
			TextureGBufferA.get(),
			TextureDepthStencil.get());

		RHICmdList.SetVertexBuffer(GFullScreenVertexRHI.RHIVertexBuffer.get(), 0, 0);
		
		for (int CSMindex = 3; CSMindex >= 0; CSMindex--)
		{
			PixelShader->SetShaderShadowMatrixBuffer(RHICmdList,
				ShadowMaskNoCommonConstantBuffer[CSMindex].get());
			
			RHICmdList.RHIDrawIndexedPrimitive(GFullScreenIndexRHI.RHIIndexBuffer.get(), 6, 1, 0, 0, 0);
		}

		mCommandList->EndEvent();
	}

	//Pass6 LightPass
	{
		{
			XRHITexture* SceneColorRTs = TextureSceneColorDeffered.get();
			XRHIRenderPassInfo RPInfos(1, &SceneColorRTs, ERenderTargetLoadAction::EClear, nullptr, EDepthStencilLoadAction::ENoAction);
			RHICmdList.RHIBeginRenderPass(RPInfos, L"LightPass");
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
			PixelShader->SetParameter(RHICmdList,
				RViewInfo.ViewConstantBuffer.get(),
				RHIcbDefferedLight.get(),
				TextureGBufferA.get(),
				TextureGBufferB.get(),
				TextureGBufferC.get(),
				TextureGBufferD.get(),
				TextureDepthStencil.get(),
				ShadowMaskTexture.get());

		}

		RHICmdList.SetVertexBuffer(GFullScreenVertexRHI.RHIVertexBuffer.get(), 0, 0);
		RHICmdList.RHIDrawIndexedPrimitive(GFullScreenIndexRHI.RHIIndexBuffer.get(), 6, 1, 0, 0, 0);
		mCommandList->EndEvent();
	}

	//Pass7 SSRPass
	{
		XRHITexture* SSRRTs = SSROutput.get();
		XRHIRenderPassInfo RPInfos(1, &SSRRTs, ERenderTargetLoadAction::EClear, nullptr, EDepthStencilLoadAction::ENoAction);
		RHICmdList.RHIBeginRenderPass(RPInfos, L"SSRPassPS");
		RHICmdList.CacheActiveRenderTargets(RPInfos);

		XGraphicsPSOInitializer GraphicsPSOInit;
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();;
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();

		TShaderReference<RFullScreenQuadVS> SSRVertexShader = GetGlobalShaderMapping()->GetShader<RFullScreenQuadVS>();
		TShaderReference<XSSRPassPS> SSRPixelShader = GetGlobalShaderMapping()->GetShader<XSSRPassPS>();
		GraphicsPSOInit.BoundShaderState.RHIVertexShader = SSRVertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.RHIPixelShader = SSRPixelShader.GetPixelShader();
		GraphicsPSOInit.BoundShaderState.RHIVertexLayout = GFullScreenLayout.RHIVertexLayout.get();

		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		SetGraphicsPipelineStateFromPSOInit(RHICmdList, GraphicsPSOInit);
		SSRPixelShader->SetParameter(RHICmdList,
			XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
			RViewInfo.ViewConstantBuffer.get(),
			TextureSceneColorDeffered.get(),
			TextureGBufferA.get(),
			TextureGBufferB.get(),
			TextureGBufferC.get(),
			TextureGBufferD.get(),
			TextureDepthStencil.get(),
			FurthestHZBOutput0.get());

		RHICmdList.SetVertexBuffer(GFullScreenVertexRHI.RHIVertexBuffer.get(), 0, 0);
		RHICmdList.RHIDrawIndexedPrimitive(GFullScreenIndexRHI.RHIIndexBuffer.get(), 6, 1, 0, 0, 0);

		mCommandList->EndEvent();

	}

	//Pass8 ReflectionEnvironment Pass
	{
		XRHITexture* TextureSceneColor = TextureSceneColorDeffered.get();
		XRHIRenderPassInfo RPInfos(1, &TextureSceneColor, ERenderTargetLoadAction::ELoad, nullptr, EDepthStencilLoadAction::ENoAction);
		RHICmdList.RHIBeginRenderPass(RPInfos, L"ReflectionEnvironmentPass");
		RHICmdList.CacheActiveRenderTargets(RPInfos);

		XGraphicsPSOInitializer GraphicsPSOInit;
		GraphicsPSOInit.BlendState = TStaticBlendState<true, EBlendOperation::BO_Add, EBlendFactor::BF_One, EBlendFactor::BF_One>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();

		TShaderReference<RFullScreenQuadVS> VertexShader = GetGlobalShaderMapping()->GetShader<RFullScreenQuadVS>();
		TShaderReference<XReflectionEnvironmentPS> PixelShader = GetGlobalShaderMapping()->GetShader<XReflectionEnvironmentPS>();
		GraphicsPSOInit.BoundShaderState.RHIVertexShader = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.RHIPixelShader = PixelShader.GetPixelShader();
		GraphicsPSOInit.BoundShaderState.RHIVertexLayout = GFullScreenLayout.RHIVertexLayout.get();

		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		SetGraphicsPipelineStateFromPSOInit(RHICmdList, GraphicsPSOInit);
		PixelShader->SetParameter(RHICmdList,
			SSROutput.get()
		);

		RHICmdList.SetVertexBuffer(GFullScreenVertexRHI.RHIVertexBuffer.get(), 0, 0);
		RHICmdList.RHIDrawIndexedPrimitive(GFullScreenIndexRHI.RHIIndexBuffer.get(), 6, 1, 0, 0, 0);

		mCommandList->EndEvent();
	}

	//Pass8 SkyAtmosphere Combine Pass
	{

		XRHITexture* TextureSceneColor = TextureSceneColorDeffered.get();
		XRHIRenderPassInfo RPInfos(1, &TextureSceneColor, ERenderTargetLoadAction::ELoad, nullptr, EDepthStencilLoadAction::ENoAction);
		RHICmdList.RHIBeginRenderPass(RPInfos, L"ReflectionEnvironmentPass");
		RHICmdList.CacheActiveRenderTargets(RPInfos);

		XGraphicsPSOInitializer GraphicsPSOInit;
		GraphicsPSOInit.BlendState = TStaticBlendState<
			true,
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
		PixelShader->SetParameter(RHICmdList,
			RViewInfo.ViewConstantBuffer.get(),
			RHICbSkyAtmosphere.get(),
			SkyViewLutUAV.get(),
			TextureDepthStencil.get(),
			TransmittanceLutUAV.get()
		);

		RHICmdList.SetVertexBuffer(GFullScreenVertexRHI.RHIVertexBuffer.get(), 0, 0);
		RHICmdList.RHIDrawIndexedPrimitive(GFullScreenIndexRHI.RHIIndexBuffer.get(), 6, 1, 0, 0, 0);

		mCommandList->EndEvent();

	}




	//ToneMapping
	{
		XRHITexture* TextureSceneColor = TextureSceneColorDefferedPingPong.get();
		XRHIRenderPassInfo RPInfos(1, &TextureSceneColor, ERenderTargetLoadAction::ELoad, nullptr, EDepthStencilLoadAction::ENoAction);
		RHICmdList.RHIBeginRenderPass(RPInfos, L"PostProcess_ToneMapping");
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
			
	}

	XRHITexture* CurrentPingPongTextureSceneColorTarget = TextureSceneColorDefferedPingPong.get();
	{

		//ImGUI Begin
		EditorUI.ImGui_Impl_RHI_NewFrame(&RHICmdList);
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		bool show_demo_window = true;
		EditorUI.OnTick();
		//ImGui::ShowDemoWindow(&show_demo_window);
		ImGui::Render();
		EditorUI.ImGui_Impl_RHI_RenderDrawData(ImGui::GetDrawData(), &RHICmdList, CurrentPingPongTextureSceneColorTarget);
		////ImGUI End
	}

	//Pass9 FinalPass
	{
		mCommandList->BeginEvent(1, "FinalPass", sizeof("FinalPass"));
		mCommandList->SetPipelineState(FullScreenPSO.Get());
		pass_state_manager->SetRootSignature(&FullScreenRootSig);
		pass_state_manager->SetShader<EShaderType::SV_Vertex>(&mShaders["fullScreenVS"]);
		pass_state_manager->SetShader<EShaderType::SV_Pixel> (&mShaders["fullScreenPS"]);
		pass_state_manager->SetShader<EShaderType::SV_Compute>(nullptr);
		
		direct_ctx->RHISetViewport(0.0f, 0.0f, 0.0f, mClientWidth, mClientHeight, 1.0f);
		
		RTViews[0] = viewport.GetCurrentBackRTView();
		direct_ctx->RHISetRenderTargets(1, RTViews, nullptr);//TODO
		direct_ctx->RHIClearMRT(true, false, clear_color, 0.0f, 0);
		
		direct_ctx->RHISetShaderTexture(EShaderType::SV_Pixel, 0, TextureSceneColorDefferedPingPong.get());
		
		RHICmdList.SetVertexBuffer(GFullScreenVertexRHI.RHIVertexBuffer.get(), 0, 0);
		RHICmdList.RHIDrawIndexedPrimitive(GFullScreenIndexRHI.RHIIndexBuffer.get(), 6, 1, 0, 0, 0);
		mCommandList->EndEvent();




		
	}



	{
		XD3D12PlatformRHI::TransitionResource(
			*direct_ctx->GetCmdList(),
			viewport.GetCurrentBackRTView(),
			D3D12_RESOURCE_STATE_PRESENT);

		direct_ctx->GetCmdList()->CmdListFlushBarrier();
		mCommandList->EndEvent();

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
    if((btnState & MK_LBUTTON) != 0)
    {
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		cam_ins.Pitch(dy);
		cam_ins.RotateY(dx);
    }
    

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}
 
void CrateApp::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();

	if (GetAsyncKeyState('W') & 0x8000)
		cam_ins.Walk(10.0f * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		cam_ins.Walk(-10.0f * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		cam_ins.Strafe(-10.0f * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		cam_ins.Strafe(10.0f * dt);
}
 
void CrateApp::UpdateShadowTransform(const GameTimer& gt)
{

}
void CrateApp::UpdateCamera(const GameTimer& gt)
{
	// Convert Spherical to Cartesian coordinates.
	//mEyePos.x = mRadius*sinf(mPhi)*cosf(mTheta);
	//mEyePos.z = mRadius*sinf(mPhi)*sinf(mTheta);
	//mEyePos.y = mRadius*cosf(mPhi);
	ViewMatrix.UpdateViewMatrix(cam_ins.GetEyePosition(), cam_ins.GetTargetPosition());

	//------------------------------------
	float NearFarLength = Far - Near;
	
	int CascadeCount = 4;
	float  Exp = 4.0f;
	float Scale = 1.2;
	float Piece = (1.0 - pow(Exp, CascadeCount)) / (1.0 - Exp);

	float FarPre = Near;
	float FrustumScale = 1.0 / Piece;
	for (int i = 0; i < CascadeCount; ++i)
	{
		
		float NearNew = FarPre;
		float FarNew = NearFarLength * FrustumScale * Scale;
		FarPre = FarNew;

		FrustumScale = FrustumScale * Exp;

		//float Far0 = NearFarLength * (1.0 / 85.0) * Scale;
		BoundSphere BoundSphere0 = BuildBoundSphere(FoVAngleY, AspectRatio(), NearNew, FarNew);

		//Compute Project Matrix
		float l = BoundSphere0.Center.x - BoundSphere0.Radius;
		float r = BoundSphere0.Center.x + BoundSphere0.Radius;
		float t = BoundSphere0.Center.y + BoundSphere0.Radius;
		float b = BoundSphere0.Center.y - BoundSphere0.Radius;
		float n = BoundSphere0.Center.z - BoundSphere0.Radius;
		float f = BoundSphere0.Center.z + BoundSphere0.Radius;
		XMMATRIX lightProj = XDirectx::XXMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
		XMStoreFloat4x4(&mLightProj, lightProj);

		//Compute Light Pos
		XMVECTOR LightDirCom = DirectX::XMVector3Normalize(XMLoadFloat3(&LightDir));
		XMVECTOR lightPos = XMLoadFloat3(&BoundSphere0.Center) + LightDirCom * BoundSphere0.Radius * 1.1;
		XVector3 lightPosStore; XMStoreFloat3(&lightPosStore, lightPos);
		LightMatrix[i].Create(mLightProj, lightPosStore, BoundSphere0.Center);
	}

	
}

void CrateApp::AnimateMaterials(const GameTimer& gt)
{
	
}

void CrateApp::UpdateObjectCBs(const GameTimer& gt)
{
	for(auto& e : mAllRitems)
	{
		XMMATRIX world = XMLoadFloat4x4(&e->World);
		//XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
		//XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
		mFrameResource.get()->ObjectConstantBuffer[e->ObjCBIndex].get()->
			UpdateData(&objConstants, sizeof(ObjectConstants), 0);
	}

}

void CrateApp::UpdateMaterialCBs(const GameTimer& gt)
{
	for(auto& e : mMaterials)
	{
		Material* mat = e.second.get();
	
		MaterialConstants matConstants;
		matConstants.Metallic = mat->Metallic;
		matConstants.Specular = mat->Specular;
		matConstants.Roughness = mat->Roughness;
		matConstants.TextureScale = mat->TextureScale;
		mFrameResource.get()->MaterialConstantBuffer[mat->MatCBIndex].get()->
			UpdateData(&matConstants, sizeof(MaterialConstants), 0);
	}
}

void CrateApp::UpdateMainPassCB(const GameTimer& gt)
{

	//for deffered light pass
	cbDefferedLightIns.LightDir = LightDir;
	cbDefferedLightIns.LightColorAndIntensityInLux = XMFLOAT4(LightColor.x, LightColor.y, LightColor.z, LightIntensity);
	RHIcbDefferedLight->UpdateData(&cbDefferedLightIns, sizeof(cbDefferedLight), 0);

	//BasePass
	memcpy(&mMainPassCB.ViewProj, &ViewMatrix.GetViewProjectionMatrixTranspose(), sizeof(DirectX::XMFLOAT4X4));
	mFrameResource->PassConstantBuffer.get()->UpdateData(&mMainPassCB, sizeof(PassConstants), 0);
	
	FrameNum++;
	RViewInfo.ViewCBCPUData.StateFrameIndexMod8 = FrameNum % 8;

	//Current For LightPass , will combine to BasePass for future
	RViewInfo.ViewCBCPUData.ViewSizeAndInvSize = XMFLOAT4(mClientWidth, mClientHeight, 1.0 / mClientWidth, 1.0 / mClientHeight);
	float LenSqrt = sqrt(LightDir.x * LightDir.x + LightDir.y * LightDir.y + LightDir.z * LightDir.z);
	RViewInfo.ViewCBCPUData.AtmosphereLightDirection = XMFLOAT4(LightDir.x / LenSqrt, LightDir.y / LenSqrt, LightDir.z / LenSqrt, 1.0f);
	RViewInfo.ViewCBCPUData.ViewToClip = ViewMatrix.GetProjectionMatrixTranspose();
	
	RViewInfo.ViewCBCPUData.TranslatedViewProjectionMatrix = ViewMatrix.GetTranslatedViewProjectionMatrixTranspose();
	RViewInfo.ViewCBCPUData.ScreenToWorld = ViewMatrix.GetScreenToWorldTranPose();
	RViewInfo.ViewCBCPUData.ScreenToTranslatedWorld = ViewMatrix.GetScreenToTranslatedWorldTranPose();
	RViewInfo.ViewCBCPUData.InvDeviceZToWorldZTransform = CreateInvDeviceZToWorldZTransform(ViewMatrix.GetProjectionMatrix());
	RViewInfo.ViewCBCPUData.WorldCameraOrigin = ViewMatrix.GetViewOrigin();

	
	//cbHZBins.DispatchThreadIdToBufferUV = XMFLOAT4(1.0 / 512.0, 1.0 / 512.0, 1.0, 1.0);
	//RHICbbHZB.get()->UpdateData(&cbHZBins, sizeof(cbHZB), 0);

	//Shadow Pass
	for (uint32 i = 0; i < 4; i++)
	{
		memcpy(&ShadowPassConstant[i].View, &LightMatrix[i].GetViewMatrixTranspose(), sizeof(XMatrix));
		memcpy(&ShadowPassConstant[i].Proj, &LightMatrix[i].GetProjectionMatrixTranspose(), sizeof(XMatrix));
		ShadowPassConstantBuffer[i].get()->UpdateData(&ShadowPassConstant[i], sizeof(ShadowPassConstants), 0);
	}


	RViewInfo.ViewCBCPUData.BufferSizeAndInvSize = XMFLOAT4(mClientWidth, mClientHeight, 1.0f / mClientWidth, 1.0f / mClientHeight);

	XMatrix TempProject = ViewMatrix.GetProjectionMatrix();
	XMatrix ScreenToClip = XMatrix::Identity;
	ScreenToClip.m[2][2] = TempProject.m[2][2];
	ScreenToClip.m[3][2] = TempProject.m[3][2];
	ScreenToClip.m[2][3] = 1.0f;
	ScreenToClip.m[3][3] = 0.0f;
	
	for (uint32 i = 0; i < 4; i++)
	{
		XMMATRIX ScreenToShadowMatrix = XMLoadFloat4x4(&ScreenToClip);
		ScreenToShadowMatrix = XMMatrixMultiply(ScreenToShadowMatrix, XMLoadFloat4x4(&ViewMatrix.GetViewProjectionMatrixInverse()));
		ScreenToShadowMatrix = XMMatrixMultiply(ScreenToShadowMatrix, XMLoadFloat4x4(&LightMatrix[i].GetViewProjectionMatrix()));
		XMStoreFloat4x4(&cbShadowMaskNoCommon[i].ScreenToShadowMatrix, XMMatrixTranspose(ScreenToShadowMatrix));
		cbShadowMaskNoCommon[i].x_offset = 0.25 * i;
		ShadowMaskNoCommonConstantBuffer[i].get()->UpdateData(&cbShadowMaskNoCommon[i], sizeof(cbShadowMaskNoCommonBuffer), 0);
	}

	//cbSkyAtmosphere
	{
		// All distance here are in kilometer and scattering/absorptions coefficient in 1/kilometers.
		const float EarthBottomRadius = 6360.0f;
		const float EarthTopRadius = 6420.0f;

		//SkyComponent SkyAtmosphereCommonData.cpp
		XMFLOAT4 RayleighScattering = XMFLOAT4(0.175287, 0.409607, 1, 1);
		float RayleighScatteringScale = 0.0331;

		float RayleighExponentialDistribution = 8.0f;
		float RayleighDensityExpScale = -1.0 / RayleighExponentialDistribution;

		XMFLOAT4 MieScattering = XMFLOAT4(1, 1, 1, 1);
		XMFLOAT4 MieAbsorption = XMFLOAT4(1, 1, 1, 1);

		// The altitude in kilometer at which Mie effects are reduced to 40%.
		float MieExponentialDistribution = 1.2;
		float MieScatteringScale = 0.003996;
		float MieAbsorptionScale = 0.000444;

		float MieDensityExpScale = -1.0f / MieExponentialDistribution;
		float TransmittanceSampleCount = 10.0f;
		float MultiScatteringSampleCount = 15.0f;


		//For RenderSkyViewLutCS
		const float CmToSkyUnit = 0.00001f;			// Centimeters to Kilometers
		const float SkyUnitToCm = 1.0f / 0.00001f;	// Kilometers to Centimeters

		XVector3 CametaWorldOrigin = cam_ins.GetEyePosition();
		XVector3 CametaTargetPos = cam_ins.GetTargetPosition();


		XMVECTOR Forward = XMLoadFloat3(GetRValuePtr(XVector3(CametaTargetPos.x - CametaWorldOrigin.x,
			CametaTargetPos.y - CametaWorldOrigin.y, CametaTargetPos.z - CametaWorldOrigin.z)));

		XMVECTOR PlanetCenterKm = XMLoadFloat3(GetRValuePtr(XVector3(0, -EarthBottomRadius, 0)));
		const float PlanetRadiusOffset = 0.005f;
		const float Offset = PlanetRadiusOffset * SkyUnitToCm;
		const float BottomRadiusWorld = EarthBottomRadius * SkyUnitToCm;
		const XMVECTOR PlanetCenterWorld = PlanetCenterKm * SkyUnitToCm;

		XMVECTOR CametaWorldOriginCom = XMLoadFloat3(&CametaWorldOrigin);
		const XMVECTOR PlanetCenterToCameraWorld = CametaWorldOriginCom - PlanetCenterWorld;

		XMFLOAT3 LengthCamToCenter;
		XMStoreFloat3(&LengthCamToCenter, XMVector3Length(PlanetCenterToCameraWorld));
		const float DistanceToPlanetCenterWorld = LengthCamToCenter.x;

		//X_Assert(DistanceToPlanetCenterWorld > (BottomRadiusWorld + Offset));
		// If the camera is below the planet surface, we snap it back onto the surface.
		XMVECTOR SkyWorldCameraOrigin = DistanceToPlanetCenterWorld < (BottomRadiusWorld + Offset)
			? PlanetCenterWorld + (BottomRadiusWorld + Offset) * (PlanetCenterToCameraWorld / DistanceToPlanetCenterWorld) :
			//XMLoadFloat3(&mEyePos) * SkyUnitToCm;
			XMLoadFloat3(&CametaWorldOrigin);

		XMFLOAT3 SkyPlanetCenter; XMStoreFloat3(&SkyPlanetCenter, PlanetCenterWorld);
		XMFLOAT3 SkyViewHeight; XMStoreFloat3(&SkyViewHeight, XMVector3Length(SkyWorldCameraOrigin - PlanetCenterWorld));

		XMStoreFloat3(&RViewInfo.ViewCBCPUData.SkyWorldCameraOrigin, SkyWorldCameraOrigin);
		RViewInfo.ViewCBCPUData.SkyPlanetCenterAndViewHeight = XMFLOAT4(SkyPlanetCenter.x, SkyPlanetCenter.y, SkyPlanetCenter.z, SkyViewHeight.x);

		XMVECTOR SkyUp = (SkyWorldCameraOrigin - PlanetCenterWorld) * CmToSkyUnit;
		SkyUp = XMVector3Normalize(SkyUp);

		//TODO
		//XMVECTOR SkyLeft = XMVector3Cross(Forward, SkyUp);
		XMVECTOR SkyLeft = XMVector3Cross(SkyUp, Forward);
		SkyLeft = XMVector3Normalize(SkyLeft);

		XVector3 DotMainDir;
		XMStoreFloat3(&DotMainDir, XMVectorAbs(XMVector3Dot(SkyUp, Forward)));
		if (DotMainDir.x > 0.999f)
		{
			XVector3 UpStore; XMStoreFloat3(&UpStore, SkyUp);
			const float Sign = UpStore.z >= 0.0f ? 1.0f : -1.0f;
			const float a = -1.0f / (Sign + UpStore.z);
			const float b = UpStore.x * UpStore.y * a;
			Forward = XMLoadFloat3(GetRValuePtr(XVector3(1 + Sign * a * pow(UpStore.x, 2.0f), Sign * b, -Sign * UpStore.x)));
			SkyLeft = XMLoadFloat3(GetRValuePtr(XVector3(b, Sign + a * pow(UpStore.y, 2.0f), -UpStore.y)));
		}
		else
		{
			Forward = XMVector3Cross(SkyUp, SkyLeft);
			Forward = XMVector3Normalize(Forward);
		}
		XMFLOAT4 SkyViewRow0; XMStoreFloat4(&SkyViewRow0, SkyLeft);
		XMFLOAT4 SkyViewRow1; XMStoreFloat4(&SkyViewRow1, SkyUp);
		XMFLOAT4 SkyViewRow2; XMStoreFloat4(&SkyViewRow2, Forward);

		XMFLOAT4X4 SkyViewLutReferentialTransposed(
			SkyViewRow0.x, SkyViewRow1.x, SkyViewRow2.x, 0,
			SkyViewRow0.y, SkyViewRow1.y, SkyViewRow2.y, 0,
			SkyViewRow0.z, SkyViewRow1.z, SkyViewRow2.z, 0,
			0, 0, 0, 0);

		RViewInfo.ViewCBCPUData.SkyViewLutReferential = SkyViewLutReferentialTransposed;
		

		//SkyAtmosphere Precompute
		cbSkyAtmosphereIns.TransmittanceLutSizeAndInvSize = XMFLOAT4(256.0, 64.0, 1.0 / 256.0, 1.0 / 64.0);
		cbSkyAtmosphereIns.MultiScatteredLuminanceLutSizeAndInvSize = XMFLOAT4(32.0, 32.0, 1.0 / 32.0, 1.0 / 32.0);
		cbSkyAtmosphereIns.SkyViewLutSizeAndInvSize = XMFLOAT4(192.0, 104.0, 1.0 / 192.0, 1.0 / 104.0);
		cbSkyAtmosphereIns.CameraAerialPerspectiveVolumeSizeAndInvSize = XMFLOAT4(32.0, 32.0, 1.0 / 32.0, 1.0 / 32.0);

		cbSkyAtmosphereIns.RayleighScattering = XMFLOAT4(
			RayleighScattering.x * RayleighScatteringScale,
			RayleighScattering.y * RayleighScatteringScale,
			RayleighScattering.z * RayleighScatteringScale,
			RayleighScattering.w * RayleighScatteringScale);

		cbSkyAtmosphereIns.MieScattering = XMFLOAT4(
			MieScattering.x * MieScatteringScale,
			MieScattering.y * MieScatteringScale,
			MieScattering.z * MieScatteringScale,
			MieScattering.w * MieScatteringScale);

		cbSkyAtmosphereIns.MieAbsorption = XMFLOAT4(
			MieAbsorption.x * MieAbsorptionScale,
			MieAbsorption.y * MieAbsorptionScale,
			MieAbsorption.z * MieAbsorptionScale,
			MieAbsorption.w * MieAbsorptionScale);

		cbSkyAtmosphereIns.MieExtinction = XMFLOAT4(
			cbSkyAtmosphereIns.MieScattering.x + cbSkyAtmosphereIns.MieAbsorption.x,
			cbSkyAtmosphereIns.MieScattering.y + cbSkyAtmosphereIns.MieAbsorption.y,
			cbSkyAtmosphereIns.MieScattering.z + cbSkyAtmosphereIns.MieAbsorption.z,
			cbSkyAtmosphereIns.MieScattering.w + cbSkyAtmosphereIns.MieAbsorption.w);

		cbSkyAtmosphereIns.BottomRadiusKm = EarthBottomRadius;
		cbSkyAtmosphereIns.TopRadiusKm = EarthTopRadius;
		cbSkyAtmosphereIns.MieDensityExpScale = MieDensityExpScale;
		cbSkyAtmosphereIns.RayleighDensityExpScale = RayleighDensityExpScale;
		cbSkyAtmosphereIns.TransmittanceSampleCount = TransmittanceSampleCount;
		cbSkyAtmosphereIns.MultiScatteringSampleCount = MultiScatteringSampleCount;
		cbSkyAtmosphereIns.GroundAlbedo = XMFLOAT4(0.66, 0.66, 0.66, 1.0);
		cbSkyAtmosphereIns.MiePhaseG = 0.8f;

		cbSkyAtmosphereIns.Light0Illuminance = XMFLOAT4(
			LightColor.x * LightIntensity, LightColor.y * LightIntensity, LightColor.z * LightIntensity, 0);

		cbSkyAtmosphereIns.CameraAerialPerspectiveVolumeDepthResolution = 16;
		cbSkyAtmosphereIns.CameraAerialPerspectiveVolumeDepthResolutionInv = 1.0 / 16.0;
		cbSkyAtmosphereIns.CameraAerialPerspectiveVolumeDepthSliceLengthKm = 96 / 16;

		RHICbSkyAtmosphere->UpdateData(&cbSkyAtmosphereIns, sizeof(cbSkyAtmosphere), 0);
	}
	



	
	RViewInfo.ViewConstantBuffer.get()->UpdateData(&RViewInfo.ViewCBCPUData, sizeof(ViewConstantBufferData), 0);

}


void CrateApp::LoadTextures()
{
	{
		int w, h, n;
		unsigned char* BaseColorData = stbi_load("E:/XEngine/XEnigine/Source/Shaders/T_Metal_Gold_D.TGA", &w, &h, &n, 0);
		if (n == 3) { X_Assert(false); }
		
		TextureMetalBaseColor = RHICreateTexture2D(w, h,1, false, false,
			EPixelFormat::FT_R8G8B8A8_UNORM_SRGB
			,ETextureCreateFlags(TexCreate_SRGB),1
			, BaseColorData);
		stbi_image_free(BaseColorData);
	}

	{
		int w_n, h_n, n_n;
		unsigned char* NormalMapData = stbi_load("E:/XEngine/XEnigine/Source/Shaders/T_Metal_Gold_N.TGA", &w_n, &h_n, &n_n, 0);
		int NormalTexSize = w_n * h_n * 4;
		unsigned char* FourChannelData = new unsigned char[NormalTexSize];
		X_Assert(n_n == 3);
		for (uint32 i = 0, k = 0; i < NormalTexSize; i += 4, k += 3)
		{
			FourChannelData[i + 0] = NormalMapData[k + 0];
			FourChannelData[i + 1] = NormalMapData[k + 1];
			FourChannelData[i + 2] = NormalMapData[k + 2];
			FourChannelData[i + 3] = 0b11111111;
		}
		TextureMetalNormal = RHICreateTexture2D(w_n, h_n, 1, false, false,
			EPixelFormat::FT_R8G8B8A8_UNORM
			,ETextureCreateFlags(TexCreate_None), 1
			, FourChannelData);
		stbi_image_free(NormalMapData);
		delete[] FourChannelData;
	}

	{
		int w_r, h_r, n_r;
		unsigned char* RoughnessMapData = stbi_load("E:/XEngine/XEnigine/Source/Shaders/T_MacroVariation.TGA", &w_r, &h_r, &n_r, 0);
		int RoughnessTexSize = w_r * h_r * 4;
		unsigned char* FourChannelData = new unsigned char[RoughnessTexSize];
		X_Assert(n_r == 3);
		for (uint32 i = 0, k = 0; i < RoughnessTexSize; i += 4, k += 3)
		{
			FourChannelData[i + 0] = RoughnessMapData[k + 0];
			FourChannelData[i + 1] = RoughnessMapData[k + 1];
			FourChannelData[i + 2] = RoughnessMapData[k + 2];
			FourChannelData[i + 3] = 0b11111111;
		}
		TextureRoughness = RHICreateTexture2D(w_r, h_r, 1, false, false,
			EPixelFormat::FT_R8G8B8A8_UNORM
			, ETextureCreateFlags(TexCreate_None), 1
			, FourChannelData);
		stbi_image_free(RoughnessMapData);
		delete[] FourChannelData;
	}

	{
		std::wstring FileName = XPath::ProjectResourceSavedDir() + L"/T_Rock_Sandstone_D.xasset";
		//std::shared_ptr<XArchiveBase>ArchiveWriterTex = XFileManagerGeneric::CreateFileWriter(FileName.c_str());
		std::shared_ptr<XArchiveBase>ArchiveReaderTex = XFileManagerGeneric::CreateFileReader(FileName.c_str());
		GTexture2D TextureWoodTexture;
		//TextureWoodTexture.LoadTextureFromImage("E:/XEngine/XEnigine/Source/Shaders/T_Rock_Sandstone_D.TGA");
		TextureWoodTexture.ArchiveImpl(*ArchiveReaderTex);
		TextureWoodTexture.CreateRHITexture(true);
		ArchiveReaderTex->Close();

		TextureWoodBaseColor = TextureWoodTexture.GetRHITexture2D();
	}
	
	{
		int w_n, h_n, n_n;
		unsigned char* NormalMapData = stbi_load("E:/XEngine/XEnigine/Source/Shaders/T_Rock_Sandstone_N.TGA", &w_n, &h_n, &n_n, 0);
		int NormalTexSize = w_n * h_n * 4;
		unsigned char* FourChannelData = new unsigned char[NormalTexSize];
		X_Assert(n_n == 3);
		for (uint32 i = 0, k = 0; i < NormalTexSize; i += 4, k += 3)
		{
			FourChannelData[i + 0] = NormalMapData[k + 0];
			FourChannelData[i + 1] = NormalMapData[k + 1];
			FourChannelData[i + 2] = NormalMapData[k + 2];
			FourChannelData[i + 3] = 0b11111111;
		}
		TextureWoodNormal = RHICreateTexture2D(w_n, h_n, 1, false, false,
			EPixelFormat::FT_R8G8B8A8_UNORM
			, ETextureCreateFlags(TexCreate_None), 1
			, FourChannelData);
		stbi_image_free(NormalMapData);
		delete[] FourChannelData;
	}


	{
		TextureGBufferA = RHICreateTexture2D(mClientWidth, mClientHeight, 1, false, false,
			EPixelFormat::FT_R16G16B16A16_FLOAT
			,ETextureCreateFlags(TexCreate_RenderTargetable), 1
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

		ShadowTexture0 = RHICreateTexture2D(ShadowMapWidth, ShadowMapHeight, 1, false, false,
			EPixelFormat::FT_R24G8_TYPELESS
			, ETextureCreateFlags(TexCreate_DepthStencilTargetable | TexCreate_ShaderResource), 1
			, nullptr);

		ShadowMaskTexture = RHICreateTexture2D(mClientWidth, mClientHeight, 1, false, false,
			EPixelFormat::FT_R8G8B8A8_UNORM
			, ETextureCreateFlags(TexCreate_RenderTargetable | TexCreate_ShaderResource), 1
			, nullptr);

		FurthestHZBOutput0 = RHICreateTexture2D(512, 512, 1, false, false,
			EPixelFormat::FT_R16_FLOAT
			, ETextureCreateFlags(TexCreate_UAV | TexCreate_ShaderResource), 5
			, nullptr);

		SSROutput = RHICreateTexture2D(mClientWidth, mClientHeight, 1, false, false,
			EPixelFormat::FT_R8G8B8A8_UNORM
			, ETextureCreateFlags(TexCreate_RenderTargetable | TexCreate_ShaderResource), 1
			, nullptr);

		TransmittanceLutUAV = RHICreateTexture2D(256, 64, 1, false, false,
			EPixelFormat::FT_R11G11B10_FLOAT
			, ETextureCreateFlags(TexCreate_UAV | TexCreate_ShaderResource), 1
			, nullptr);

		MultiScatteredLuminanceLutUAV = RHICreateTexture2D(32, 32, 1, false, false,
			EPixelFormat::FT_R11G11B10_FLOAT
			, ETextureCreateFlags(TexCreate_UAV | TexCreate_ShaderResource), 1
			, nullptr);

		SkyViewLutUAV = RHICreateTexture2D(192, 104, 1, false, false,
			EPixelFormat::FT_R11G11B10_FLOAT
			, ETextureCreateFlags(TexCreate_UAV | TexCreate_ShaderResource), 1
			, nullptr);

		CameraAerialPerspectiveVolumeUAV = RHICreateTexture3D(32, 32, 16,
			EPixelFormat::FT_R16G16B16A16_FLOAT
			, ETextureCreateFlags(TexCreate_UAV | TexCreate_ShaderResource), 1
			, nullptr);
	}

}

void CrateApp::BuildRootSignature()
{
	//PrePass
	{
		XPipelineRegisterBoundCount pipeline_register_count;
		memset(&pipeline_register_count, 0, sizeof(XPipelineRegisterBoundCount));
		
		pipeline_register_count.register_count[EShaderType_Underlying(EShaderType::SV_Vertex)].ConstantBufferCount
			= mShaders["PrePassVS"].GetCBVCount();
		PrePassRootSig.Create(&Device, pipeline_register_count);
	}

	{
		XPipelineRegisterBoundCount pipeline_register_count;
		memset(&pipeline_register_count, 0, sizeof(XPipelineRegisterBoundCount));

		pipeline_register_count.register_count[EShaderType_Underlying(EShaderType::SV_Vertex)].ConstantBufferCount
			= mShaders["ShadowPassVS"].GetCBVCount();

		ShadowPassRootSig.Create(&Device, pipeline_register_count);
	}

	//BasePass
	{
		XPipelineRegisterBoundCount pipeline_register_count;
		memset(&pipeline_register_count, 0, sizeof(XPipelineRegisterBoundCount));

		pipeline_register_count.register_count[EShaderType_Underlying(EShaderType::SV_Vertex)].ConstantBufferCount
			= mShaders["standardVS"].GetCBVCount();
		
		
		pipeline_register_count.register_count[EShaderType_Underlying(EShaderType::SV_Pixel)].ConstantBufferCount
			= mShaders["opaquePS"].GetCBVCount();
		pipeline_register_count.register_count[EShaderType_Underlying(EShaderType::SV_Pixel)].ShaderResourceCount
			= mShaders["opaquePS"].GetSRVCount();

		d3d12_root_signature.Create(&Device, pipeline_register_count);
	}

	//FullScreenPass
	{
		XPipelineRegisterBoundCount pipeline_register_count;
		memset(&pipeline_register_count, 0, sizeof(XPipelineRegisterBoundCount));
		pipeline_register_count.register_count[EShaderType_Underlying(EShaderType::SV_Pixel)].ShaderResourceCount
			= mShaders["fullScreenPS"].GetSRVCount();

		FullScreenRootSig.Create(&Device, pipeline_register_count);
	}
}

void CrateApp::BuildShadersAndInputLayout()
{
	{
		mShaders["PrePassVS"].CreateShader(EShaderType::SV_Vertex);
		mShaders["PrePassVS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/DepthOnlyVertexShader.hlsl", nullptr, "VS", "vs_5_1");
		mShaders["PrePassVS"].ShaderReflect();

		mShaders["PrePassPS"].CreateShader(EShaderType::SV_Pixel);
		mShaders["PrePassPS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/DepthOnlyVertexShader.hlsl", nullptr, "PS", "ps_5_1");
		mShaders["PrePassPS"].ShaderReflect();
	}

	{
		mShaders["ShadowPassVS"].CreateShader(EShaderType::SV_Vertex);
		mShaders["ShadowPassVS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/ShadowDepthShader.hlsl", nullptr, "VS", "vs_5_1");
		mShaders["ShadowPassVS"].ShaderReflect();

		mShaders["ShadowPassPS"].CreateShader(EShaderType::SV_Pixel);
		mShaders["ShadowPassPS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/ShadowDepthShader.hlsl", nullptr, "PS", "ps_5_1");
		mShaders["ShadowPassPS"].ShaderReflect();
	}

	{
		mShaders["standardVS"].CreateShader(EShaderType::SV_Vertex);
		mShaders["standardVS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/BasePassPixelShader.hlsl", nullptr, "VS", "vs_5_1");
		mShaders["standardVS"].ShaderReflect();

		mShaders["opaquePS"].CreateShader(EShaderType::SV_Pixel);
		mShaders["opaquePS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/BasePassPixelShader.hlsl", nullptr, "PS", "ps_5_1");
		mShaders["opaquePS"].ShaderReflect();
	}

	{
		mShaders["fullScreenVS"].CreateShader(EShaderType::SV_Vertex);
		mShaders["fullScreenVS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/fullScreen.hlsl", nullptr, "VS", "vs_5_1");
		mShaders["fullScreenVS"].ShaderReflect();

		mShaders["fullScreenPS"].CreateShader(EShaderType::SV_Pixel);
		mShaders["fullScreenPS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/fullScreen.hlsl", nullptr, "PS", "ps_5_1");
		mShaders["fullScreenPS"].ShaderReflect();
	}

	mBasePassLayout =
	{
		{ "ATTRIBUTE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "ATTRIBUTE", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "ATTRIBUTE", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "ATTRIBUTE", 3, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

    mFullScreenInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
}

void CrateApp::BuildShapeGeometry()
{
	//step1
    GeometryGenerator geoGen;
	{
		GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5, 20, 20);//geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
		GeometryGenerator::MeshData grid = geoGen.CreateGrid(10.0f, 15.0f, 60, 40);

		//step2
		UINT sphereVertexOffset = 0;
		UINT gridVertexOffset = (UINT)sphere.Vertices.size();

		UINT sphereIndexOffset = 0;
		UINT gridIndexOffset = (UINT)sphere.Indices32.size();

		//step3
		SubmeshGeometry sphereSubmesh;
		sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
		sphereSubmesh.StartIndexLocation = 0;
		sphereSubmesh.BaseVertexLocation = 0;

		SubmeshGeometry gridSubmesh;
		gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
		gridSubmesh.StartIndexLocation = gridIndexOffset;
		gridSubmesh.BaseVertexLocation = gridVertexOffset;

		//step4 vertices
		auto totalVertexCount =
			sphere.Vertices.size() +
			grid.Vertices.size();

		std::vector<Vertex> vertices(totalVertexCount);

		uint32 k = 0;
		for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
		{
			vertices[k].Position = DirectX::XMFLOAT4(
				sphere.Vertices[i].Position.x, 
				sphere.Vertices[i].Position.y, 
				sphere.Vertices[i].Position.z, 
				1.0f);
			
			vertices[k].TangentX = sphere.Vertices[i].TangentU;
			vertices[k].TangentZ = DirectX::XMFLOAT4(
				sphere.Vertices[i].Normal.x,
				sphere.Vertices[i].Normal.y,
				sphere.Vertices[i].Normal.z,
				1.0f);
			vertices[k].TexCoord = sphere.Vertices[i].TexC;
		}

		for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
		{
			vertices[k].Position = DirectX::XMFLOAT4(
				grid.Vertices[i].Position.x,
				grid.Vertices[i].Position.y,
				grid.Vertices[i].Position.z,
				1.0f);

			vertices[k].TangentX = grid.Vertices[i].TangentU;
			vertices[k].TangentZ = DirectX::XMFLOAT4(
				grid.Vertices[i].Normal.x,
				grid.Vertices[i].Normal.y,
				grid.Vertices[i].Normal.z,
				1.0f);
			vertices[k].TexCoord = grid.Vertices[i].TexC;
		}

		//step4 indices
		std::vector<std::uint16_t> indices;
		indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
		indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		auto geo = std::make_unique<MeshGeometry>();
		geo->Name = "shapeGeo";

		//CPU
		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
		CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
		CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		//GPU
		geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

		geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

		geo->VertexByteStride = sizeof(Vertex);
		geo->VertexBufferByteSize = vbByteSize;
		geo->IndexFormat = DXGI_FORMAT_R16_UINT;
		geo->IndexBufferByteSize = ibByteSize;

		geo->DrawArgs["sphere"] = sphereSubmesh;
		geo->DrawArgs["grid"] = gridSubmesh;
		mGeometries[geo->Name] = std::move(geo);
	}
	

}

void CrateApp::BuildPSOs()
{
	CompileGlobalShaderMap();

	//PrePass
	//ShadowPass 
	//BasePass 
	//FullScreenPass 
	 
	//PrePass
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC DepthOnlyPSODesc;

		ZeroMemory(&DepthOnlyPSODesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		DepthOnlyPSODesc.InputLayout = { mBasePassLayout.data(), (UINT)mBasePassLayout.size() };
		DepthOnlyPSODesc.pRootSignature = PrePassRootSig.GetDXRootSignature();
		DepthOnlyPSODesc.VS =
		{
			reinterpret_cast<BYTE*>(mShaders["PrePassVS"].GetByteCode()->GetBufferPointer()),
			mShaders["PrePassVS"].GetByteCode()->GetBufferSize()
		};
		DepthOnlyPSODesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["PrePassPS"].GetByteCode()->GetBufferPointer()),
			mShaders["PrePassPS"].GetByteCode()->GetBufferSize()
		};

		//TODO
		DepthOnlyPSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		DepthOnlyPSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		DepthOnlyPSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		DepthOnlyPSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;

		DepthOnlyPSODesc.SampleMask = UINT_MAX;
		DepthOnlyPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		DepthOnlyPSODesc.NumRenderTargets = 0;
		DepthOnlyPSODesc.SampleDesc.Count = 1;
		DepthOnlyPSODesc.SampleDesc.Quality = 0;
		DepthOnlyPSODesc.DSVFormat = mDepthStencilFormat;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&DepthOnlyPSODesc, IID_PPV_ARGS(&mDepthOnlyPSO)));
	}

	//ShadowPass
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC ShadowPSODesc;

		ZeroMemory(&ShadowPSODesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		ShadowPSODesc.InputLayout = { mBasePassLayout.data(), (UINT)mBasePassLayout.size() };
		ShadowPSODesc.pRootSignature = PrePassRootSig.GetDXRootSignature();
		ShadowPSODesc.VS =
		{
			reinterpret_cast<BYTE*>(mShaders["ShadowPassVS"].GetByteCode()->GetBufferPointer()),
			mShaders["ShadowPassVS"].GetByteCode()->GetBufferSize()
		};
		ShadowPSODesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["ShadowPassPS"].GetByteCode()->GetBufferPointer()),
			mShaders["ShadowPassPS"].GetByteCode()->GetBufferSize()
		};

		//TODO
		ShadowPSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		ShadowPSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		ShadowPSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		ShadowPSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;

		ShadowPSODesc.SampleMask = UINT_MAX;
		ShadowPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		ShadowPSODesc.NumRenderTargets = 0;
		ShadowPSODesc.SampleDesc.Count = 1;
		ShadowPSODesc.SampleDesc.Quality = 0;
		ShadowPSODesc.DSVFormat = mDepthStencilFormat;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&ShadowPSODesc, IID_PPV_ARGS(&ShadowPSO)));
	}

	SphereGMaterial.CreateGMaterial(XPath::ProjectMaterialSavedDir()+L"/Material.hlsl");
	SphereMaterial.BeginCompileShaderMap();
	//XD3D12PixelShader* BaseD3DPixelShader;
	//XD3D12VertexShader* BaseD3DVertexShader;
	//{
	//	
	//
	//
	//
	//	//PassShaders
	//	
	//	TShaderReference<TBasePassPS<false>> BasePixelShader;
	//	TShaderReference<TBasePassVS<false>> BaseVertexShader;
	//	//GetBasePassShaders
	//	{
	//		XMaterialShaderInfo_Set ShaderInfos;
	//		XMaterialShader_Set XShaders;
	//
	//		ShaderInfos.ShaderInfoSet[(int)EShaderType::SV_Compute] = nullptr;
	//		ShaderInfos.ShaderInfoSet[(int)EShaderType::SV_Vertex] = &TBasePassVS<false>::StaticShaderInfos;
	//		ShaderInfos.ShaderInfoSet[(int)EShaderType::SV_Pixel] = &TBasePassPS<false>::StaticShaderInfos;
	//
	//		
	//		SphereMaterial.GetShaderInfos(ShaderInfos, XShaders);
	//
	//		BasePixelShader = TShaderReference<TBasePassPS<false>>(
	//			static_cast<TBasePassPS<false>*>(XShaders.XShaderSet[(int32)EShaderType::SV_Pixel]),
	//			XShaders.ShaderMap);
	//
	//		BaseVertexShader = TShaderReference<TBasePassVS<false>>(
	//			static_cast<TBasePassVS<false>*>(XShaders.XShaderSet[(int32)EShaderType::SV_Vertex]),
	//			XShaders.ShaderMap);
	//	}
	//	XRHIPixelShader* RHIPixelShader = BasePixelShader.GetPixelShader();
	//	BaseD3DPixelShader = static_cast<XD3D12PixelShader*>(RHIPixelShader);
	//
	//	XRHIVertexShader* RHIVertexShader = BaseVertexShader.GetVertexShader();
	//	BaseD3DVertexShader = static_cast<XD3D12VertexShader*>(RHIVertexShader);
	//}

	

	//BasePass
	//{
	//	XRHIVertexLayout* BaseRHILayout = LocalVertexFactory.GetLayout(ELayoutType::Layout_Default).get();
	//	XD3D12VertexLayout* BassLayout = static_cast<XD3D12VertexLayout*>(BaseRHILayout);
	//	
	//
	//	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
	//
	//	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	//	opaquePsoDesc.InputLayout = { BassLayout->VertexElements.data(), (UINT)BassLayout->VertexElements.size()};
	//	opaquePsoDesc.pRootSignature = d3d12_root_signature.GetDXRootSignature();
	//	opaquePsoDesc.VS = BaseD3DVertexShader->D3DByteCode;
	//	//{
	//	//	reinterpret_cast<BYTE*>(mShaders["standardVS"].GetByteCode()->GetBufferPointer()),
	//	//	mShaders["standardVS"].GetByteCode()->GetBufferSize()
	//	//};
	//	opaquePsoDesc.PS = BaseD3DPixelShader->D3DByteCode;
	//	//{
	//	//	reinterpret_cast<BYTE*>(mShaders["opaquePS"].GetByteCode()->GetBufferPointer()),
	//	//	mShaders["opaquePS"].GetByteCode()->GetBufferSize()
	//	//};
	//
	//	//TODO
	//	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	//	opaquePsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	//
	//	opaquePsoDesc.SampleMask = UINT_MAX;
	//	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//	opaquePsoDesc.NumRenderTargets = 4;
	//	opaquePsoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	//	opaquePsoDesc.RTVFormats[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	//	opaquePsoDesc.RTVFormats[2] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	//	opaquePsoDesc.RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//	opaquePsoDesc.SampleDesc.Count = 1;
	//	opaquePsoDesc.SampleDesc.Quality = 0;
	//	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	//	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mOpaquePSO)));
	//}

	//FullScreenPass
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC FullScreenPsoDesc;

		ZeroMemory(&FullScreenPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		FullScreenPsoDesc.InputLayout = { mFullScreenInputLayout.data(), (UINT)mFullScreenInputLayout.size() };
		FullScreenPsoDesc.pRootSignature = FullScreenRootSig.GetDXRootSignature();
		FullScreenPsoDesc.VS =
		{
			reinterpret_cast<BYTE*>(mShaders["fullScreenVS"].GetByteCode()->GetBufferPointer()),
			mShaders["fullScreenVS"].GetByteCode()->GetBufferSize()
		};
		FullScreenPsoDesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["fullScreenPS"].GetByteCode()->GetBufferPointer()),
			mShaders["fullScreenPS"].GetByteCode()->GetBufferSize()
		};
		FullScreenPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		FullScreenPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		FullScreenPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		FullScreenPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		FullScreenPsoDesc.SampleMask = UINT_MAX;
		FullScreenPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		FullScreenPsoDesc.NumRenderTargets = 1;
		FullScreenPsoDesc.RTVFormats[0] = mBackBufferFormat;
		FullScreenPsoDesc.SampleDesc.Count = 1;
		FullScreenPsoDesc.SampleDesc.Quality = 0;
		FullScreenPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&FullScreenPsoDesc, IID_PPV_ARGS(&FullScreenPSO)));
	}
}


void CrateApp::BuildMaterials()
{
	{
		auto metalmat = std::make_unique<Material>();
		metalmat->Name = "metal";
		metalmat->MatCBIndex = 0;
		metalmat->Metallic = 0.8f;
		metalmat->Specular = 0.0;
		metalmat->Roughness = 0.4f;
		metalmat->TextureScale = 1.0f;
		metalmat->TextureBaseColor = TextureMetalBaseColor;
		metalmat->TextureNormal = TextureMetalNormal;
		metalmat->TextureRoughness = TextureRoughness;
		mMaterials["metal"] = std::move(metalmat);
	}

	{
		auto woodCrate = std::make_unique<Material>();
		woodCrate->Name = "wood";
		woodCrate->MatCBIndex = 1;
		woodCrate->Metallic = 0.0;
		woodCrate->Specular = 0.5;
		woodCrate->Roughness = 0.8f;
		woodCrate->TextureScale = 10.0f;
		woodCrate->TextureBaseColor = TextureWoodBaseColor;
		woodCrate->TextureNormal = TextureWoodNormal;
		woodCrate->TextureRoughness = TextureRoughness;
		mMaterials["wood"] = std::move(woodCrate);
	}

}

void CrateApp::BuildRenderItems()
{

	{
		auto sphereBackRitem = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&sphereBackRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(0.0f, -0.5f, 0.0f));
		//XMStoreFloat4x4(&sphereBackRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		sphereBackRitem->ObjCBIndex = 0;
		sphereBackRitem->Mat = mMaterials["metal"].get();
		sphereBackRitem->Geo = mGeometries["shapeGeo"].get();
		sphereBackRitem->IndexCount = sphereBackRitem->Geo->DrawArgs["sphere"].IndexCount;
		sphereBackRitem->StartIndexLocation = sphereBackRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
		sphereBackRitem->BaseVertexLocation = sphereBackRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;
		mAllRitems.push_back(std::move(sphereBackRitem));

		auto sphereRitem = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&sphereRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(0.0f, 1.5f, 0.0f));
		//XMStoreFloat4x4(&sphereRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		sphereRitem->ObjCBIndex = 1;
		sphereRitem->Mat = mMaterials["metal"].get();
		sphereRitem->Geo = mGeometries["shapeGeo"].get();
		sphereRitem->IndexCount = sphereRitem->Geo->DrawArgs["sphere"].IndexCount;
		sphereRitem->StartIndexLocation = sphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
		sphereRitem->BaseVertexLocation = sphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;
		mAllRitems.push_back(std::move(sphereRitem));


		auto gridRitem = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&gridRitem->World, XMMatrixTranslation(0.0f, 1.0f, 0.0f));
		//XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(8.0f, 1.0f, 8.0f) * XMMatrixTranslation(0.0f, 1.0f, 0.0f));
		gridRitem->ObjCBIndex = 2;
		gridRitem->Mat = mMaterials["wood"].get();
		gridRitem->Geo = mGeometries["shapeGeo"].get();
		gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
		gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
		gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
		mAllRitems.push_back(std::move(gridRitem));


		auto sphereRitem2 = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&sphereRitem2->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(2.0f, 1.5f, 0.0f));
		//XMStoreFloat4x4(&sphereRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		sphereRitem2->ObjCBIndex = 3;
		sphereRitem2->Mat = mMaterials["metal"].get();
		sphereRitem2->Geo = mGeometries["shapeGeo"].get();
		sphereRitem2->IndexCount = sphereRitem2->Geo->DrawArgs["sphere"].IndexCount;
		sphereRitem2->StartIndexLocation = sphereRitem2->Geo->DrawArgs["sphere"].StartIndexLocation;
		sphereRitem2->BaseVertexLocation = sphereRitem2->Geo->DrawArgs["sphere"].BaseVertexLocation;
		mAllRitems.push_back(std::move(sphereRitem2));
		// All the render items are opaque.
		for (auto& e : mAllRitems)
			mOpaqueRitems.push_back(e.get());
	}




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
	std::shared_ptr<XArchiveBase>ArchiveWriterTex= XFileManagerGeneric::CreateFileWriter(FileName.c_str());
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
	try
	{
		MainInit::Init();
		TestReflectAndArchive();

		CrateApp theApp;
		if (!theApp.Initialize())return 0;
		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
	
	_CrtDumpMemoryLeaks();
}



