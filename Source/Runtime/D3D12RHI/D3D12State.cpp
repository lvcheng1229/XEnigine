#include "D3D12PlatformRHI.h"
#include "D3D12State.h"

static D3D12_COMPARISON_FUNC TranslateCompareFunction(ECompareFunction CompareFunction)
{
	switch (CompareFunction)
	{
	case ECompareFunction::CF_Greater: return D3D12_COMPARISON_FUNC_GREATER;
	case ECompareFunction::CF_Always:return D3D12_COMPARISON_FUNC_ALWAYS;
	default: X_Assert(false); return D3D12_COMPARISON_FUNC_ALWAYS;
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