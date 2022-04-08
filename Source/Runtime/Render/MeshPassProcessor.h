#pragma once
#include <memory>
#include "Runtime/RHI/RHIResource.h"
#include "Runtime/RenderCore/Shader.h"

struct XRHIBoundShaderStateInput_WithoutRT
{
	XRHIVertexLayout* RHIVertexLayout;

	std::shared_ptr<XShaderMappingToRHIShaders>MappingRHIVertexShader;
	std::shared_ptr<XShaderMappingToRHIShaders>MappingRHIPixelShader;
	int32 IndexRHIVertexShader;
	int32 IndexRHIPixelShader;
};

class XGraphicsPSOInitializer_WithoutRT
{
public:
	XRHIBoundShaderStateInput_WithoutRT BoundShaderState;
	XRHIBlendState* BlendState;
	XRHIDepthStencilState* DepthStencilState;
};