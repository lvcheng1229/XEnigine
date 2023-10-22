#pragma once

#if !USE_DX12

#include "Runtime\RenderCore\GlobalShader.h"
#include "Runtime\RHI\RHIStaticStates.h"
#include "Runtime\RHI\PipelineStateCache.h"

class XBuildInRGS :public XGloablShader
{
public:
    static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
    {
        return new XBuildInRGS(Initializer);
    }

    static ShaderInfos StaticShaderInfos;
    static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}

    XBuildInRGS(const XShaderInitlizer& Initializer)
        :XGloablShader(Initializer)
    {

    }

    void SetParameter(
        XRHICommandList& RHICommandList,
        XRHITexture* InTexture)
    {
    }
};

class XBuildInCHS :public XGloablShader
{
public:
    static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
    {
        return new XBuildInCHS(Initializer);
    }

    static ShaderInfos StaticShaderInfos;
    static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}

    XBuildInCHS(const XShaderInitlizer& Initializer)
        :XGloablShader(Initializer)
    {

    }

    void SetParameter(
        XRHICommandList& RHICommandList,
        XRHITexture* InTexture)
    {
    }
};

class XBuildInMSS :public XGloablShader
{
public:
    static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
    {
        return new XBuildInMSS(Initializer);
    }

    static ShaderInfos StaticShaderInfos;
    static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}

    XBuildInMSS(const XShaderInitlizer& Initializer)
        :XGloablShader(Initializer)
    {

    }

    void SetParameter(
        XRHICommandList& RHICommandList,
        XRHITexture* InTexture)
    {
    }
};

void DispatchRayTest(XRHICommandList& RHICmdList);

#endif