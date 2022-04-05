#include "MaterialShared.h"

bool RMaterial::BeginCompileShaderMap()
{
	std::shared_ptr<XMaterialShaderMap> NewShaderMap = std::make_shared<XMaterialShaderMap>();
	NewShaderMap->Compile();
	return false;
}