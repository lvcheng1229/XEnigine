#pragma once
#include "D3D12PhysicDevice.h"
#include "D3D12Allocation.h"

#include "D3D12View.h"
#include "Runtime/RHI/RHIDefines.h"
#include <vector>
class XD3D12TextureBase
{
public:
	inline void SetShaderResourceView(XD3D12ShaderResourceView ShaderResourceViewIn) 
	{ 
		ShaderResourceViews.push_back(ShaderResourceViewIn);
	}
	inline void SetRenderTargetView(XD3D12RenderTargetView RenderTargetViewIn) { RenderTargetView = RenderTargetViewIn; }
	inline void SetDepthStencilView(XD3D12DepthStencilView DepthStencilViewIn) { DepthStencilView = DepthStencilViewIn; }
	
	inline void SetUnorderedAcessView(XD3D12UnorderedAcessView UnorderedAcessViewIn) 
	{ 
		UnorderedAcessViews.push_back(UnorderedAcessViewIn);
	}

	inline XD3D12ShaderResourceView* GetShaderResourceView(uint32 MipIndex = 0) { return &ShaderResourceViews[MipIndex]; };
	inline XD3D12RenderTargetView* GetRenderTargetView() { return &RenderTargetView; };
	inline XD3D12DepthStencilView* GeDepthStencilView() { return &DepthStencilView; };
	
	inline XD3D12UnorderedAcessView* GeUnorderedAcessView(uint32 MipIndex = 0)
	{ 
		return &UnorderedAcessViews[MipIndex];
	};
	
	//XD3D12ResourceLocation ResourceLocation;
private:

	std::vector<XD3D12UnorderedAcessView>UnorderedAcessViews;
	std::vector<XD3D12ShaderResourceView>ShaderResourceViews;

	XD3D12RenderTargetView RenderTargetView;
	XD3D12DepthStencilView DepthStencilView;
	 
};

class XD3D12Texture2D :public XRHITexture2D, public XD3D12TextureBase
{
public:
	virtual void* GetTextureBaseRHI() final override
	{
		return static_cast<XD3D12TextureBase*>(this);
	}
};

class XD3D12Texture3D :public XRHITexture3D, public XD3D12TextureBase
{
public:
	virtual void* GetTextureBaseRHI() final override
	{
		return static_cast<XD3D12TextureBase*>(this);
	}
};

/** Given a pointer to a RHI texture that was created by the D3D12 RHI, returns a pointer to the FD3D12TextureBase it encapsulates. */
inline XD3D12TextureBase* GetD3D12TextureFromRHITexture(XRHITexture* Texture)
{
	if (!Texture)	{	return NULL;	}
	XD3D12TextureBase* Result = ((XD3D12TextureBase*)Texture->GetTextureBaseRHI());
	X_Assert(Result);
	return Result;
}

