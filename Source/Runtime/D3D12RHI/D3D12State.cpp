#include "D3D12PlatformRHI.h"
#include "D3D12State.h"
#include "d3dx12.h"
#include "D3D12Rootsignature.h"
#include "D3D12PipelineState.h"
#include "D3D12Shader.h"
#include <string>
static D3D12_COMPARISON_FUNC TranslateCompareFunction(ECompareFunction CompareFunction)
{
	switch (CompareFunction)
	{
	case ECompareFunction::CF_Greater: return D3D12_COMPARISON_FUNC_GREATER;
	case ECompareFunction::CF_Always:return D3D12_COMPARISON_FUNC_ALWAYS;
	default: X_Assert(false); return D3D12_COMPARISON_FUNC_ALWAYS;
	};
}

static D3D12_BLEND_OP TranslateBlendOp(EBlendOperation BlendOp)
{
	switch (BlendOp)
	{
	case EBlendOperation::BO_Add: return D3D12_BLEND_OP_ADD;
	default: X_Assert(false); return D3D12_BLEND_OP_ADD;
	};
}

static D3D12_BLEND TranslateBlendFactor(EBlendFactor BlendFactor)
{
	switch (BlendFactor)
	{
	case EBlendFactor::BF_One: return D3D12_BLEND_ONE;
	case EBlendFactor::BF_Zero: return D3D12_BLEND_ZERO;
	default: X_Assert(false); return D3D12_BLEND_ZERO;
	};
}

static void GetPSODescAndHash(
	const XGraphicsPSOInitializer& PSOInit, 
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& PSODesc,
	std::size_t& OutHash)
{
	ZeroMemory(&PSODesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	const XRHIBoundShaderStateInput& BoundShaderState = PSOInit.BoundShaderState;

	//Layout
	XD3D12VertexLayout* D3D12Layout = static_cast<XD3D12VertexLayout*>(BoundShaderState.RHIVertexLayout);
	PSODesc.InputLayout = { D3D12Layout->VertexElements.data(),(UINT)D3D12Layout->VertexElements.size() };

	//Shsader
	PSODesc.VS = static_cast<XD3D12VertexShader*>(BoundShaderState.RHIVertexShader)->D3DByteCode;
	PSODesc.PS = static_cast<XD3D12PixelShader*>(BoundShaderState.RHIPixelShader)->D3DByteCode;

	//State
	PSODesc.BlendState = static_cast<XD3D12BlendState*>(PSOInit.BlendState)->Desc;
	PSODesc.DepthStencilState = static_cast<XD3D12DepthStencilState*>(PSOInit.DepthStencilState)->Desc;

	//RTV DSV
	PSODesc.NumRenderTargets = PSOInit.RTNums;
	for (int i = 0; i < PSOInit.RTNums; i++)
	{
		PSODesc.RTVFormats[i] = (DXGI_FORMAT)GPixelFormats[(int)PSOInit.RT_Format[i]].PlatformFormat;
	}
	PSODesc.DSVFormat = (DXGI_FORMAT)GPixelFormats[(int)PSOInit.DS_Format].PlatformFormat;

	//default
	PSODesc.SampleMask = UINT_MAX;
	PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	PSODesc.SampleDesc.Count = 1;
	PSODesc.SampleDesc.Quality = 0;

	//GetHash
	OutHash = std::hash<std::string>{}(std::string(
		(char*)(&PSODesc.StreamOutput),
		(sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC) - sizeof(D3D12_SHADER_BYTECODE) * 5 - sizeof(ID3D12RootSignature*)))
		);
	THashCombine(OutHash, PSOInit.BoundShaderState.RHIVertexShader->GetHash());
	THashCombine(OutHash, PSOInit.BoundShaderState.RHIPixelShader->GetHash());
}

static void GetBoundCountAndHash(
	const XGraphicsPSOInitializer& PSOInit,
	XPipelineRegisterBoundCount& RegisterBoundCount,
	std::size_t& OutRootSigHash)
{
	const XRHIBoundShaderStateInput& BoundShaderState = PSOInit.BoundShaderState;

	//RootSiganture
	memset(&RegisterBoundCount, 0, sizeof(XPipelineRegisterBoundCount));

	RegisterBoundCount.register_count[(int)EShaderType::SV_Vertex].UnorderedAccessCount
		= static_cast<XD3D12VertexShader*>(BoundShaderState.RHIVertexShader)->ResourceCount.NumUAV;
	RegisterBoundCount.register_count[(int)EShaderType::SV_Vertex].ShaderResourceCount
		= static_cast<XD3D12VertexShader*>(BoundShaderState.RHIVertexShader)->ResourceCount.NumSRV;
	RegisterBoundCount.register_count[(int)EShaderType::SV_Vertex].ConstantBufferCount
		= static_cast<XD3D12VertexShader*>(BoundShaderState.RHIVertexShader)->ResourceCount.NumCBV;

	RegisterBoundCount.register_count[(int)EShaderType::SV_Pixel].UnorderedAccessCount
		= static_cast<XD3D12PixelShader*>(BoundShaderState.RHIPixelShader)->ResourceCount.NumUAV;
	RegisterBoundCount.register_count[(int)EShaderType::SV_Pixel].ShaderResourceCount
		= static_cast<XD3D12PixelShader*>(BoundShaderState.RHIPixelShader)->ResourceCount.NumSRV;
	RegisterBoundCount.register_count[(int)EShaderType::SV_Pixel].ConstantBufferCount
		= static_cast<XD3D12PixelShader*>(BoundShaderState.RHIPixelShader)->ResourceCount.NumCBV;

	OutRootSigHash = std::hash<std::string>{}
	(std::string((char*)&RegisterBoundCount, sizeof(XPipelineRegisterBoundCount)));
	
}
static std::unordered_map<std::size_t, std::shared_ptr<XD3D12RootSignature>>HashToTempRootSig;
static std::unordered_map<std::size_t, std::shared_ptr<XD3D12PSOStoreID3DPSO>>HashToID3D12;


