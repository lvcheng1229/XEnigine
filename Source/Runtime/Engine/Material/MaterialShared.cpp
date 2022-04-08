#include <memory>
#include "MaterialShared.h"
#include "MaterialShaderMapSet.h"


//temp
#include <filesystem>
#include <fstream>
//

#define TEMP_MATERIAL_PATH L"E:/XEngine/XEnigine/MaterialShaders"



#define TEMP_SHADER_PATH L"E:/XEngine/XEnigine/MaterialShaders/Material.hlsl"
void RMaterial::BeginCompileShaderMap()
{
	std::shared_ptr<XMaterialShaderMapping_MatUnit> NewShaderMap = std::make_shared<XMaterialShaderMapping_MatUnit>();
	std::shared_ptr<XShaderCompileSetting> MaterialCompileSetting = std::make_shared<XShaderCompileSetting>();

	NewShaderMap->AssignShaderMappingXShader(new XShaderMappingToXShaders());

	std::string TemplShaderCode;
	{
		std::ifstream FileCode(std::filesystem::path(TEMP_SHADER_PATH), std::ios::ate);
		std::ifstream::pos_type FileSize = FileCode.tellg();
		FileCode.seekg(0, std::ios_base::beg);
		TemplShaderCode.resize(FileSize, '\n');
		FileCode.read(TemplShaderCode.data(), FileSize);
		FileCode.close();
	}
	
	MaterialCompileSetting->IncludePathToCode["Generated/Material.hlsl"] = TemplShaderCode;
	NewShaderMap->Compile(*MaterialCompileSetting);

	//set shader mapping
	RThreadShaderMap = NewShaderMap;
}

void RMaterial::GetShaderInfos(const XMaterialShaderInfo_Set& ShaderInfos, XMaterialShader_Set& ShaderOut)
{
	const XMaterialShaderMapping_MatUnit* ShaderMap = RThreadShaderMap.get();
	ShaderOut.ShaderMap = ShaderMap;

	const XShaderMappingToXShaders* MappingXShaders = ShaderMap->GetShaderMapStoreXShaders();//TODO Mesh Shader
	for (int32 index = 0; index < (int32)EShaderType::SV_ShaderCount; ++index)
	{
		const XShaderInfo* ShaderInfo = ShaderInfos.ShaderInfoSet[index];
		if (ShaderInfo != nullptr)
		{
			ShaderOut.XShaderSet[index] = MappingXShaders->GetXShader(ShaderInfo->GetHashedShaderNameIndex());
		}
	}
}