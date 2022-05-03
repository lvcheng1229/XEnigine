#include "Material.h"

void GMaterialInstance::SetMaterialValueFloat(const std::string& ValueName, float Value)
{
	for (auto iter = MaterialFloatArray.begin(); iter != MaterialFloatArray.end(); iter++)
	{
		if (iter->Name == ValueName)
		{
			iter->Value.push_back(Value);
			return;
		}
	}
	X_Assert(false);
}

void GMaterialInstance::SetMaterialValueFloat2(const std::string& ValueName, XVector2 Value)
{
	for (auto iter = MaterialFloatArray.begin(); iter != MaterialFloatArray.end(); iter++)
	{
		if (iter->Name == ValueName)
		{
			iter->Value.push_back(Value.x);
			iter->Value.push_back(Value.y);
			return;
		}
	}
	X_Assert(false);
}

void GMaterialInstance::SetMaterialValueFloat3(const std::string& ValueName, XVector3 Value)
{
	for (auto iter = MaterialFloatArray.begin(); iter != MaterialFloatArray.end(); iter++)
	{
		if (iter->Name == ValueName)
		{
			iter->Value.push_back(Value.x);
			iter->Value.push_back(Value.y);
			iter->Value.push_back(Value.z);
			return;
		}
	}
	X_Assert(false);
}

void GMaterialInstance::SetMaterialValueFloat4(const std::string& ValueName, XVector4 Value)
{
	for (auto iter = MaterialFloatArray.begin(); iter != MaterialFloatArray.end(); iter++)
	{
		if (iter->Name == ValueName)
		{
			iter->Value.push_back(Value.x);
			iter->Value.push_back(Value.y);
			iter->Value.push_back(Value.z);
			iter->Value.push_back(Value.w);
			return;
		}
	}
	X_Assert(false);
}

void GMaterialInstance::SetMaterialTexture2D(const std::string& TexName, std::shared_ptr<GTexture2D> TexPtrIn)
{
	for (auto iter = MaterialTextureArray.begin(); iter != MaterialTextureArray.end(); iter++)
	{
		if (iter->Name == TexName)
		{
			iter->TexturePtr = TexPtrIn;
			return;
		}
	}
	X_Assert(false);
}




