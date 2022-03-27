#pragma once
#include <d3d12.h>
#include "Runtime/RHI/RHIResource.h"

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