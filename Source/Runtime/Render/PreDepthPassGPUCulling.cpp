#include "DeferredShadingRenderer.h"
#include "Runtime/RenderCore/ShaderParameter.h"
#include "Runtime/RenderCore/GlobalShader.h"
#include "Runtime/RHI/PipelineStateCache.h"
#include "Runtime/RHI/RHIStaticStates.h"

class DepthGPUCullingCS : public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new DepthGPUCullingCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	DepthGPUCullingCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		outputCommands.Bind(Initializer.ShaderParameterMap, "outputCommands");
		inputCommands.Bind(Initializer.ShaderParameterMap, "inputCommands");
		SceneConstantBufferIN.Bind(Initializer.ShaderParameterMap, "SceneConstantBufferIN");
		cbCullingParameters.Bind(Initializer.ShaderParameterMap, "cbCullingParameters");
	}

	void SetParameters(XRHICommandList& RHICommandList,
		XRHIUnorderedAcessView* OutputCommandsUAV,
		XRHIShaderResourceView* InputCommandsSRV,
		XRHIShaderResourceView* SceneConstantBufferINView,
		XRHIConstantBuffer* CullingParametersCBV)
	{
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, outputCommands, OutputCommandsUAV);
		SetShaderSRVParameter(RHICommandList, EShaderType::SV_Compute, inputCommands, InputCommandsSRV);
		SetShaderSRVParameter(RHICommandList, EShaderType::SV_Compute, SceneConstantBufferIN, SceneConstantBufferINView);
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbCullingParameters, CullingParametersCBV);
	}

	UAVParameterType outputCommands;
	SRVParameterType inputCommands;
	SRVParameterType SceneConstantBufferIN;
	CBVParameterType cbCullingParameters;
};
DepthGPUCullingCS::ShaderInfos DepthGPUCullingCS::StaticShaderInfos(
	"DepthGPUCullingCS", L"E:/XEngine/XEnigine/Source/Shaders/GPUCulling.hlsl",
	"CSMain", EShaderType::SV_Compute, DepthGPUCullingCS::CustomConstrucFunc,
	DepthGPUCullingCS::ModifyShaderCompileSettings);


class XPreDepthPassVS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XPreDepthPassVS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	XPreDepthPassVS(const XShaderInitlizer& Initializer)
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


class XPreDepthPassPS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new XPreDepthPassPS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	XPreDepthPassPS(const XShaderInitlizer& Initializer)
		:XGloablShader(Initializer) {}

	void SetParameter(XRHICommandList& RHICommandList) {}
};

XPreDepthPassVS::ShaderInfos XPreDepthPassVS::StaticShaderInfos(
	"XPreDepthPassVS", L"E:/XEngine/XEnigine/Source/Shaders/DepthOnlyVertexShader.hlsl",
	"VS", EShaderType::SV_Vertex, XPreDepthPassVS::CustomConstrucFunc,
	XPreDepthPassVS::ModifyShaderCompileSettings);
XPreDepthPassPS::ShaderInfos XPreDepthPassPS::StaticShaderInfos(
	"XPreDepthPassPS", L"E:/XEngine/XEnigine/Source/Shaders/DepthOnlyVertexShader.hlsl",
	"PS", EShaderType::SV_Pixel, XPreDepthPassPS::CustomConstrucFunc,
	XPreDepthPassPS::ModifyShaderCompileSettings);

