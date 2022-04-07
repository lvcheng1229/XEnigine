#pragma once
#include "MaterialShader.h"

class XMeshMaterialShaderInfo :public XShaderInfo
{
public:
};

class XMeshMaterialShader :public XMaterialShader
{
public:
	using ShaderInfos = XMeshMaterialShaderInfo;
};