#pragma once
#include "Shader.h"

#pragma region GloablShaderMap

class XXShader;
//FGlobalShaderContent
class XGloablShaderMapStoreShadersInfoInFileUnit :public XShaderMapStoreShadersInfoInFileUnit
{
	friend class XGlobalShaderMapInFileUnit;
public:
	XXShader ShadersInfoFindOrAddShader() {}
private:
	XGloablShaderMapStoreShadersInfoInFileUnit(std::size_t InHashedSourceFileIndex) :
		HashedSourceFileIndex(InHashedSourceFileIndex) {};
	std::size_t HashedSourceFileIndex;
};

//FGlobalShaderMapSection
class XGlobalShaderMapInFileUnit :public TShaderMap<XGloablShaderMapStoreShadersInfoInFileUnit>
{
public:
	XGlobalShaderMapInFileUnit(const std::size_t& InHashedSourceFileIndex)
	{
		AssignShadersInfo(new XGloablShaderMapStoreShadersInfoInFileUnit(InHashedSourceFileIndex));
	}
};

//FGlobalShaderMap
class XGlobalShaderMapInProjectUnit 
{
public:
	~XGlobalShaderMapInProjectUnit();
	XXShader* GlobalShaderMapFindOrAddShader(const XShaderInfosUsedToCompile* ShaderInfoToCompile, int32 PermutationId, XXShader* Shader);
private:
	std::unordered_map<std::size_t, XGlobalShaderMapInFileUnit*>MapFromHashedFileIndexToPtr;
};

extern XGlobalShaderMapInProjectUnit* GetGlobalShaderMap();
extern class XGlobalShaderMapInProjectUnit* GGlobalShaderMap;
#pragma endregion GloablShaderMap



class XGloablShaderInfosUsedToCompile : public XShaderInfosUsedToCompile
{
public:
	XGloablShaderInfosUsedToCompile(
		const char* InShaderName,
		const wchar_t* InSourceFileName,
		const char* InEntryName,
		EShaderType ShaderType) :
		XShaderInfosUsedToCompile(
			EShaderTypeForDynamicCast::Global,
			InShaderName,
			InSourceFileName,
			InEntryName,
			ShaderType) {}
};

class XGloablShader : public XXShader
{
public:
	using ShaderInfosUsedToCompile = XGloablShaderInfosUsedToCompile;
};