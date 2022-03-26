#include "D3D12PlatformRHI.h"
#include "Runtime/RenderCore/ShaderCore.h"
#include "D3D12Shader.h"

//std::shared_ptr<XRHIComputeShader> XD3D12PlatformRHI::RHICreateComputeShader(XArrayView<uint8> Code)
//{
//	XD3D12VertexShader* VertexShader = new XD3D12VertexShader;
//	XShaderCodeReader ShaderCodeReader(Code);
//
//	const XShaderResourceCount* ResourceCount = (const XShaderResourceCount*)(ShaderCodeReader.FindOptionalData(
//		XShaderResourceCount::Key, sizeof(XShaderResourceCount)));
//	if (ResourceCount == 0) { X_Assert(false); return nullptr; }
//
//	const uint8* CodaData = (const uint8*)Code.data();
//	VertexShader->BinaryCode.insert(VertexShader->BinaryCode.end(), &CodaData[0], &CodaData[Code.size()]);
//	VertexShader->ResourceCount = *ResourceCount;
//
//	SIZE_T CodeSize = ShaderCodeReader.GetActualShaderCodeSize();
//	D3D12_SHADER_BYTECODE ShaderBytecode;
//	ShaderBytecode.pShaderBytecode = VertexShader->BinaryCode.data();
//	ShaderBytecode.BytecodeLength = CodeSize;
//	VertexShader->D3DByteCode = ShaderBytecode;
//	return std::shared_ptr<XRHIVertexShader>(VertexShader);
//}

std::shared_ptr<XRHIVertexShader> XD3D12PlatformRHI::RHICreateVertexShader(XArrayView<uint8> Code)
{
	XD3D12VertexShader* VertexShader = new XD3D12VertexShader;
	XShaderCodeReader ShaderCodeReader(Code);

	const XShaderResourceCount* ResourceCount = (const XShaderResourceCount*)(ShaderCodeReader.FindOptionalData(
		XShaderResourceCount::Key, sizeof(XShaderResourceCount)));
	if (ResourceCount == 0) { X_Assert(false); return nullptr; }

	const uint8* CodaData = (const uint8*)Code.data();
	VertexShader->BinaryCode.insert(VertexShader->BinaryCode.end(), &CodaData[0], &CodaData[Code.size()]);
	VertexShader->ResourceCount = *ResourceCount;

	SIZE_T CodeSize = ShaderCodeReader.GetActualShaderCodeSize();
	D3D12_SHADER_BYTECODE ShaderBytecode;
	ShaderBytecode.pShaderBytecode = VertexShader->BinaryCode.data();
	ShaderBytecode.BytecodeLength = CodeSize;
	VertexShader->D3DByteCode = ShaderBytecode;
	return std::shared_ptr<XRHIVertexShader>(VertexShader);
}

std::shared_ptr<XRHIPixelShader> XD3D12PlatformRHI::RHICreatePixelShader(XArrayView<uint8> Code)
{
	XD3D12PixelShader* PixelShader = new XD3D12PixelShader;
	XShaderCodeReader ShaderCodeReader(Code);

	const XShaderResourceCount* ResourceCount = (const XShaderResourceCount*)(ShaderCodeReader.FindOptionalData(
		XShaderResourceCount::Key, sizeof(XShaderResourceCount)));
	if (ResourceCount == 0) { return nullptr; }

	const uint8* CodaData = (const uint8*)Code.data();
	PixelShader->BinaryCode.insert(PixelShader->BinaryCode.end(), &CodaData[0], &CodaData[Code.size()]);
	PixelShader->ResourceCount = *ResourceCount;

	SIZE_T CodeSize = ShaderCodeReader.GetActualShaderCodeSize();
	D3D12_SHADER_BYTECODE ShaderBytecode;
	ShaderBytecode.pShaderBytecode = PixelShader->BinaryCode.data();
	ShaderBytecode.BytecodeLength = CodeSize;
	PixelShader->D3DByteCode = ShaderBytecode;
	return std::shared_ptr<XRHIPixelShader>(PixelShader);
}