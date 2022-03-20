#include "D3D12PlatformRHI.h"
#include "D3D12State.h"
#include "d3dx12.h"

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
