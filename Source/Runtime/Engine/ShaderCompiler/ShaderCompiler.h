#pragma once
struct XShaderCompileInput
{
	EShaderType Shadertype;
	std::wstring SourceFilePath;
	std::string EntryPointName;
};

struct XShaderCompileOutput
{
	std::vector<uint8>ShaderCode;
};

extern void CompileGlobalShaderMap();