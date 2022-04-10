#include "UIBackend.h"
#include <imgui.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") 
#endif

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/wstringize.hpp>

#include "Runtime/Core/Math/Math.h"
#include "Runtime/HAL/Mch.h"
#include "Runtime/RenderCore/GlobalShader.h"
#include "Runtime/RHI/RHIResource.h"
#include "Runtime/RHI/RHIStaticStates.h"

class RUIBackendVertexLayout : public XRenderResource
{
public:
	std::shared_ptr<XRHIVertexLayout> RHIVertexLayout;
	virtual void InitRHI()override
	{
		XRHIVertexLayoutArray LayoutArray;
		LayoutArray.push_back(XVertexElement(0, EVertexElementType::VET_Float2, 0, 0));
		LayoutArray.push_back(XVertexElement(1, EVertexElementType::VET_Float2, 0, 0 + sizeof(XVector2)));
		LayoutArray.push_back(XVertexElement(1, EVertexElementType::VET_Color, 0, 0 + sizeof(XVector2) + sizeof(XVector2)));
		RHIVertexLayout = RHICreateVertexLayout(LayoutArray);
	}

	virtual void ReleaseRHI()override
	{
		RHIVertexLayout.reset();
	}
};

TGlobalResource<RUIBackendVertexLayout>UIBackendVertexLayout;

class XUIBackendVS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XUIBackendVS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}

public:
	XUIBackendVS(const XShaderInitlizer& Initializer)
		:XGloablShader(Initializer)
	{

	}

	void SetParameter()
	{

	}
};

class XUIBackendPS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XUIBackendPS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}

public:
	XUIBackendPS(const XShaderInitlizer& Initializer)
		:XGloablShader(Initializer)
	{

	}

	void SetParameter()
	{

	}
};


;
XUIBackendVS::ShaderInfos XUIBackendVS::StaticShaderInfos(
	"XUIBackendVS", BOOST_PP_CAT(L, BOOST_PP_CAT(BOOST_PP_STRINGIZE(ROOT_DIR_XENGINE), "/Source/Shaders/UIBackend.hlsl")),
	"UI_VS", EShaderType::SV_Vertex, XUIBackendVS::CustomConstrucFunc,
	XUIBackendVS::ModifyShaderCompileSettings);

XUIBackendPS::ShaderInfos XUIBackendPS::StaticShaderInfos(
	"XUIBackendPS", BOOST_PP_CAT(L, BOOST_PP_CAT(BOOST_PP_STRINGIZE(ROOT_DIR_XENGINE), "/Source/Shaders/UIBackend.hlsl")),
	"UI_PS", EShaderType::SV_Pixel, XUIBackendPS::CustomConstrucFunc,
	XUIBackendPS::ModifyShaderCompileSettings);


struct ImGui_Impl_RHI_Data
{
	XGraphicsPSOInitializer* PSOInit;
	std::shared_ptr<XRHITexture2D>FontsTexture;

	UINT frameIndex;
	ImGui_Impl_RHI_Data() 
	{ 
		memset((void*)this, 0, sizeof(*this)); 
		frameIndex = UINT_MAX;
	}
};

static ImGui_Impl_RHI_Data* ImGui_Impl_RHI_GetBackendData()
{
	return ImGui::GetCurrentContext() ? (ImGui_Impl_RHI_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
}

void RHIUI::ImGui_Impl_RHI_Init()
{
	ImGuiIO& io = ImGui::GetIO();

	ImGui_Impl_RHI_Data* BackendData = new ImGui_Impl_RHI_Data();
	io.BackendRendererUserData = (void*)BackendData;
	io.BackendRendererName = "ImGui_Impl_RHI";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;


}

void RHIUI::ImGui_Impl_RHI_Shutdown()
{
	delete ImGui_Impl_RHI_GetBackendData()->PSOInit;
	delete ImGui_Impl_RHI_GetBackendData();
}

void CreatePSOInit()
{

}



void RHIUI::ImGui_Impl_RHI_NewFrame(XRHICommandList* RHICmdList)
{
	ImGuiIO& io = ImGui::GetIO();
	ImGui_Impl_RHI_Data* BackendData = ImGui_Impl_RHI_GetBackendData();
	if (BackendData->PSOInit == nullptr)
	{
		BackendData->PSOInit = new XGraphicsPSOInitializer();
		BackendData->PSOInit->BlendState = TStaticBlendState<>::GetRHI();
		BackendData->PSOInit->DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();

		TShaderReference<XUIBackendVS> VertexShader = GetGlobalShaderMapping()->GetShader<XUIBackendVS>();
		TShaderReference<XUIBackendPS> PixelShader = GetGlobalShaderMapping()->GetShader<XUIBackendPS>();
		BackendData->PSOInit->BoundShaderState.RHIVertexShader = VertexShader.GetVertexShader();
		BackendData->PSOInit->BoundShaderState.RHIPixelShader = PixelShader.GetPixelShader();

		BackendData->PSOInit->BoundShaderState.RHIVertexLayout = UIBackendVertexLayout.RHIVertexLayout.get();

		unsigned char* pixels; int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		
	}
}
