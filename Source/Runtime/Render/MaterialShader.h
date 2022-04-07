#pragma once
#include "Runtime/RenderCore/Shader.h"

class XMaterialShaderInfo :public XShaderInfo
{
public:
};

class XMaterialShader :public XXShader
{
public:
	using ShaderInfos = XMaterialShaderInfo;
};



//struct FMaterialShaderTypes
struct XMaterialShaderInfo_Set
{
	XShaderInfo* ShaderInfoSet[(int)EShaderType::SV_ShaderCount];
};

//struct FMaterialShaders
struct XMaterialShader_Set
{
	XShaderMappingBase* ShaderMap;
	XXShader* XShaderSet[(int)EShaderType::SV_ShaderCount];
};