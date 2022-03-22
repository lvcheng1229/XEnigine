#include "GlobalShader.h"

XGlobalShaderMapInProjectUnit* GGlobalShaderMap = nullptr;
//XGlobalShaderMapInProjectUnit* GetGlobalShaderMap()
//{
//	return GGlobalShaderMap;
//}
//
XGlobalShaderMapInProjectUnit::~XGlobalShaderMapInProjectUnit()
{
	for (auto iter = MapFromHashedFileIndexToPtr.begin(); iter != MapFromHashedFileIndexToPtr.end(); iter++)
	{
		delete iter->second;
	}
	MapFromHashedFileIndexToPtr.clear();
}

XGlobalShaderMapInFileUnit* XGlobalShaderMapInProjectUnit::FindOrAddShaderMapFileUnit(const XShaderInfosUsedToCompile* ShaderInfoToCompile)
{
	std::size_t HashedFileIndex = ShaderInfoToCompile->GetHashedFileIndex();
	auto iter = MapFromHashedFileIndexToPtr.find(HashedFileIndex);
	XGlobalShaderMapInFileUnit* ShaderMapInFileUnit;
	if (iter == MapFromHashedFileIndexToPtr.end())
	{
		ShaderMapInFileUnit = new XGlobalShaderMapInFileUnit(HashedFileIndex);
		MapFromHashedFileIndexToPtr[HashedFileIndex] = ShaderMapInFileUnit;
	}
	else
	{
		ShaderMapInFileUnit = iter->second;
	}
	return ShaderMapInFileUnit;
}
//
//TShaderReference<XXShader> XGlobalShaderMapInProjectUnit::GetShader(XShaderInfosUsedToCompile* ShaderInfo, int32 PermutationId) const
//{
//	auto iter = MapFromHashedFileIndexToPtr.find(ShaderInfo->GetHashedFileIndex());
//	if (iter != MapFromHashedFileIndexToPtr.end())
//	{
//		XGlobalShaderMapInFileUnit* MapInFileUnit = iter->second;
//		return TShaderReference<XXShader>(
//			MapInFileUnit->GetShaderInfo()->GetShader(ShaderInfo->GetHashedEntryIndex(), 0),
//			MapInFileUnit);
//	}
//	return TShaderReference<XXShader>();
//}



