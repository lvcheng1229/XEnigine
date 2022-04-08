#pragma once
#include "MeshMaterialShader.h"

template<bool TempType>
class TBasePassPS : public XMeshMaterialShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new TBasePassPS<TempType>(Initializer);
	}

	static ShaderInfos StaticShaderInfos;

	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	TBasePassPS(const XShaderInitlizer& Initializer):
		XMeshMaterialShader(Initializer)
	{

	}
};

template<bool TempType>
class TBasePassVS : public XMeshMaterialShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new TBasePassVS<TempType>(Initializer);
	}

	static ShaderInfos StaticShaderInfos;

	static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}
public:
	TBasePassVS(const XShaderInitlizer& Initializer) :
		XMeshMaterialShader(Initializer)
	{

	}
};