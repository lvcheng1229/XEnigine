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

class XRHITexture
{

};

class XRHITexture2D :public XRHITexture
{

};

class XRHIRenderTargetView
{

};

class XRHIDepthStencilView
{

};

class XRHIShaderResourceView
{

};

class XRHIConstantBuffer
{
public:
	virtual void UpdateData(void* data, uint32 size, uint32 offset_byte) = 0;
};