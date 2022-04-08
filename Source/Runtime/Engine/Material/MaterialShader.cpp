#include "MaterialShared.h"
#include "MaterialShaderMapSet.h"
#include "Runtime/RenderCore/VertexFactory.h"

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

	void CreateSet(XMaterialShaderMapSet& MapSet, const XMaterialShaderParameters_ForIndex& Parameters)
	{
		const std::list<XShaderInfo*>& MeshMaterialShaderInfos = XShaderInfo::GetShaderInfo_LinkedList(
			XShaderInfo::EShaderTypeForDynamicCast::MeshMaterial);

		for (auto iter = MeshMaterialShaderInfos.begin(); iter != MeshMaterialShaderInfos.end(); iter++)
		{
			//ShouldCompilePermutaion
			MapSet.ShaderInfos.push_back(*iter);
		}

		std::vector<XVertexFactoryShaderInfo*>& FactoryInfos = XVertexFactoryShaderInfo::GetVertexFactoryShaderInfo_Array();
		for (auto iter = FactoryInfos.begin(); iter != FactoryInfos.end(); ++iter)
		{

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

void XMaterialShaderMapping_MatUnit::Compile(const XShaderCompileSetting& ShaderCompileSetting)
{
	//SubmitCompileJobs()
	{
		
		XMaterialShaderParameters_ForIndex MaterialParameters;
		//const XMaterialShaderParameters_ForIndex& MaterialParameters;

		//Temp
		{
			memset(&MaterialParameters, 0, sizeof(XMaterialShaderParameters_ForIndex));
			MaterialParameters.MaterialDomain = EMaterialDomain::Surface;
		}


		const XMaterialShaderMapSet& MapSet = GetMaterialShaderMapSet(MaterialParameters);
		for (auto ShaderInfo : MapSet.ShaderInfos)
		{
			XShaderCompileInput Input;
			Input.SourceFilePath = ShaderInfo->GetSourceFileName();
			Input.EntryPointName = ShaderInfo->GetEntryName();
			Input.Shadertype = ShaderInfo->GetShaderType();
			Input.ShaderName = ShaderInfo->GetShaderName();
			Input.CompileSettings = ShaderCompileSetting;
			
			XShaderCompileOutput Output;//TODO if(!GetContent().HashShader()), then Compile

			CompileMaterialShader(Input, Output);

			//step1 store compile shader code
			std::size_t HashIndex = this->GetShaderMapStoreCodes()->AddShaderCompilerOutput(Output);

			//step2 store xshader
			XXShader* Shader = ShaderInfo->CtorPtr(XShaderInitlizer(ShaderInfo, Output, HashIndex));
			this->GetShaderMapStoreXShaders()->FindOrAddXShader(ShaderInfo->GetHashedShaderNameIndex(), Shader);
		}

		this->InitRHIShaders_InlineCode();
	}
}