#include "DeferredShadingRenderer.h"
#include "Runtime/RenderCore/ShaderParameter.h"
#include "Runtime/RenderCore/GlobalShader.h"
#include "Runtime/RHI/PipelineStateCache.h"
#include "Runtime/RHI/RHIStaticStates.h"

#include "VoxelizationScene.h"
#include "ShadowMap_Old.h"

class ShadowMapInjectLightCS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new ShadowMapInjectLightCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	ShadowMapInjectLightCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		cbShadow.Bind(Initializer.ShaderParameterMap, "cbShadow");
		cbVoxelization.Bind(Initializer.ShaderParameterMap, "cbVoxelization");
		cbMainLight.Bind(Initializer.ShaderParameterMap, "cbMainLight");

		VoxelArrayR.Bind(Initializer.ShaderParameterMap, "VoxelArrayR");
		SceneDepthInput.Bind(Initializer.ShaderParameterMap, "SceneDepthInput");

		IrradianceBrickBufferRW.Bind(Initializer.ShaderParameterMap, "IrradianceBrickBufferRW");
		SpaseVoxelOctreeRW.Bind(Initializer.ShaderParameterMap, "SpaseVoxelOctreeRW");
	}

	void SetParameters(
		XRHICommandList& RHICommandList,
		XRHIConstantBuffer* cbShadowIn,
		XRHIConstantBuffer* cbVoxelizationIn,
		XRHIConstantBuffer* cbMainLightIn,

		XRHIShaderResourceView* VoxelArrayRIn,
		XRHIShaderResourceView* SceneDepthInputIn,

		XRHIUnorderedAcessView* IrradianceBrickBufferRWIn,
		XRHIUnorderedAcessView* SpaseVoxelOctreeRWIn
	)
	{
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbShadow, cbShadowIn);
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbVoxelization, cbVoxelizationIn);
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbMainLight, cbMainLightIn);

		SetShaderSRVParameter(RHICommandList, EShaderType::SV_Compute, VoxelArrayR, VoxelArrayRIn);
		SetShaderSRVParameter(RHICommandList, EShaderType::SV_Compute, SceneDepthInput, SceneDepthInputIn);

		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, IrradianceBrickBufferRW, IrradianceBrickBufferRWIn);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, SpaseVoxelOctreeRW, SpaseVoxelOctreeRWIn);
	}

	CBVParameterType cbShadow;
	CBVParameterType cbVoxelization;
	CBVParameterType cbMainLight;

	SRVParameterType VoxelArrayR;
	SRVParameterType SceneDepthInput;

	UAVParameterType IrradianceBrickBufferRW;
	UAVParameterType SpaseVoxelOctreeRW;
};
ShadowMapInjectLightCS::ShaderInfos ShadowMapInjectLightCS::StaticShaderInfos(
	"ShadowMapInjectLightCS", L"E:/XEngine/XEnigine/Source/Shaders/SVOInjectLight.hlsl",
	"ShadowMapInjectLightCS", EShaderType::SV_Compute, ShadowMapInjectLightCS::CustomConstrucFunc,
	ShadowMapInjectLightCS::ModifyShaderCompileSettings);

class ResetOctreeFlagsCS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new ResetOctreeFlagsCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	ResetOctreeFlagsCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		SpaseVoxelOctreeRW.Bind(Initializer.ShaderParameterMap, "SpaseVoxelOctreeRW");
		NodeCountAndOffsetBuffer.Bind(Initializer.ShaderParameterMap, "NodeCountAndOffsetBuffer");
	}

	void SetParameters(
		XRHICommandList& RHICommandList,
		XRHIUnorderedAcessView* SpaseVoxelOctreeRWIn,
		XRHIUnorderedAcessView* NodeCountAndOffsetBufferUAVIn)
	{
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, SpaseVoxelOctreeRW, SpaseVoxelOctreeRWIn);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, NodeCountAndOffsetBuffer, NodeCountAndOffsetBufferUAVIn);
	}
	UAVParameterType NodeCountAndOffsetBuffer;
	UAVParameterType SpaseVoxelOctreeRW;
};




