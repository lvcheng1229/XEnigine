#pragma once
#include "Texture.h"
#include "Runtime/Core/Math/Math.h"

struct MaterialValueParas
{
	std::string Name;
	std::vector<float>Value;
	unsigned int BufferIndex;
	unsigned int VariableOffsetInBuffer;
	unsigned int SizeInByte;
};

struct MaterialTexParas
{
	std::string Name;
	unsigned int ResourceIndex;
	std::shared_ptr<GTexture2D>TexturePtr;
};

class RMaterial;
class GMaterial :public GObject
{
public:
	std::vector<MaterialValueParas>MaterialValueArray;
	std::vector<MaterialTexParas>MaterialTextureArray;
	std::wstring CodePath;
	//void CreateGMaterial(const std::wstring& CodePathIn);

	//void CompileRMaterial()const;
	//RMaterial* GetRMaterial()const;
	//RMaterial* MaterialRender;
};

//attach to mesh
class GMaterialInstance :public GObject
{
public:
	GMaterialInstance(std::shared_ptr<GMaterial>MaterialPtrIn)
	{
		MaterialPtr = MaterialPtrIn;
		MaterialFloatArray = MaterialPtrIn->MaterialValueArray;
		MaterialTextureArray = MaterialPtrIn->MaterialTextureArray;
	}

	void SetMaterialValueFloat(const std::string& ValueName, float Value);
	void SetMaterialValueFloat2(const std::string& ValueName, XVector2 Value);
	void SetMaterialValueFloat3(const std::string& ValueName, XVector3 Value);
	void SetMaterialValueFloat4(const std::string& ValueName, XVector4 Value);
	void SetMaterialTexture2D(const std::string& TexName, std::shared_ptr<GTexture2D> TexPtrIn);

	std::vector<MaterialValueParas>MaterialFloatArray;
	std::vector<MaterialTexParas>MaterialTextureArray;

	std::shared_ptr<GMaterial>MaterialPtr;
	//RMaterial* GetRMaterial()const;
};