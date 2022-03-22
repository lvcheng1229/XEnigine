#pragma once
#include "Shader.h"

//#pragma region GloablShaderMap
//
class XGloablShaderMapStoreShadersInfoInFileUnit :public XShaderMapStoreShadersInfoInFileUnit
{
	friend class XGlobalShaderMapInFileUnit;
public:
	XGloablShaderMapStoreShadersInfoInFileUnit(std::size_t InHashedSourceFileIndex) :
		HashedSourceFileIndex(InHashedSourceFileIndex) {};
private:
	std::size_t HashedSourceFileIndex;
};

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
	XGlobalShaderMapInFileUnit* FindOrAddShaderMapFileUnit(const XShaderInfosUsedToCompile* ShaderInfoToCompile);
	//
	//TShaderReference<XXShader> GetShader(XShaderInfosUsedToCompile* ShaderInfo, int32 PermutationId = 0) const;
	//
	//template<typename XXShaderClass>
	//TShaderReference<XXShaderClass> GetShader(int32 PermutationId = 0)const
	//{
	//	TShaderReference<XXShader> Shader = GetShader(&XXShaderClass::StaticShaderInfosUsedToCompile, PermutationId);
	//	return TShaderReference<XXShaderClass>::Cast(Shader);
	//}
	//
	//inline std::unordered_map<std::size_t, XGlobalShaderMapInFileUnit*>& GetGlobalShaderMap_HashMap()
	//{
	//	return MapFromHashedFileIndexToPtr;
	//}
private:
	std::unordered_map<std::size_t, XGlobalShaderMapInFileUnit*>MapFromHashedFileIndexToPtr;
};

//extern XGlobalShaderMapInProjectUnit* GetGlobalShaderMap();
extern class XGlobalShaderMapInProjectUnit* GGlobalShaderMap;


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