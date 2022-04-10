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
#include "Runtime/RHI/PipelineStateCache.h"
#include "Runtime/RenderCore/ShaderParameter.h"

class RUIBackendVertexLayout : public XRenderResource
{
public:
	std::shared_ptr<XRHIVertexLayout> RHIVertexLayout;
	virtual void InitRHI()override
	{
		XRHIVertexLayoutArray LayoutArray;
		LayoutArray.push_back(XVertexElement(0, EVertexElementType::VET_Float2, 0, 0));
		LayoutArray.push_back(XVertexElement(1, EVertexElementType::VET_Float2, 0, 0 + sizeof(XVector2)));
		LayoutArray.push_back(XVertexElement(2, EVertexElementType::VET_Color, 0, 0 + sizeof(XVector2) + sizeof(XVector2)));
		RHIVertexLayout = RHICreateVertexLayout(LayoutArray);
	}

	virtual void ReleaseRHI()override
	{
		RHIVertexLayout.reset();
	}
};

TGlobalResource<RUIBackendVertexLayout>UIBackendVertexLayout;

struct VERTEX_CONSTANT_BUFFER
{
	float   mvp[4][4];
};

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
		ProjectionMatrix.Bind(Initializer.ShaderParameterMap, "ProjectionMatrix");
	}

	void SetParameter(
		XRHICommandList& RHICommandList, 
		VERTEX_CONSTANT_BUFFER& MatIn)
	{
		SetShaderValue(RHICommandList, EShaderType::SV_Vertex, ProjectionMatrix, MatIn);
	}

	XShaderVariableParameter ProjectionMatrix;
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
		UITexture.Bind(Initializer.ShaderParameterMap, "texture0");
	}

	void SetParameter(
		XRHICommandList& RHICommandList,
		XRHITexture* InUITexture)
	{
		SetTextureParameter(RHICommandList, EShaderType::SV_Pixel, UITexture, InUITexture);
	}
	TextureParameterType UITexture;
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


struct RHIUI_VBIB
{
	std::shared_ptr<XRHIVertexBuffer>VertexBuffer;
	std::shared_ptr<XRHIIndexBuffer>IndexBuffer;

	int VertexBufferSize;
	int IndexBufferSize;
};



struct RHIUI_RenderBuffer
{
	RHIUI_VBIB VBIB;
};

struct ImGui_Impl_RHI_Data
{
	RHIUI_RenderBuffer* RenderBuffer;
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

	BackendData->RenderBuffer = new RHIUI_RenderBuffer();
	BackendData->RenderBuffer->VBIB.VertexBufferSize = 10000;
	BackendData->RenderBuffer->VBIB.IndexBufferSize = 5000;


}

void RHIUI::ImGui_Impl_RHI_Shutdown()
{
	ImGui_Impl_RHI_Data* BackendData = ImGui_Impl_RHI_GetBackendData();
	delete BackendData->RenderBuffer;
	delete BackendData->PSOInit;
	delete BackendData;
}


void RHIUI::ImGui_Impl_RHI_NewFrame(XRHICommandList* RHICmdList)
{
	ImGuiIO& io = ImGui::GetIO();
	ImGui_Impl_RHI_Data* BackendData = ImGui_Impl_RHI_GetBackendData();
	if (BackendData->PSOInit == nullptr)
	{
		BackendData->PSOInit = new XGraphicsPSOInitializer();
		BackendData->PSOInit->BlendState = TStaticBlendState<
			true,
			EBlendOperation::BO_Add,
			EBlendFactor::BF_SourceAlpha,
			EBlendFactor::BF_InverseSourceAlpha,
			EBlendOperation::BO_Add,
			EBlendFactor::BF_One,
			EBlendFactor::BF_InverseSourceAlpha
		>::GetRHI();
		BackendData->PSOInit->DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();

		TShaderReference<XUIBackendVS> VertexShader = GetGlobalShaderMapping()->GetShader<XUIBackendVS>();
		TShaderReference<XUIBackendPS> PixelShader = GetGlobalShaderMapping()->GetShader<XUIBackendPS>();
		BackendData->PSOInit->BoundShaderState.RHIVertexShader = VertexShader.GetVertexShader();
		BackendData->PSOInit->BoundShaderState.RHIPixelShader = PixelShader.GetPixelShader();

		BackendData->PSOInit->BoundShaderState.RHIVertexLayout = UIBackendVertexLayout.RHIVertexLayout.get();


		
		unsigned char* pixels; int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		BackendData->FontsTexture = RHICreateTexture2D(width, height, 1, false, false, EPixelFormat::FT_R8G8B8A8_UNORM, ETextureCreateFlags(TexCreate_None), 1, pixels);
	}


}

