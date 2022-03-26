#pragma once
#include "Shader.h"

//#pragma region GloablShaderMap
//
class XGloablShaderMapStoreShadersInfoInFileUnit :public XShaderMapStoreXShaders
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
	XGlobalShaderMapInFileUnit* FindOrAddShaderMapFileUnit(const XShaderInfos* ShaderInfoToCompile);
	
	TShaderReference<XXShader> GetShader(XShaderInfos* ShaderInfo, int32 PermutationId = 0) const;
	
	template<typename XXShaderClass>
	TShaderReference<XXShaderClass> GetShader(int32 PermutationId = 0)const
	{
		TShaderReference<XXShader> Shader = GetShader(&XXShaderClass::StaticShaderInfos, PermutationId);
		return TShaderReference<XXShaderClass>::Cast(Shader);
	}
	
	inline std::unordered_map<std::size_t, XGlobalShaderMapInFileUnit*>& GetGlobalShaderMap_HashMap()
	{
		return MapFromHashedFileIndexToPtr;
	}
private:
	std::unordered_map<std::size_t, XGlobalShaderMapInFileUnit*>MapFromHashedFileIndexToPtr;
};

extern XGlobalShaderMapInProjectUnit* GetGlobalShaderMap();
extern class XGlobalShaderMapInProjectUnit* GGlobalShaderMap;


class XGloablShaderInfos : public XShaderInfos
{
public:
	XGloablShaderInfos(
		const char* InShaderName,
		const wchar_t* InSourceFileName,
		const char* InEntryName,
		EShaderType ShaderType,
		XShaderCustomConstructFunctionPtr InCtorPtr,
		ModifyShaderCompileDefinesFunctionPtr InModifyDefinesPtr) :
		XShaderInfos(
			EShaderTypeForDynamicCast::Global,
			InShaderName,
			InSourceFileName,
			InEntryName,
			ShaderType,
			InCtorPtr,
			InModifyDefinesPtr) {}
};

class XGloablShader : public XXShader
{
public:
	XGloablShader(const XShaderInitlizer& Initializer):XXShader(Initializer) {}
	using ShaderInfos = XGloablShaderInfos;
};