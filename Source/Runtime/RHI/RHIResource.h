#pragma once

#include <vector>
#include <functional>
#include "RHI.h"
#include "Runtime/HAL/PlatformTypes.h"
#include "Runtime/Core/PixelFormat.h"

class XRHIVertexBuffer
{
public:
	XRHIVertexBuffer(uint32 StrideIn, uint32 SizeIn) :
		Size(SizeIn),
		Stride(StrideIn) {}
	inline uint32 GetStride()const { return Stride; }
	inline uint32 GetSize()const { return Size; }
private:
	uint32 Size;
	uint32 Stride;
};

class XRHIIndexBuffer
{
public:
	XRHIIndexBuffer(uint32 StrideIn, uint32 SizeIn)
		:Size(SizeIn),
		Stride(StrideIn) {}
	inline uint32 GetStride()const { return Stride; }
	inline uint32 GetSize()const { return Size; }
private:
	uint32 Size;
	uint32 Stride;
};

class XRHIStructBuffer
{
public:
	XRHIStructBuffer(uint32 StrideIn, uint32 SizeIn)
		:Size(SizeIn),
		Stride(StrideIn) {}
private:
	uint32 Size;
	uint32 Stride;
};

class XRHIShader
{
public:
	explicit XRHIShader(EShaderType ShaderTypeIn) :ShaderType(ShaderTypeIn), CodeHash(0) {}

	inline EShaderType		GetShaderType()				{ return ShaderType; }
	inline std::size_t		GetHash()const				{ return CodeHash; }
	inline void				SetHash(std::size_t Hahs)	{ CodeHash = Hahs; }

private:
	EShaderType ShaderType;
	std::size_t CodeHash;
};

class XRHIGraphicsShader :public XRHIShader
{
public:
	explicit XRHIGraphicsShader(EShaderType ShaderTypeIn) :XRHIShader(ShaderTypeIn) {}
};

class XRHIComputeShader :public XRHIShader
{
public:
	XRHIComputeShader() :XRHIShader(EShaderType::SV_Compute) {}
};

class XRHITexture
{
public:
	XRHITexture(EPixelFormat FormatIn) :Format(FormatIn) {}
	inline EPixelFormat GetFormat()const { return Format; }
	virtual void* GetTextureBaseRHI()
	{
		return nullptr;
	}
private:
	EPixelFormat Format;
};

class XRHITexture2D :public XRHITexture 
{
public:
	XRHITexture2D(EPixelFormat FormatIn) :XRHITexture(FormatIn) {}
};

class XRHITexture3D :public XRHITexture 
{
public:
	XRHITexture3D(EPixelFormat FormatIn) :XRHITexture(FormatIn) {}
};


class XRHIRenderTargetView 
{
public:
	XRHIRenderTargetView():Texture(nullptr){}
	XRHITexture* Texture;
};
class XRHIDepthStencilView 
{
public:
	XRHIDepthStencilView():Texture(nullptr){}
	XRHITexture* Texture;
};
class XRHIShaderResourceView {};
class XRHIUnorderedAcessView {};

class XRHIConstantBuffer
{
public:
	virtual void UpdateData(const void* data, uint32 size, uint32 offset_byte) = 0;
};


class XRHIBlendState {};
class XRHIDepthStencilState {};

#define VERTEX_LAYOUT_MAX 16
using XRHIVertexLayoutArray = std::vector<XVertexElement>;
class XRHIVertexLayout {};


class XRHIVertexShader: public XRHIGraphicsShader 
{
public:
	XRHIVertexShader() :XRHIGraphicsShader(EShaderType::SV_Vertex) {}
};
class XRHIPixelShader : public XRHIGraphicsShader 
{
public:
	XRHIPixelShader() :XRHIGraphicsShader(EShaderType::SV_Pixel) {}
};

struct XRHIBoundShaderStateInput
{
	XRHIVertexLayout* RHIVertexLayout;
	XRHIVertexShader* RHIVertexShader;
	XRHIPixelShader* RHIPixelShader;
	XRHIBoundShaderStateInput() :
		RHIVertexLayout(nullptr),
		RHIVertexShader(nullptr),
		RHIPixelShader(nullptr) {}

