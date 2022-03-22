#pragma once

#include "Runtime/RenderCore/ShaderCore.h"
#include <vector>
#include <array>
#include <d3d12.h>
#include "Runtime/RHI/RHIResource.h"
#include "D3D12Rootsignature.h"

#define VERTEX_LAYOUT_MAX 16
class XD3D12VertexLayout :public XRHIVertexLayout
{
public:
	std::array<D3D12_INPUT_ELEMENT_DESC, VERTEX_LAYOUT_MAX>VertexElements;
};


class XD3D12VertexShader :public XRHIVertexShader
{
public:
	//EShaderType ShaderType; ?? XRHIShader
	std::vector<uint8>BinaryCode;
	D3D12_SHADER_BYTECODE D3DByteCode;
	XShaderResourceCount ResourceCount;
};

class XD3D12PixelShader : public XRHIPixelShader
{
public:
	//EShaderType ShaderType; ?? XRHIShader
	std::vector<uint8>BinaryCode;
	D3D12_SHADER_BYTECODE D3DByteCode;
	XShaderResourceCount ResourceCount;
};

class XD3D12BoundShaderState
{
public:
	const XD3D12RootSignature* RootSignature;
	
	//CacheLink
	XD3D12VertexLayout* VertexLayout;
	XD3D12VertexShader* VertexShader;
	XD3D12PixelShader* PiexlShader;
};