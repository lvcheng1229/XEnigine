#pragma once
#include "Runtime/RenderCore/Shader.h"

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

};