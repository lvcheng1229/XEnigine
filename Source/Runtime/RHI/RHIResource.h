#pragma once

#include <vector>
#include <functional>
#include "RHI.h"
#include "Runtime/HAL/PlatformTypes.h"
#include "Runtime/Core/PixelFormat.h"

enum class IndirectArgType
{
	Arg_CBV,
	Arg_VBV,
	Arg_IBV,
	Arg_Draw_Indexed,
};

struct XRHIIndirectArg
{
	IndirectArgType type;
	union
	{
		struct
		{
			uint32 Slot;
		}VertexBuffer;

		struct
		{
			uint32 RootParameterIndex;
		}CBV;
	};
};

class XRHICommandSignature
{
public:
};

class XRHIViewport
{
public:
};


class XRHIBuffer
{
public:
	XRHIBuffer(uint32 StrideIn, uint32 SizeIn) :
		Size(SizeIn),
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
	inline uint32 GetStride()const { return Stride; }
	inline uint32 GetSize()const { return Size; }
private:
	uint32 Size;// Used For Counter Offset
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

struct XRHICommandData
{
	std::vector<XRHIConstantBuffer*>CBVs;
	XRHIBuffer* VB;
	XRHIBuffer* IB;

	uint32 IndexCountPerInstance;
	uint32 InstanceCount;
	uint32 StartIndexLocation;
	int32  BaseVertexLocation;
	uint32 StartInstanceLocation;
};


class XRHISamplerState {};
class XRHIBlendState {};
class XRHIDepthStencilState {};
class XRHIRasterizationState {};

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
	XRHIRasterizationState* RasterState;
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
		THashCombine(seed, RasterState);
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
		ERenderTargetStoreAction StoreAction;
	};
	XColorTarget RenderTargets[8];

	struct EDepthStencilTarget
	{
		XRHITexture* DepthStencilTarget;
		EDepthStencilLoadAction LoadAction;
		ERenderTargetStoreAction StoreAction;
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
		EDepthStencilLoadAction DSLoadAction,

		ERenderTargetStoreAction ColorStoreAction = ERenderTargetStoreAction::EStore,//TempHack
		ERenderTargetStoreAction DSStoreAction = ERenderTargetStoreAction::EStore
	)
	{
		for (int i = 0; i < NumColorRTs; i++)
		{
			RenderTargets[i].RenderTarget = ColorRTs[i];
			RenderTargets[i].LoadAction = ColorLoadAction;
			RenderTargets[i].StoreAction = ColorStoreAction;
		}
		memset(&RenderTargets[NumColorRTs], 0, sizeof(XColorTarget) * (8 - NumColorRTs));

		if (DepthRT != nullptr)
		{
			DepthStencilRenderTarget.DepthStencilTarget = DepthRT;
			DepthStencilRenderTarget.LoadAction = DSLoadAction;
			DepthStencilRenderTarget.StoreAction = DSStoreAction;
		}
		else
		{
			DepthStencilRenderTarget.DepthStencilTarget = nullptr;
			DepthStencilRenderTarget.LoadAction = EDepthStencilLoadAction::ENoAction;
			DepthStencilRenderTarget.StoreAction = ERenderTargetStoreAction::ENoAction;
		}
	}
};

enum class ERayTracingAccelerationStructureFlags : uint32
{
	None = 0,
	AllowUpdate = 1 << 0,
	AllowCompaction = 1 << 1,
	PreferTrace = 1 << 2,
	PreferBuild = 1 << 3,
};

enum class EAccelerationStructureBuildMode
{
	Build,
	Update
};

struct XRayTracingGeometrySegment
{
public:
	std::shared_ptr<XRHIBuffer> VertexBuffer = nullptr;
	EVertexElementType VertexElementType = EVertexElementType::VET_Float3;

	// Offset in bytes from the base address of the vertex buffer.
	uint32 VertexBufferOffset = 0;
	uint32 MaxVertices = 0;
	uint32 FirstPrimitive = 0;
	uint32 NumPrimitives = 0;

	// Number of bytes between elements of the vertex buffer (sizeof VET_Float3 by default).
	// Must be equal or greater than the size of the position vector.
	uint32 VertexBufferStride = 12;

	// Indicates whether any-hit shader could be invoked when hitting this geometry segment.
	// Setting this to `false` turns off any-hit shaders, making the section "opaque" and improving ray tracing performance.
	bool bForceOpaque = true;
	bool bEnabled = true;
};

struct XRayTracingGeometryInitializer
{
public:
	std::shared_ptr<XRHIBuffer>IndexBuffer;
	uint32 IndexBufferOffset = 0;
	std::vector<XRayTracingGeometrySegment>Segments;

	bool bPreferBuild = false;
	bool bAllowUpdate = false;
	bool bAllowCompaction = true;
};

struct XRayTracingAccelerationStructSize
{
	uint64 ResultSize = 0;
	uint64 BuildScratchSize = 0;
	uint64 UpdateScratchSize = 0;
};

class XRHIRayTracingAccelerationStruct
{
public:
	friend class IRHIContext;

	XRayTracingAccelerationStructSize GetSizeInfo()const
	{
		return SizeInfo;
	}
protected:
	XRayTracingAccelerationStructSize SizeInfo = {};
};

class XRHIRayTracingGeometry : public XRHIRayTracingAccelerationStruct
{
public:
	XRHIRayTracingGeometry(const XRayTracingGeometryInitializer& InInitializer)
		: Initializer(InInitializer)
	{}

	XRayTracingGeometryInitializer Initializer;
};