class AverageLitNodeValuesCS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new AverageLitNodeValuesCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	AverageLitNodeValuesCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		IrradianceBrickBufferROnly.Bind(Initializer.ShaderParameterMap, "IrradianceBrickBufferROnly");
		IrradianceBrickBufferWOnly.Bind(Initializer.ShaderParameterMap, "IrradianceBrickBufferWOnly");
		//NodeCountAndOffsetBuffer.Bind(Initializer.ShaderParameterMap, "NodeCountAndOffsetBuffer");
		NodeCountAndOffsetBufferR.Bind(Initializer.ShaderParameterMap, "NodeCountAndOffsetBufferR");
	}

	void SetParameters(
		XRHICommandList& RHICommandList,
		XRHIShaderResourceView* IrradianceBrickBufferROnlyIn,
		XRHIShaderResourceView* NodeCountAndOffsetBufferRIn,
		XRHIUnorderedAcessView* IrradianceBrickBufferWOnlyIn
		//,XRHIUnorderedAcessView* NodeCountAndOffsetBufferUAVIn
	)
	{
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, IrradianceBrickBufferWOnly, IrradianceBrickBufferWOnlyIn);
		SetShaderSRVParameter(RHICommandList, EShaderType::SV_Compute, NodeCountAndOffsetBufferR, NodeCountAndOffsetBufferRIn);
		SetShaderSRVParameter(RHICommandList, EShaderType::SV_Compute, IrradianceBrickBufferROnly, IrradianceBrickBufferROnlyIn);
		//SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, NodeCountAndOffsetBuffer, NodeCountAndOffsetBufferUAVIn);
	}
	//UAVParameterType NodeCountAndOffsetBuffer;
	SRVParameterType NodeCountAndOffsetBufferR;
	UAVParameterType IrradianceBrickBufferROnly;
	UAVParameterType IrradianceBrickBufferWOnly;
};
AverageLitNodeValuesCS::ShaderInfos AverageLitNodeValuesCS::StaticShaderInfos(
	"AverageLitNodeValuesCS", L"E:/XEngine/XEnigine/Source/Shaders/SVOInjectLight.hlsl",
	"AverageLitNodeValuesCS", EShaderType::SV_Compute, AverageLitNodeValuesCS::CustomConstrucFunc,
	AverageLitNodeValuesCS::ModifyShaderCompileSettings);

class GatherValuesFromLowLevelCS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new GatherValuesFromLowLevelCS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;
	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	GatherValuesFromLowLevelCS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer)
	{
		SpaseVoxelOctreeR.Bind(Initializer.ShaderParameterMap, "SpaseVoxelOctreeR");
		IrradianceBrickBufferROnly.Bind(Initializer.ShaderParameterMap, "IrradianceBrickBufferROnly");
		NodeCountAndOffsetBufferR.Bind(Initializer.ShaderParameterMap, "NodeCountAndOffsetBufferR");
		IrradianceBrickBufferWOnly.Bind(Initializer.ShaderParameterMap, "IrradianceBrickBufferWOnly");
		cbSVOBuildBuffer.Bind(Initializer.ShaderParameterMap, "cbSVOBuildBuffer");
	}

	void SetParameters(
		XRHICommandList& RHICommandList,
		XRHIShaderResourceView* SpaseVoxelOctreeRIn,
		XRHIShaderResourceView* IrradianceBrickBufferROnlyIn,
		XRHIShaderResourceView* NodeCountAndOffsetBufferRIn,
		XRHIUnorderedAcessView* IrradianceBrickBufferWOnlyIn,
		XRHIConstantBuffer* cbSVOBuildBufferIn
	)
	{
		SetShaderSRVParameter(RHICommandList, EShaderType::SV_Compute, SpaseVoxelOctreeR, SpaseVoxelOctreeRIn);
		SetShaderSRVParameter(RHICommandList, EShaderType::SV_Compute, IrradianceBrickBufferROnly, IrradianceBrickBufferROnlyIn);
		SetShaderSRVParameter(RHICommandList, EShaderType::SV_Compute, NodeCountAndOffsetBufferR, NodeCountAndOffsetBufferRIn);
		SetShaderUAVParameter(RHICommandList, EShaderType::SV_Compute, IrradianceBrickBufferWOnly, IrradianceBrickBufferWOnlyIn);
		SetShaderConstantBufferParameter(RHICommandList, EShaderType::SV_Compute, cbSVOBuildBuffer, cbSVOBuildBufferIn);
	}
	SRVParameterType SpaseVoxelOctreeR;
	SRVParameterType IrradianceBrickBufferROnly;
	SRVParameterType NodeCountAndOffsetBufferR;
	UAVParameterType IrradianceBrickBufferWOnly;
	CBVParameterType cbSVOBuildBuffer;
};
GatherValuesFromLowLevelCS::ShaderInfos GatherValuesFromLowLevelCS::StaticShaderInfos(
	"GatherValuesFromLowLevelCS", L"E:/XEngine/XEnigine/Source/Shaders/SVOInjectLight.hlsl",
	"GatherValuesFromLowLevelCS", EShaderType::SV_Compute, GatherValuesFromLowLevelCS::CustomConstrucFunc,
	GatherValuesFromLowLevelCS::ModifyShaderCompileSettings);

