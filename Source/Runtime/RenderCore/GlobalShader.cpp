#include "GlobalShader.h"

XGlobalShaderMapInProjectUnit* GGlobalShaderMap = nullptr;
XGlobalShaderMapInProjectUnit* GetGlobalShaderMap()
{
	return GGlobalShaderMap;
}

XGlobalShaderMapInProjectUnit::~XGlobalShaderMapInProjectUnit()
{
	for (auto iter = MapFromHashedFileIndexToPtr.begin(); iter != MapFromHashedFileIndexToPtr.end(); iter++)
	{
		delete iter->second;
	}
	MapFromHashedFileIndexToPtr.clear();
}

XXShader* XGlobalShaderMapInProjectUnit::GlobalShaderMapFindOrAddShader(const XShaderInfosUsedToCompile* ShaderInfoToCompile, int32 PermutationId, XXShader* Shader)
{
	const std::size_t HashedFileIndex = ShaderInfoToCompile->GetHashedFileIndex();
	auto iter = MapFromHashedFileIndexToPtr.find(HashedFileIndex);
	if (iter == MapFromHashedFileIndexToPtr.end())
	{
		MapFromHashedFileIndexToPtr[HashedFileIndex] = new XGlobalShaderMapInFileUnit(HashedFileIndex);
	}
	return MapFromHashedFileIndexToPtr[HashedFileIndex]->GetShaderInfo()->FindOrAddShader(ShaderInfoToCompile->GetHashedEntryIndex(), Shader, 0);
}
