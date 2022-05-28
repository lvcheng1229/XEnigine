#include "DeferredShadingRenderer.h"
#include "Runtime/RenderCore/ShaderParameter.h"
#include "Runtime/RenderCore/GlobalShader.h"
#include "Runtime/RHI/PipelineStateCache.h"
#include "Runtime/RHI/RHIStaticStates.h"

#include "VoxelizationScene.h"
#include "MeshMaterialShader.h"
#include "Runtime/Engine/Classes/Material.h"
#include "Runtime/Engine/Material/MaterialShared.h"
#include "MeshPassProcessor.h"


class CheckSVOFlagCS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new CheckSVOFlagCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	CheckSVOFlagCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		cbSVOBuildBuffer.Bind(Initializer.ShaderParameterMap, "cbSVOBuildBuffer");
		VoxelArray.Bind(Initializer.ShaderParameterMap, "VoxelArray");
		SpaseVoxelOctreeRW.Bind(Initializer.ShaderParameterMap, "SpaseVoxelOctreeRW");
	}

	void SetParameters(
		XRHICommandList& RHICommandList,
		XRHIConstantBuffer* cbSVOBuildBufferIn,
		XRHIShaderResourceView* VoxelArrayIn,
		XRHIUnorderedAcessView* SpaseVoxelOctreeRWIn
	)
	{

		SetShaderConstantBufferParameter(
			RHICommandList, EShaderType::SV_Compute, cbSVOBuildBuffer, cbSVOBuildBufferIn);
		SetShaderSRVParameter(RHICommandList, EShaderType::SV_Compute, VoxelArray, VoxelArrayIn);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, SpaseVoxelOctreeRW, SpaseVoxelOctreeRWIn);
	}

	CBVParameterType cbSVOBuildBuffer;
	UAVParameterType VoxelArray;
	UAVParameterType SpaseVoxelOctreeRW;
};

CheckSVOFlagCS::ShaderInfos CheckSVOFlagCS::StaticShaderInfos(
	"CheckSVOFlagCS", L"E:/XEngine/XEnigine/Source/Shaders/SVOBuildCS.hlsl",
	"CheckSVOFlagCS", EShaderType::SV_Compute, CheckSVOFlagCS::CustomConstrucFunc,
	CheckSVOFlagCS::ModifyShaderCompileSettings);


class SubDivideNodeCS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new SubDivideNodeCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	SubDivideNodeCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		cbSVOBuildBuffer.Bind(Initializer.ShaderParameterMap, "cbSVOBuildBuffer");
		SpaseVoxelOctreeRW.Bind(Initializer.ShaderParameterMap, "SpaseVoxelOctreeRW");
		NodeCountAndOffsetBufferUAV.Bind(Initializer.ShaderParameterMap, "NodeCountAndOffsetBuffer");
		NodesCounterUAV.Bind(Initializer.ShaderParameterMap, "NodesCounter");
	}

	void SetParameters(
		XRHICommandList& RHICommandList,
		XRHIConstantBuffer* cbSVOBuildBufferIn,
		XRHIUnorderedAcessView* SpaseVoxelOctreeRWIn,
		XRHIUnorderedAcessView* NodeCountAndOffsetBufferUAVIn,
		XRHIUnorderedAcessView* NodesCounterUAVIn
	)
	{
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbSVOBuildBuffer, cbSVOBuildBufferIn);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, SpaseVoxelOctreeRW, SpaseVoxelOctreeRWIn);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, NodeCountAndOffsetBufferUAV, NodeCountAndOffsetBufferUAVIn);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, NodesCounterUAV, NodesCounterUAVIn);
	}
	CBVParameterType cbSVOBuildBuffer;
	UAVParameterType SpaseVoxelOctreeRW;
	UAVParameterType NodeCountAndOffsetBufferUAV;
	UAVParameterType NodesCounterUAV;
};
SubDivideNodeCS::ShaderInfos SubDivideNodeCS::StaticShaderInfos(
	"SubDivideNodeCS", L"E:/XEngine/XEnigine/Source/Shaders/SVOBuildCS.hlsl",
	"SubDivideNodeCS", EShaderType::SV_Compute, SubDivideNodeCS::CustomConstrucFunc,
	SubDivideNodeCS::ModifyShaderCompileSettings);


