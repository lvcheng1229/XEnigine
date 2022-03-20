#pragma once
#include "Runtime/DataStruct/XRefCountPtr.h"
#include "Runtime/Core/Template/XEngineTemplate.h"
#include "RHIDefines.h"
enum class EShaderType
{
	SV_Vertex = 0,
	SV_Pixel,
	SV_Compute,
	SV_ShaderCount
};

extern void RHIInit();


using EShaderType_Underlying = std::underlying_type<EShaderType>::type;

struct XDepthStencilStateInitializerRHI
{
	bool bEnableDepthWrite;
	ECompareFunction DepthCompFunc;
	XDepthStencilStateInitializerRHI(
		bool bEnableDepthWriteIn,
		ECompareFunction DepthCompFuncIn) :
		bEnableDepthWrite(bEnableDepthWriteIn),
		DepthCompFunc(DepthCompFuncIn) {}
};