	XRHIBoundShaderStateInput(
		XRHIVertexLayout* RHIVertexLayoutIn,
		XRHIVertexShader* RHIVertexShaderIn,
		XRHIPixelShader* RHIPixelShaderIn) :
		RHIVertexLayout(RHIVertexLayoutIn),
		RHIVertexShader(RHIVertexShaderIn),
		RHIPixelShader(RHIPixelShaderIn) {}
};

class XGraphicsPSOInitializer
{
public:
	XRHIBoundShaderStateInput BoundShaderState;
	XRHIBlendState* BlendState;
	XRHIDepthStencilState* DepthStencilState;
	uint32 RTNums;
	std::array<EPixelFormat, 8>RT_Format;
	EPixelFormat DS_Format;

	inline std::size_t GetHashIndex()const
	{
		std::size_t seed = 42;
		THashCombine(seed, BoundShaderState.RHIVertexLayout);
		THashCombine(seed, BoundShaderState.RHIVertexShader);
		THashCombine(seed, BoundShaderState.RHIPixelShader);
		THashCombine(seed, BlendState);
		THashCombine(seed, DepthStencilState);
		THashCombine(seed, RTNums);
		for (int i = 0; i < 8; i++)
		{
			THashCombine(seed, (int)RT_Format[i]);
		}
		THashCombine(seed, (int)DS_Format);
		return seed;
	}
};

class XRHIGraphicsPSO {};
class XRHIComputePSO{};
class XRHISetRenderTargetsInfo
{
public:
	// Color Render Targets Info
	XRHIRenderTargetView ColorRenderTarget[8];
	int32 NumColorRenderTargets;
	bool bClearColor;

	// Depth/Stencil Render Target Info
	XRHIDepthStencilView DepthStencilRenderTarget;
	bool bClearDepth;
	bool bClearStencil;

	XRHISetRenderTargetsInfo()
		:NumColorRenderTargets(0),
		bClearColor(false),
		bClearDepth(false),
		bClearStencil(false) {}
};
struct XRHIRenderPassInfo
{
	struct XColorTarget
	{
		XRHITexture* RenderTarget;
		ERenderTargetLoadAction LoadAction;
	};
	XColorTarget RenderTargets[8];

	struct EDepthStencilTarget
	{
		XRHITexture* DepthStencilTarget;
		EDepthStencilLoadAction LoadAction;
	};
	EDepthStencilTarget DepthStencilRenderTarget;

	void ConvertToRenderTargetsInfo(XRHISetRenderTargetsInfo& OutRTInfo) const
	{
		for (int i = 0; i < 8; i++)
		{
			if (!RenderTargets[i].RenderTarget)
			{
				break;
			}
			OutRTInfo.ColorRenderTarget[i].Texture = RenderTargets[i].RenderTarget;
			OutRTInfo.bClearColor |= (RenderTargets[i].LoadAction == ERenderTargetLoadAction::EClear ? true : false);
			OutRTInfo.NumColorRenderTargets++;
		}

		OutRTInfo.DepthStencilRenderTarget.Texture = DepthStencilRenderTarget.DepthStencilTarget;
		OutRTInfo.bClearDepth = (DepthStencilRenderTarget.LoadAction == EDepthStencilLoadAction::EClear ? true : false);
	}
	explicit XRHIRenderPassInfo(
		int NumColorRTs,
		XRHITexture* ColorRTs[],
		ERenderTargetLoadAction ColorLoadAction,
		XRHITexture* DepthRT,
		EDepthStencilLoadAction DSLoadAction)
	{
		for (int i = 0; i < NumColorRTs; i++)
		{
			RenderTargets[i].RenderTarget = ColorRTs[i];
			RenderTargets[i].LoadAction = ColorLoadAction;
		}
		memset(&RenderTargets[NumColorRTs], 0, sizeof(XColorTarget) * (8 - NumColorRTs));

		if (DepthRT != nullptr)
		{
			DepthStencilRenderTarget.DepthStencilTarget = DepthRT;
			DepthStencilRenderTarget.LoadAction = DSLoadAction;
		}
		else
		{
			DepthStencilRenderTarget.DepthStencilTarget = nullptr;
			DepthStencilRenderTarget.LoadAction = EDepthStencilLoadAction::ENoAction;
		}
	}
};