class ConnectNeighborsCS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new ConnectNeighborsCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	ConnectNeighborsCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		cbSVOBuildBuffer.Bind(Initializer.ShaderParameterMap, "cbSVOBuildBuffer");
		SpaseVoxelOctreeRW.Bind(Initializer.ShaderParameterMap, "SpaseVoxelOctreeRW");
		NodeCountAndOffsetBufferUAV.Bind(Initializer.ShaderParameterMap, "NodeCountAndOffsetBuffer");
	}
	void SetParameters(
		XRHICommandList& RHICommandList,
		XRHIConstantBuffer* cbSVOBuildBufferIn,
		XRHIUnorderedAcessView* SpaseVoxelOctreeRWIn,
		XRHIUnorderedAcessView* NodeCountAndOffsetBufferUAVIn
	)
	{
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbSVOBuildBuffer, cbSVOBuildBufferIn);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, SpaseVoxelOctreeRW, SpaseVoxelOctreeRWIn);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, NodeCountAndOffsetBufferUAV, NodeCountAndOffsetBufferUAVIn);
	}
	CBVParameterType cbSVOBuildBuffer;
	UAVParameterType SpaseVoxelOctreeRW;
	UAVParameterType NodeCountAndOffsetBufferUAV;
};
ConnectNeighborsCS::ShaderInfos ConnectNeighborsCS::StaticShaderInfos(
	"ConnectNeighborsCS", L"E:/XEngine/XEnigine/Source/Shaders/SVOBuildCS.hlsl",
	"ConnectNeighborsCS", EShaderType::SV_Compute, ConnectNeighborsCS::CustomConstrucFunc,
	ConnectNeighborsCS::ModifyShaderCompileSettings);


class ConnectNodesToVoxelsCS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new ConnectNodesToVoxelsCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	ConnectNodesToVoxelsCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		VoxelArray.Bind(Initializer.ShaderParameterMap, "VoxelArray");
		SpaseVoxelOctreeRW.Bind(Initializer.ShaderParameterMap, "SpaseVoxelOctreeRW");
	}
	void SetParameters(
		XRHICommandList& RHICommandList,
		XRHIShaderResourceView* VoxelArrayIn,
		XRHIUnorderedAcessView* SpaseVoxelOctreeRWIn
	)
	{
		SetShaderSRVParameter(RHICommandList, EShaderType::SV_Compute, VoxelArray, VoxelArrayIn);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, SpaseVoxelOctreeRW, SpaseVoxelOctreeRWIn);
	}
	UAVParameterType VoxelArray;
	UAVParameterType SpaseVoxelOctreeRW;
};
ConnectNodesToVoxelsCS::ShaderInfos ConnectNodesToVoxelsCS::StaticShaderInfos(
	"ConnectNodesToVoxelsCS", L"E:/XEngine/XEnigine/Source/Shaders/SVOBuildCS.hlsl",
	"ConnectNodesToVoxelsCS", EShaderType::SV_Compute, ConnectNodesToVoxelsCS::CustomConstrucFunc,
	ConnectNodesToVoxelsCS::ModifyShaderCompileSettings);