void XDeferredShadingRenderer::SVOInjectLightPass(XRHICommandList& RHICmdList)
{
	cbMainLightStruct cbMainLightins;
	cbMainLightins.LightColor = MainLightColor;
	cbMainLightins.LightIntensity = LightIntensity;
	cbMainLightins.LightDir = ShadowLightDir;
	SVOGIResourece.RHIcbMainLight->UpdateData(&cbMainLightins, sizeof(cbMainLightStruct), 0);

	{
		RHICmdList.RHIEventBegin(1, "ShadowMapInjectLightCS", sizeof("ShadowMapInjectLightCS"));
		TShaderReference<ShadowMapInjectLightCS> Shader = GetGlobalShaderMapping()->GetShader<ShadowMapInjectLightCS>();
		XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
		SetComputePipelineStateFromCS(RHICmdList, ComputeShader);
		Shader->SetParameters(
			RHICmdList,
			ShadowMapResourece_Old.ShadowViewProjectionCB.get(),
			SVOGIResourece.VoxelScenePSCBuffer.get(),
			SVOGIResourece.RHIcbMainLight.get(),
			SVOGIResourece.VoxelArrayShaderResourceView.get(),
			GetRHISRVFromTexture(SceneTargets.TempShadowDepthTexture.get()),
			GetRHIUAVFromTexture(SVOGIResourece.IrradianceBrickBufferRWUAV.get()),
			GetRHIUAVFromTexture(SVOGIResourece.SpaseVoxelOctree.get())
		);
		RHICmdList.RHIDispatchComputeShader(ShadowMapDepthTextureSize / 16, ShadowMapDepthTextureSize /16, 1);
		RHICmdList.RHIEventEnd();
	}

	XRHITexture3D* BrickBufferPingPong[2];
	BrickBufferPingPong[0] = SVOGIResourece.IrradianceBrickBufferRWUAV.get();
	BrickBufferPingPong[1] = SVOGIResourece.IrradianceBrickBufferPinPongUAV.get();

	//()%2 for R
	//( + 1)%2 for W
	int PingPongIndex = 0;
	{
		RHICmdList.RHIEventBegin(1, "AverageLitNodeValuesCS", sizeof("AverageLitNodeValuesCS"));
		TShaderReference<AverageLitNodeValuesCS> Shader = GetGlobalShaderMapping()->GetShader<AverageLitNodeValuesCS>();
		XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
		SetComputePipelineStateFromCS(RHICmdList, ComputeShader);
		Shader->SetParameters(
			RHICmdList,
			//GetRHISRVFromTexture(SVOGIResourece.IrradianceBrickBufferRWUAV.get()),
			GetRHISRVFromTexture(BrickBufferPingPong[PingPongIndex % 2]),
			GetRHISRVFromTexture(SVOGIResourece.NodeCountAndOffsetBuffer.get()),
			//GetRHIUAVFromTexture(SVOGIResourece.IrradianceBrickBufferPinPongUAV.get())
			GetRHIUAVFromTexture(BrickBufferPingPong[(PingPongIndex + 1) % 2])
			//,GetRHIUAVFromTexture(SVOGIResourece.NodeCountAndOffsetBuffer.get())
		);
		RHICmdList.RHIDispatchComputeShader(VoxelDimension * 256 / 128, 1, 1);
		RHICmdList.RHIEventEnd();
		PingPongIndex++;
	}

	uint32 DispatchSize = VoxelDimension * 256 / 128;
	{
		DispatchSize /= 2;
	
		RHICmdList.RHIEventBegin(1, "GatherValuesFromLowLevelCS", sizeof("GatherValuesFromLowLevelCS"));
		TShaderReference<GatherValuesFromLowLevelCS> Shader = GetGlobalShaderMapping()->GetShader<GatherValuesFromLowLevelCS>();
		XRHIComputeShader* ComputeShader = Shader.GetComputeShader();
		SetComputePipelineStateFromCS(RHICmdList, ComputeShader);
		Shader->SetParameters(
			RHICmdList,
			GetRHISRVFromTexture(SVOGIResourece.SpaseVoxelOctree.get()),
			GetRHISRVFromTexture(BrickBufferPingPong[PingPongIndex % 2]),
			GetRHISRVFromTexture(SVOGIResourece.NodeCountAndOffsetBuffer.get()),
			GetRHIUAVFromTexture(BrickBufferPingPong[(PingPongIndex + 1) % 2]),
			SVOGIResourece.cbSVOBuildBufferLevels[9 - 2].get()
		);
		RHICmdList.RHIDispatchComputeShader(DispatchSize, 1, 1);
		RHICmdList.RHIEventEnd();
	}
}