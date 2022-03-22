#pragma once
#include <vector>
#include <string>
#include "Runtime/RHI/RHI.h"
struct XShaderCompileInput
{
	EShaderType Shadertype;
	std::wstring SourceFilePath;
	std::string EntryPointName;
};

struct XShaderCompileOutput
{
	EShaderType Shadertype;
	std::vector<uint8>ShaderCode;

	std::size_t SourceCodeHash;
};

extern void CompileGlobalShaderMap();