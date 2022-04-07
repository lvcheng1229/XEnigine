#pragma once
#include "Runtime/RenderCore/Shader.h"
#include "Runtime/Engine/Classes/EngineTypes.h"

enum class EMaterialDomain
{
	Surface,
	PostProcess,
};

struct XMaterialShaderParameters_ForIndex
{
	EMaterialDomain MaterialDomain;
	XMaterialShadingModelField ShadingModels;

	uint64 bHasNormalConnected : 1;
	uint64 bHasEmissiveColorConnected : 1;
};

class XMaterialShaderMappingToXShaders :public XShaderMappingToXShaders
{
public:

};

class XMaterialShaderMapping_MatUnit :public TShaderMapping<XMaterialShaderMappingToXShaders>
{
public:
	void Compile(const XShaderCompileSetting& ShaderCompileSetting);
};

class RMaterial
{
public:
	bool BeginCompileShaderMap();
private:
	std::shared_ptr<XMaterialShaderMapping_MatUnit>RThreadShaderMap;
};

class XMaterialResource :public RMaterial
{
public:
private:
	//UMaterial* Material;
	//UMaterialInstance* MaterialInstance;
};