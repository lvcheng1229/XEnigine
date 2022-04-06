#pragma once
#include "Runtime/RenderCore/Shader.h"

enum class MaterialDomain
{
	Surface,
	PostProcess,
};

class XMaterialShaderMap
{
public:
	void Compile();
};

class RMaterial
{
public:
	bool BeginCompileShaderMap();
private:
	std::shared_ptr<XMaterialShaderMap>RThreadShaderMap;
};

class XMaterialResource :public RMaterial
{
public:
private:
	//UMaterial* Material;
	//UMaterialInstance* MaterialInstance;
};