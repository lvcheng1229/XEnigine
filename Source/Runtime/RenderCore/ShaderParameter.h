#pragma once
#include "ShaderCore.h"
#include "Runtime/HAL/Mch.h"
class XSRVParameter
{
public:
	XSRVParameter() :ResourceIndex(0), ResourceNum(0) {}
	inline void Bind(const XShaderParameterMap& ParameterMap, const char* ParameterName)
	{
		auto iter = ParameterMap.MapNameToParameter.find(ParameterName);
		if (iter != ParameterMap.MapNameToParameter.end())
		{
			ResourceIndex = iter->second.BufferIndex;
			ResourceNum = iter->second.ResourceCount;
		}
	}
	uint16 GetResourceIndex()const { return ResourceIndex; }
	uint16 GetResourceNum()const { return ResourceNum; };
private:
	uint16 ResourceIndex;
	uint16 ResourceNum;
};

template<typename TRHICmdList>
inline void SetTextureParameter(
	TRHICmdList& RHICmdList,
	EShaderType ShaderType,
	const XSRVParameter& TextureParameter,
	XRHITexture* TextureRHI,
	uint32 ElementIndex = 0
)
{
	X_Assert(TextureParameter.GetResourceNum() > 0);
	RHICmdList.SetShaderTexture(ShaderType, TextureParameter.GetResourceIndex() + ElementIndex, TextureRHI);
}