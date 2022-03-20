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


class XRHIBlendState {};;
class XRHIDepthStencilState {};;

class XRHIVertexLayout {};
class XRHIVertexShader: public XRHIGraphicsShader {};
class XRHIPixelShader : public XRHIGraphicsShader {};

struct XRHIBoundShaderStateInput
{
	XRHIVertexLayout* RHIVertexLayout;
	XRHIVertexShader* RHIVertexShader;
	XRHIPixelShader* RHIPixelShader;
};

class XGraphicsPSOInit
{
	XRHIBoundShaderStateInput BoundShaderState;
};