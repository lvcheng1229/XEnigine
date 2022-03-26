#pragma once
#include "RenderResource.h"
#include "Runtime/RHI/RHIResource.h"
#include "Runtime/RHI/PlatformRHI.h"
#include "Runtime/Core/XMath.h"
#include "GlobalShader.h"

class RFullScreenQuadVertexLayout : public XRenderResource
{
public:
	std::shared_ptr<XRHIVertexLayout> RHIVertexLayout;
	virtual void InitRHI()override
	{
		XRHIVertexLayoutArray LayoutArray;
		LayoutArray.push_back(XVertexElement(0, EVertexElementType::VET_Float2, 0, 0));
		LayoutArray.push_back(XVertexElement(1, EVertexElementType::VET_Float2, 0, 0 + sizeof(DirectX::XMFLOAT2)));
		RHIVertexLayout = RHICreateVertexDeclaration(LayoutArray);
	}

	virtual void ReleaseRHI()override
	{
		RHIVertexLayout.reset();
	}
};

class RFullScreenQuadVS :public XGloablShader
{
public:
	static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
	{
		return new RFullScreenQuadVS(Initializer);
	}
	static ShaderInfos StaticShaderInfos;

	static void ModifyShaderCompileDefines(XShaderDefines& OutDefines) {}
public:
	RFullScreenQuadVS(const XShaderInitlizer& Initializer)
		:XGloablShader(Initializer)
	{

	}
};

extern TGlobalResource<RFullScreenQuadVertexLayout> GFullScreenLayout;