static void ImGui_Impl_RHI_SetupRenderState(ImDrawData* draw_data, XRHICommandList* RHICmdList, XRHITexture* SceneColorTex)
{
	ImGui_Impl_RHI_Data* BackendData = ImGui_Impl_RHI_GetBackendData();
	VERTEX_CONSTANT_BUFFER vertex_constant_buffer;
	{
		float L = draw_data->DisplayPos.x;
		float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
		float T = draw_data->DisplayPos.y;
		float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
		float mvp[4][4] =
		{
			{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
			{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
			{ 0.0f,         0.0f,           0.5f,       0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
		};
		memcpy(&vertex_constant_buffer.mvp, mvp, sizeof(mvp));
	}


	XRHIRenderPassInfo RPInfos(1, &SceneColorTex, ERenderTargetLoadAction::ELoad, nullptr, EDepthStencilLoadAction::ENoAction);
	RHICmdList->RHIBeginRenderPass(RPInfos, L"IMGUI_PASS");
	RHICmdList->CacheActiveRenderTargets(RPInfos);
	RHICmdList->ApplyCachedRenderTargets(*BackendData->PSOInit);
	SetGraphicsPipelineStateFromPSOInit(*RHICmdList, *BackendData->PSOInit);

	TShaderReference<XUIBackendVS> VertexShader = GetGlobalShaderMapping()->GetShader<XUIBackendVS>();
	TShaderReference<XUIBackendPS> PixelShader = GetGlobalShaderMapping()->GetShader<XUIBackendPS>();
	
	VertexShader->SetParameter(*RHICmdList, vertex_constant_buffer);
	PixelShader->SetParameter(*RHICmdList, BackendData->FontsTexture.get());
}

void RHIUI::ImGui_Impl_RHI_RenderDrawData(ImDrawData* draw_data, XRHICommandList* RHICmdList, XRHITexture* SceneColorTex)
{
	ImGui_Impl_RHI_Data* BackendData = ImGui_Impl_RHI_GetBackendData();

	RHIUI_VBIB* VBIB = &BackendData->RenderBuffer->VBIB;
	
	// Create and grow vertex/index buffers if needed
	if (VBIB->VertexBuffer.get() == nullptr || VBIB->VertexBufferSize < draw_data->TotalVtxCount)
	{
		XRHIResourceCreateData NullData;
		VBIB->VertexBufferSize = draw_data->TotalVtxCount + 5000;
		VBIB->VertexBuffer = RHIcreateVertexBuffer(sizeof(ImDrawVert), sizeof(ImDrawVert) * VBIB->VertexBufferSize, EBufferUsage::BUF_Dynamic, NullData);
	}

	if (VBIB->IndexBuffer.get() == nullptr || VBIB->IndexBufferSize < draw_data->TotalIdxCount)
	{
		XRHIResourceCreateData NullData;
		VBIB->IndexBufferSize = draw_data->TotalIdxCount + 10000;
		VBIB->IndexBuffer = RHICreateIndexBuffer(sizeof(uint16), VBIB->IndexBufferSize, EBufferUsage::BUF_Dynamic, NullData);
	}

	
	void* VertexResourceData = LockVertexBuffer(VBIB->VertexBuffer.get(), 0, VBIB->VertexBufferSize);
	void* IndexResourceData = LockIndexBuffer(VBIB->IndexBuffer.get(), 0, VBIB->IndexBufferSize);

	X_Assert(draw_data->CmdListsCount == 1);
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		X_Assert(n != 1);
		const ImDrawList* DrawList = draw_data->CmdLists[0];
		memcpy(VertexResourceData, DrawList->VtxBuffer.Data, DrawList->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(IndexResourceData, DrawList->IdxBuffer.Data, DrawList->IdxBuffer.Size * sizeof(ImDrawIdx));
	}

	
	UnLockIndexBuffer(VBIB->IndexBuffer.get());
	UnLockVertexBuffer(VBIB->VertexBuffer.get());

	ImGui_Impl_RHI_SetupRenderState(draw_data, RHICmdList, SceneColorTex);

	X_Assert(draw_data->CmdListsCount == 1);

	const ImDrawList* cmd_list = draw_data->CmdLists[0];
	for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
	{
		const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
		if (pcmd->UserCallback != NULL)
		{
			// User callback, registered via ImDrawList::AddCallback()
			// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
			if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
				ImGui_Impl_RHI_SetupRenderState(draw_data, RHICmdList, SceneColorTex);
			else
				pcmd->UserCallback(cmd_list, pcmd);
		}
		else
		{
			RHICmdList->SetVertexBuffer(VBIB->VertexBuffer.get(), 0, 0);
			RHICmdList->RHIDrawIndexedPrimitive(VBIB->IndexBuffer.get(), pcmd->ElemCount, 1, pcmd->IdxOffset, pcmd->VtxOffset, 0);
		}
	}


}
