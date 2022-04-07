#include <memory>
#include "MaterialShared.h"
#include "MaterialShaderMapSet.h"

#define TEMP_MATERIAL_PATH L"E:/XEngine/XEnigine/MaterialShaders"

class XMaterialShaderMappingSetsManager
{
public:
	static XMaterialShaderMappingSetsManager& Get()
	{
		static XMaterialShaderMappingSetsManager Instance;
		return Instance;
	}

	std::unordered_map<std::size_t, std::size_t>HashToIndex;
	std::vector<std::unique_ptr<XMaterialShaderMapSet>>ShaderMapSets;

	XMaterialShaderMapSet* FindSet(std::size_t HashIndex)
	{
		auto iter = HashToIndex.find(HashIndex);//???
		if (iter != HashToIndex.end())
		{
			return ShaderMapSets[iter->second].get();
		}
		return nullptr;
	}
	
	void CreateSet(XMaterialShaderMapSet& MapSet,const XMaterialShaderParameters_ForIndex& Parameters)
	{
		const std::list<XShaderInfo*>& MaterialShaderInfos = XShaderInfo::GetShaderInfo_LinkedList(
			XShaderInfo::EShaderTypeForDynamicCast::Material);

		for (auto iter = MaterialShaderInfos.begin(); iter != MaterialShaderInfos.end(); iter++)
		{
			//ShouldCompilePermutaion
			MapSet.ShaderInfos.push_back(*iter);
		}
	}

	const XMaterialShaderMapSet& GetSet(const XMaterialShaderParameters_ForIndex& Parameters)
	{
		const std::size_t HashIndex = std::hash<std::string>{}(std::string((char*)(&Parameters)));
		XMaterialShaderMapSet* MapSet = FindSet(HashIndex);

		if (MapSet == nullptr)
		{
			MapSet = new XMaterialShaderMapSet();
			CreateSet(*MapSet, Parameters);
			ShaderMapSets.push_back(std::unique_ptr<XMaterialShaderMapSet>(MapSet));
			HashToIndex[HashIndex] = ShaderMapSets.size() - 1;
		}
		return *MapSet;
	}
};

const XMaterialShaderMapSet& GetMaterialShaderMapSet(const XMaterialShaderParameters_ForIndex& MaterialParameters)
{
	return XMaterialShaderMappingSetsManager::Get().GetSet(MaterialParameters);
}

bool RMaterial::BeginCompileShaderMap()
{
	std::shared_ptr<XMaterialShaderMapping_MatUnit> NewShaderMap = std::make_shared<XMaterialShaderMapping_MatUnit>();
	std::shared_ptr<XShaderCompileSetting> MaterialCompileSetting = std::make_shared<XShaderCompileSetting>();

	std::string TemplShaderCode;
	MaterialCompileSetting->IncludePathToCode[L"Generated/Material.hlsl"] = TemplShaderCode;

	NewShaderMap->Compile(*MaterialCompileSetting);
	return false;
}