std::shared_ptr<XRHIGraphicsPSO> XD3D12PlatformRHI::RHICreateGraphicsPipelineState(const XGraphicsPSOInitializer& PSOInit)
{

	D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc;
	std::size_t PSOHash;
	GetPSODescAndHash(PSOInit, PSODesc, PSOHash);
	
	XPipelineRegisterBoundCount RegisterBoundCount;
	std::size_t BoundHash;
	GetBoundCountAndHash(PSOInit, RegisterBoundCount, BoundHash);

	auto RootIter = HashToTempRootSig.find(BoundHash);
	if (RootIter == HashToTempRootSig.end())
	{
		std::shared_ptr<XD3D12RootSignature>RootSigPtr = std::make_shared<XD3D12RootSignature>();
		RootSigPtr->Create(PhyDevice, RegisterBoundCount);
		HashToTempRootSig[BoundHash] = RootSigPtr;
	}
	else
	{
		PSODesc.pRootSignature = HashToTempRootSig[BoundHash]->GetDXRootSignature();
	}

	THashCombine(PSOHash, BoundHash);

	auto PsoIter = HashToID3D12.find(PSOHash);
	
	if (PsoIter == HashToID3D12.end())
	{
		const std::wstring PSOCacheName = std::to_wstring(PSOHash);
		XD3D12PSOStoreID3DPSO* IPSO = new XD3D12PSOStoreID3DPSO();
		HRESULT hr = PhyDevice->GetD3D12PipelineLibrary()->GetID3D12PipelineLibrary()
			->LoadGraphicsPipeline(PSOCacheName.data(), &PSODesc, IID_PPV_ARGS(IPSO->GetID3DPSO_Address()));

		if (hr == E_INVALIDARG)
		{
			ThrowIfFailed(PhyDevice->GetDXDevice()->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(IPSO->GetID3DPSO_Address())));
			ThrowIfFailed(PhyDevice->GetD3D12PipelineLibrary()->GetID3D12PipelineLibrary()->StorePipeline(PSOCacheName.data(), IPSO->GetID3DPSO()));
		}
		HashToID3D12[PSOHash] = std::shared_ptr<XD3D12PSOStoreID3DPSO>(IPSO);
	}
	return std::make_shared<XD3DGraphicsPSO>(PSOInit, HashToTempRootSig[BoundHash].get(), HashToID3D12[PSOHash].get());
}

std::shared_ptr<XRHIDepthStencilState> XD3D12PlatformRHI::RHICreateDepthStencilState(const XDepthStencilStateInitializerRHI& Initializer)
{
	XD3D12DepthStencilState* DepthStencilState = new XD3D12DepthStencilState;
	D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc = DepthStencilState->Desc;
	memset(&DepthStencilDesc, 0, sizeof(D3D12_DEPTH_STENCIL_DESC));

	DepthStencilDesc.DepthEnable = Initializer.DepthCompFunc != ECompareFunction::CF_Always || Initializer.bEnableDepthWrite;
	DepthStencilDesc.DepthWriteMask = Initializer.bEnableDepthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	DepthStencilDesc.DepthFunc = TranslateCompareFunction(Initializer.DepthCompFunc);
	return std::shared_ptr<XRHIDepthStencilState>(DepthStencilState);
}

std::shared_ptr<XRHIBlendState> XD3D12PlatformRHI::RHICreateBlendState(const XBlendStateInitializerRHI& Initializer)
{
	XD3D12BlendState* BlendState = new XD3D12BlendState;
	D3D12_BLEND_DESC& BlendStateDesc = BlendState->Desc;
	BlendStateDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	BlendStateDesc.RenderTarget[0].BlendEnable = Initializer.RenderTargets[0].RTBlendeEnable;

	BlendStateDesc.RenderTarget[0].BlendOp = TranslateBlendOp(Initializer.RenderTargets[0].RTColorBlendOp);
	BlendStateDesc.RenderTarget[0].SrcBlend = TranslateBlendFactor(Initializer.RenderTargets[0].RTColorSrcBlend);
	BlendStateDesc.RenderTarget[0].DestBlend = TranslateBlendFactor(Initializer.RenderTargets[0].RTColorDestBlend);

	BlendStateDesc.RenderTarget[0].BlendOpAlpha = TranslateBlendOp(Initializer.RenderTargets[0].RTAlphaBlendOp);
	BlendStateDesc.RenderTarget[0].SrcBlendAlpha = TranslateBlendFactor(Initializer.RenderTargets[0].RTAlphaSrcBlend);
	BlendStateDesc.RenderTarget[0].DestBlendAlpha = TranslateBlendFactor(Initializer.RenderTargets[0].RTAlphaDestBlend);
	
	return std::shared_ptr<XRHIBlendState>(BlendState);
}
