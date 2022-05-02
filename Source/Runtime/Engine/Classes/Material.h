#pragma once
#include "Texture.h"
#include "Runtime/Core/Math/Math.h"
#include "Runtime/Engine/Classes/Texture.h"

struct MaterialValueParas
{
	std::string Name;
	float Value[4];
	unsigned int SizeInByte;
};

struct MaterialTexParas
{
	std::string Name;
	GTexture* TexturePtr;
};

class RMaterial;
class GMaterial :public GObject
{
public:
	std::vector<MaterialValueParas>MaterialValueArray;
	std::vector<MaterialTexParas>MaterialTextureArray;
	std::wstring CodePath;
	void CreateGMaterial(const std::wstring& CodePathIn);

	//void CompileRMaterial()const;
	//RMaterial* GetRMaterial()const;
	//RMaterial* MaterialRender;
};

//attach to mesh
class GMaterialInstance :public GObject
{
public:
	std::vector<MaterialValueParas>MaterialFloatArray;
	std::vector<MaterialTexParas>MaterialTextureArray;

	//RMaterial* GetRMaterial()const;
	//GMaterial* MaterialPtr;
};