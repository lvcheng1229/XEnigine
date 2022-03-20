#pragma once
#include "Runtime/RHI/RHIResource.h"
#include <d3d12.h>

//class FD3D12SamplerState
//class FD3D12RasterizerState

class XD3D12DepthStencilState :public XRHIDepthStencilState
{
public:
	D3D12_DEPTH_STENCIL_DESC Desc;
};

class XD3D12BlendState : public XRHIBlendState
{
public:
	D3D12_BLEND_DESC Desc;
};