void XDeferredShadingRenderer::PreDepthPassGPUCullingSetup()
{
	std::vector<XRHIIndirectArg>IndirectPreDepthArgs;
	IndirectPreDepthArgs.resize(4);
	IndirectPreDepthArgs[0].type = IndirectArgType::Arg_CBV;
	IndirectPreDepthArgs[0].CBV.RootParameterIndex = 0;
	IndirectPreDepthArgs[1].type = IndirectArgType::Arg_VBV;
	IndirectPreDepthArgs[2].type = IndirectArgType::Arg_IBV;
	IndirectPreDepthArgs[3].type = IndirectArgType::Arg_Draw_Indexed;

	TShaderReference<XPreDepthPassVS> VertexShader = GetGlobalShaderMapping()->GetShader<XPreDepthPassVS>();
	TShaderReference<XPreDepthPassPS> PixelShader = GetGlobalShaderMapping()->GetShader<XPreDepthPassPS>();
	RHIDepthCommandSignature = RHICreateCommandSignature(IndirectPreDepthArgs.data(), 4, VertexShader.GetVertexShader(), PixelShader.GetPixelShader());

	void* DataPtrret;
	{
		std::vector<XRHICommandData> RHICmdData;
		RHICmdData.resize(RenderGeos.size());
		for (int i = 0; i < RenderGeos.size(); i++)
		{
			auto& it = RenderGeos[i];
			RHICmdData[i].CBVs.push_back(it->GetPerObjectVertexCBuffer().get());
			auto VertexBufferPtr = it->GetRHIVertexBuffer();
			auto IndexBufferPtr = it->GetRHIIndexBuffer();
			RHICmdData[i].VB = VertexBufferPtr.get();
			RHICmdData[i].IB = IndexBufferPtr.get();
			RHICmdData[i].IndexCountPerInstance = it->GetIndexCount();
			RHICmdData[i].InstanceCount = 1;
			RHICmdData[i].StartIndexLocation = 0;
			RHICmdData[i].BaseVertexLocation = 0;
			RHICmdData[i].StartInstanceLocation = 0;
		}

		uint32 OutCmdDataSize;
		DataPtrret = RHIGetCommandDataPtr(RHICmdData, OutCmdDataSize);

		uint32 DepthPassIndirectBufferDataSize = OutCmdDataSize * RHICmdData.size();
		FResourceVectorUint8 DepthIndirectBufferData;
		DepthIndirectBufferData.Data = DataPtrret;
		DepthIndirectBufferData.SetResourceDataSize(DepthPassIndirectBufferDataSize);
		XRHIResourceCreateData IndirectBufferResourceData(&DepthIndirectBufferData);
		DepthCmdBufferNoCulling = RHIcreateStructBuffer(OutCmdDataSize, DepthPassIndirectBufferDataSize,
			EBufferUsage(int(EBufferUsage::BUF_DrawIndirect) | (int)EBufferUsage::BUF_ShaderResource), IndirectBufferResourceData);

		DepthCmdBufferOffset = RHIGetCmdBufferOffset(DepthCmdBufferNoCulling.get());
		DepthCounterOffset = AlignArbitrary(DepthPassIndirectBufferDataSize, D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT);

		DepthCmdBufferCulled = RHIcreateStructBuffer(DepthPassIndirectBufferDataSize, DepthCounterOffset + sizeof(UINT),
			EBufferUsage(int(EBufferUsage::BUF_StructuredBuffer) | int(EBufferUsage::BUF_UnorderedAccess)), nullptr);

		CmdBufferShaderResourceView = RHICreateShaderResourceView(DepthCmdBufferNoCulling.get());
		CmdBufferUnorderedAcessView = RHICreateUnorderedAccessView(DepthCmdBufferCulled.get(), true, true, DepthCounterOffset);

		CullingParametersIns.commandCount = RenderGeos.size();
	}


	//ObjectConstants
	GlobalShadowViewProjMatrix = RHICreateConstantBuffer(TileNumWidthPerVirtualTex * TileNumWidthPerVirtualTex * sizeof(TiledInfoStruct));
}


void XDeferredShadingRenderer::PreDepthPassGPUCulling(XRHICommandList& RHICmdList)
{
	RHICmdList.RHIEventBegin(1, "GPUCulling", sizeof("GPUCulling"));
	TShaderReference<DepthGPUCullingCS> Shader = GetGlobalShaderMapping()->GetShader<DepthGPUCullingCS>();
	XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
	SetComputePipelineStateFromCS(RHICmdList, ComputeShader);
	RHIResetStructBufferCounter(DepthCmdBufferCulled.get(), DepthCounterOffset);
	Shader->SetParameters(RHICmdList, CmdBufferUnorderedAcessView.get(),
		CmdBufferShaderResourceView.get(), GlobalObjectStructBufferSRV.get(), cbCullingParameters.get());

	RHICmdList.RHIDispatchComputeShader(static_cast<UINT>(ceil(RenderGeos.size() / float(128))), 1, 1);
	RHICmdList.RHIEventEnd();
}

void XDeferredShadingRenderer::PreDepthPassRendering(XRHICommandList& RHICmdList)
{
	XRHIRenderPassInfo RPInfos(0, nullptr, ERenderTargetLoadAction::ENoAction, TextureDepthStencil.get(), EDepthStencilLoadAction::EClear);
	RHICmdList.RHIBeginRenderPass(RPInfos, "PreDepthPass", sizeof("PreDepthPass"));
	RHICmdList.CacheActiveRenderTargets(RPInfos);

	XGraphicsPSOInitializer GraphicsPSOInit;
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();;
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<true, ECompareFunction::CF_GreaterEqual>::GetRHI();

	TShaderReference<XPreDepthPassVS> VertexShader = GetGlobalShaderMapping()->GetShader<XPreDepthPassVS>();
	TShaderReference<XPreDepthPassPS> PixelShader = GetGlobalShaderMapping()->GetShader<XPreDepthPassPS>();
	GraphicsPSOInit.BoundShaderState.RHIVertexShader = VertexShader.GetVertexShader();
	GraphicsPSOInit.BoundShaderState.RHIPixelShader = PixelShader.GetPixelShader();
	GraphicsPSOInit.BoundShaderState.RHIVertexLayout = LocalVertexFactory.GetLayout(ELayoutType::Layout_Default).get();

	RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	SetGraphicsPipelineStateFromPSOInit(RHICmdList, GraphicsPSOInit);

	VertexShader->SetParameter(RHICmdList, RViewInfo.ViewConstantBuffer.get());

	RHICmdList.RHIExecuteIndirect(RHIDepthCommandSignature.get(), RenderGeos.size(), DepthCmdBufferCulled.get(),
		DepthCmdBufferOffset, DepthCmdBufferCulled.get(), DepthCounterOffset);
	RHICmdList.RHIEventEnd();
}
