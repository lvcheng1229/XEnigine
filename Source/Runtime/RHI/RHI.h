#pragma once
#include "Runtime/DataStruct/XRefCountPtr.h"
#include "Runtime/Core/Template/XEngineTemplate.h"
#include "RHIDefines.h"
#include <array>
#include "Runtime/Core/PixelFormat.h"
enum class EShaderType
{
	SV_Vertex = 0,
	SV_Pixel,
	SV_Compute,
	SV_ShaderCount
};

extern void RHIInit();
struct FPixelFormatInfo
{
	const wchar_t* Name;
	uint32 PlatformFormat;
};
extern FPixelFormatInfo GPixelFormats[(int)EPixelFormat::FT_MAX];

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

class XBlendStateInitializerRHI
{
public:
	struct XRenderTarget
	{
		bool RTBlendeEnable;
		EBlendOperation	RTColorBlendOp = EBlendOperation::BO_Add;
		EBlendFactor    RTColorSrcBlend = EBlendFactor::BF_One;
		EBlendFactor    RTColorDestBlend = EBlendFactor::BF_Zero;
		EBlendOperation RTAlphaBlendOp = EBlendOperation::BO_Add;
		EBlendFactor    RTAlphaSrcBlend = EBlendFactor::BF_One;
		EBlendFactor    RTAlphaDestBlend = EBlendFactor::BF_Zero;
		XRenderTarget(
			bool InRTBlendeEnable = false,
			EBlendOperation	InRTColorBlendOp = EBlendOperation::BO_Add,
			EBlendFactor    InRTColorSrcBlend = EBlendFactor::BF_One,
			EBlendFactor    InRTColorDestBlend = EBlendFactor::BF_Zero,
			EBlendOperation InRTAlphaBlendOp = EBlendOperation::BO_Add,
			EBlendFactor    InRTAlphaSrcBlend = EBlendFactor::BF_One,
			EBlendFactor    InRTAlphaDestBlend = EBlendFactor::BF_Zero) :
			RTBlendeEnable(InRTBlendeEnable),
			RTColorBlendOp(InRTColorBlendOp),
			RTColorSrcBlend(InRTColorSrcBlend),
			RTColorDestBlend(InRTColorDestBlend),
			RTAlphaBlendOp(InRTAlphaBlendOp),
			RTAlphaSrcBlend(InRTAlphaSrcBlend),
			RTAlphaDestBlend(InRTAlphaDestBlend) {}
	};
	std::array<XRenderTarget, 8>RenderTargets;
	XBlendStateInitializerRHI(std::array<XRenderTarget, 8>& InRenderTargets):RenderTargets(InRenderTargets) {}
};


struct XVertexElement
{
	uint32 SemanticIndex;
	EVertexElementType Format;
	uint32 InputSlot;
	uint32 AlignedByteOffset;
	XVertexElement()
		: SemanticIndex(0),
		Format(EVertexElementType::VET_None),
		InputSlot(0),
		AlignedByteOffset(0) {}

	XVertexElement(
		uint32 InSemanticIndex,
		EVertexElementType InFormat,
		uint32 InInputSlot,
		uint32 InAlignedByteOffset)
		:SemanticIndex(InSemanticIndex),
		Format(InFormat),
		InputSlot(InInputSlot),
		AlignedByteOffset(InAlignedByteOffset) {}
};