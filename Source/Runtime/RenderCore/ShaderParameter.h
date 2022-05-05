#pragma once
#include "ShaderCore.h"
#include "Runtime/HAL/Mch.h"

class XShaderVariableParameter
{
public:
	XShaderVariableParameter():BufferIndex(0), VariableOffsetInBuffer(0),NumBytes(0) {}
	inline void Bind(const XShaderParameterMap& ParameterMap, const char* ParameterName)
	{
		auto iter = ParameterMap.MapNameToParameter.find(ParameterName);
		if (iter != ParameterMap.MapNameToParameter.end())
		{
			BufferIndex = iter->second.BufferIndex;
			VariableOffsetInBuffer = iter->second.VariableOffsetInBuffer;
			NumBytes = iter->second.VariableSize;
		}
	}

	inline uint16 GetBufferIndex()const { return BufferIndex; }
	inline uint16 GetVariableOffsetInBuffer()const { return VariableOffsetInBuffer; }
	inline uint16 GetNumBytes()const { return NumBytes; }
private:
	uint16 BufferIndex;
	uint16 VariableOffsetInBuffer;
	uint16 NumBytes;
};


class XSRVCBVUAVParameter
{
public:
	XSRVCBVUAVParameter() :ResourceIndex(0), ResourceNum(0) {}
	inline void Bind(const XShaderParameterMap& ParameterMap, const char* ParameterName)
	{
		auto iter = ParameterMap.MapNameToParameter.find(ParameterName);
		if (iter != ParameterMap.MapNameToParameter.end())
		{
			ResourceIndex = iter->second.BufferIndex;
			ResourceNum = iter->second.ResourceCount;
			return;
		}
		X_Assert(false);
	}
	uint16 GetResourceIndex()const { return ResourceIndex; }
	uint16 GetResourceNum()const { return ResourceNum; };
private:
	uint16 ResourceIndex;
	uint16 ResourceNum;
};


using TextureParameterType = XSRVCBVUAVParameter;
using SRVParameterType = XSRVCBVUAVParameter;
using UAVParameterType = XSRVCBVUAVParameter;
using CBVParameterType = XSRVCBVUAVParameter;

template<typename TRHICmdList>
inline void SetShaderConstantBufferParameter(
	TRHICmdList& RHICmdList,
	EShaderType ShaderType,
	const CBVParameterType& ConstantBufferParameter,
	XRHIConstantBuffer* RHIConstantBuffer,
	uint32 ElementIndex = 0
)
{
	X_Assert(ConstantBufferParameter.GetResourceNum() > 0);
	RHICmdList.SetConstantBuffer(ShaderType, ConstantBufferParameter.GetResourceIndex() + ElementIndex, RHIConstantBuffer);
}

template<typename TRHICmdList, class ParameterType>
inline void SetShaderValue(
	TRHICmdList& RHICmdList,
	EShaderType ShaderType,
	const XShaderVariableParameter& ShaderVariableParameter,
	ParameterType& Value,
	uint32 ElementIndex = 0
)
{
	X_Assert(sizeof(ParameterType) == ShaderVariableParameter.GetNumBytes());
	RHICmdList.SetShaderValue(
		ShaderType,
		ShaderVariableParameter.GetBufferIndex(),
		ShaderVariableParameter.GetVariableOffsetInBuffer(),
		ShaderVariableParameter.GetNumBytes(),
		&Value);
}
template<typename TRHICmdList>
inline void SetTextureParameter(
	TRHICmdList& RHICmdList,
	EShaderType ShaderType,
	const TextureParameterType& TextureParameter,
	XRHITexture* TextureRHI,
	uint32 ElementIndex = 0
)
{
	X_Assert(TextureParameter.GetResourceNum() > 0);
	RHICmdList.SetShaderTexture(ShaderType, TextureParameter.GetResourceIndex() + ElementIndex, TextureRHI);
}

template<typename TRHICmdList>
inline void SetShaderUAVParameter(
	TRHICmdList& RHICmdList,
	EShaderType ShaderType,
	const UAVParameterType& UAVParameter,
	XRHIUnorderedAcessView* InUAV,
	uint32 ElementIndex = 0
)
{
	X_Assert(UAVParameter.GetResourceNum() > 0);
	RHICmdList.SetShaderUAV(ShaderType, UAVParameter.GetResourceIndex() + ElementIndex, InUAV);
}