void XDeferredShadingRenderer::SpaseVoxelOctreeBuild(XRHICommandList& RHICmdList)
{
	if (VoxelSceneInitialize == true) [[unlikely]]
	{
		return;
	}
	VoxelSceneInitialize = true;

	cbSVOBuildBufferStruct SVOBuildBufferIns[4];
	SVOBuildBufferIns[0].CurrentLevel = 0;
	SVOBuildBufferIns[1].CurrentLevel = 1;
	SVOBuildBufferIns[2].CurrentLevel = 2;
	SVOBuildBufferIns[3].CurrentLevel = 3;

	SVOGIResourece.cbSVOBuildBufferLevel0->UpdateData(&SVOBuildBufferIns[0], sizeof(cbSVOBuildBufferStruct),0);
	SVOGIResourece.cbSVOBuildBufferLevel1->UpdateData(&SVOBuildBufferIns[1], sizeof(cbSVOBuildBufferStruct),0);
	SVOGIResourece.cbSVOBuildBufferLevel2->UpdateData(&SVOBuildBufferIns[2], sizeof(cbSVOBuildBufferStruct),0);
	SVOGIResourece.cbSVOBuildBufferLevel3->UpdateData(&SVOBuildBufferIns[3], sizeof(cbSVOBuildBufferStruct),0);

	XRHIConstantBuffer* ArrayBuffer[4];
	ArrayBuffer[0] = SVOGIResourece.cbSVOBuildBufferLevel0.get();
	ArrayBuffer[1] = SVOGIResourece.cbSVOBuildBufferLevel1.get();
	ArrayBuffer[2] = SVOGIResourece.cbSVOBuildBufferLevel2.get();
	ArrayBuffer[3] = SVOGIResourece.cbSVOBuildBufferLevel3.get();

	for (uint32 i = 0; i < OctreeHeight - 1; i++)
	{
		{
			RHICmdList.RHIEventBegin(1, "CheckSVOFlagCS", sizeof("CheckSVOFlagCS"));
			TShaderReference<CheckSVOFlagCS> Shader = GetGlobalShaderMapping()->GetShader<CheckSVOFlagCS>();
			XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
			SetComputePipelineStateFromCS(RHICmdList, ComputeShader);
			Shader->SetParameters(
				RHICmdList,
				ArrayBuffer[i],
				SVOGIResourece.VoxelArrayShaderResourceView.get(),
				GetRHIUAVFromTexture(SVOGIResourece.SpaseVoxelOctree.get()));
			RHICmdList.RHIDispatchComputeShader(static_cast<UINT>(VoxelDimension * VoxelDimension / float(128)), 1, 1);
			RHICmdList.RHIEventEnd();
		}

		{
			RHICmdList.RHIEventBegin(1, "SubDivideNodeCS", sizeof("SubDivideNodeCS"));
			TShaderReference<SubDivideNodeCS> Shader = GetGlobalShaderMapping()->GetShader<SubDivideNodeCS>();
			XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
			SetComputePipelineStateFromCS(RHICmdList, ComputeShader);
			Shader->SetParameters(
				RHICmdList,
				ArrayBuffer[i],
				GetRHIUAVFromTexture(SVOGIResourece.SpaseVoxelOctree.get()),
				GetRHIUAVFromTexture(SVOGIResourece.NodeCountAndOffsetBuffer.get()),
				SVOGIResourece.SVOCounterBufferUnorderedAcessView.get()
			);
			RHICmdList.RHIDispatchComputeShader(static_cast<UINT>(ApproxVoxelDimension / float(128)), 1, 1);
			RHICmdList.RHIEventEnd();
		}

		{
			RHICmdList.RHIEventBegin(1, "ConnectNeighborsCS", sizeof("ConnectNeighborsCS"));
			TShaderReference<ConnectNeighborsCS> Shader = GetGlobalShaderMapping()->GetShader<ConnectNeighborsCS>();
			XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
			SetComputePipelineStateFromCS(RHICmdList, ComputeShader);
			Shader->SetParameters(
				RHICmdList,
				ArrayBuffer[i],
				GetRHIUAVFromTexture(SVOGIResourece.SpaseVoxelOctree.get()),
				GetRHIUAVFromTexture(SVOGIResourece.NodeCountAndOffsetBuffer.get())
			);
			RHICmdList.RHIDispatchComputeShader(static_cast<UINT>(ApproxVoxelDimension / float(128)), 1, 1);
			RHICmdList.RHIEventEnd();
		}
	}

	{
		RHICmdList.RHIEventBegin(1, "ConnectNodesToVoxelsCS", sizeof("ConnectNodesToVoxelsCS"));
		TShaderReference<ConnectNodesToVoxelsCS> Shader = GetGlobalShaderMapping()->GetShader<ConnectNodesToVoxelsCS>();
		XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
		SetComputePipelineStateFromCS(RHICmdList, ComputeShader);
		Shader->SetParameters(
			RHICmdList,
			SVOGIResourece.VoxelArrayShaderResourceView.get(),
			GetRHIUAVFromTexture(SVOGIResourece.SpaseVoxelOctree.get()));
		RHICmdList.RHIDispatchComputeShader(static_cast<UINT>(ApproxVoxelDimension / float(128)), 1, 1);
		RHICmdList.RHIEventEnd();
	}

}