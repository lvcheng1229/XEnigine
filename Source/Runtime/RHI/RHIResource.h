#pragma once
#include "Runtime/HAL/PlatformTypes.h"
#include "RHI.h"

class XRHIShader
{
public:
	explicit XRHIShader(EShaderType ShaderTypeIn) :ShaderType(ShaderTypeIn) {}
	inline EShaderType GetShaderType() { return ShaderType; }
private:
	EShaderType ShaderType;
};

class XRHIGraphicsShader :public XRHIShader
{
public:
	explicit XRHIGraphicsShader(EShaderType ShaderTypeIn) :XRHIShader(ShaderTypeIn) {}
};

class XRHIComputeShader :public XRHIShader
{
public:
	explicit XRHIComputeShader(EShaderType ShaderTypeIn) :XRHIShader(ShaderTypeIn) {}
};

class XRHITexture
{
public:
	virtual void* GetTextureBaseRHI()
	{
		return nullptr;// Override this in derived classes to expose access to the native texture resource
	}
};

class XRHITexture2D :public XRHITexture {};
class XRHITexture3D :public XRHITexture {};


class XRHIRenderTargetView {};
class XRHIDepthStencilView {};
class XRHIShaderResourceView {};
class XRHIUnorderedAcessView {};

class XRHIConstantBuffer
{
public:
	virtual void UpdateData(const void* data, uint32 size, uint32 offset_byte) = 0;
};


class XRHIBlendState {};
class XRHIDepthStencilState {};

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
};

class XGraphicsPSOInitializer
{
public:
	XRHIBoundShaderStateInput BoundShaderState;
	XRHIBlendState* BlendState;
	XRHIDepthStencilState* DepthStencilState;
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

		DepthStencilRenderTarget.DepthStencilTarget = DepthRT;
		DepthStencilRenderTarget.LoadAction = DSLoadAction;
	}
};