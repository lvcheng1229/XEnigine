#pragma once
#include <array>
#include "RHIDefines.h"
#include "Runtime/DataStruct/XRefCountPtr.h"
#include "Runtime/Core/Template/XEngineTemplate.h"
#include "Runtime/Core/PixelFormat.h"
#include "Runtime/Core/ResourceCreateDataInterface.h"

struct FPixelFormatInfo
{
	const wchar_t* Name;
	uint32 PlatformFormat;
};

class XPlatformRHI;
class XRHIModule
{
public:
	virtual void ReleaseRHI() = 0;
	virtual XPlatformRHI* CreateRHI() = 0;
};

extern void RHIRelease();
extern void RHIInit(uint32 Width, uint32 Height);
extern FPixelFormatInfo GPixelFormats[(int)EPixelFormat::FT_MAX];
using EShaderType_Underlying = std::underlying_type<EShaderType>::type;

struct XRHIResourceCreateData
{
	XRHIResourceCreateData()
		:ResourceArray(nullptr) {}

	XRHIResourceCreateData(FResourceArrayInterface* ResourceArrayIn) :ResourceArray(ResourceArrayIn) {}
	
	FResourceArrayInterface* ResourceArray;
};

struct XRasterizationStateInitializerRHI
{
	bool bConservative; 
	EFaceCullMode CullMode;
	XRasterizationStateInitializerRHI(
		bool bConservativeIn,
		EFaceCullMode CullModeIn) :
		bConservative(bConservativeIn),
		CullMode(CullModeIn) {}
};

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