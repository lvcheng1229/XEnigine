#pragma once
#include <vector>
#include "Runtime/RHI/RHI.h"
#include "Runtime/RenderCore/ShaderCore.h"
struct XShaderCompileInput
{
	EShaderType Shadertype;
	std::wstring SourceFilePath;
	std::string EntryPointName;
	std::string ShaderName;
	XShaderDefines ShaderDefines;
};

struct XShaderCompileOutput
{
	EShaderType Shadertype;
	std::vector<uint8>ShaderCode;

	std::size_t SourceCodeHash;

	XShaderParameterMap ShaderParameterMap;
};

extern void CompileGlobalShaderMap();