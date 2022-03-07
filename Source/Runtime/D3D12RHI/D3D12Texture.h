#pragma once
#include "D3D12PhysicDevice.h"
#include "D3D12Allocation.h"

#include "D3D12View.h"
#include "Runtime/RHI/RHIDefines.h"
class XD3D12TextureBase
{
public:
	inline void SetShaderResourceView(XD3D12ShaderResourceView ShaderResourceViewIn) { ShaderResourceView = ShaderResourceViewIn; }
	inline void SetRenderTargetView(XD3D12RenderTargetView RenderTargetViewIn) { RenderTargetView = RenderTargetViewIn; }

	inline XD3D12ShaderResourceView* GetShaderResourceView() { return &ShaderResourceView; };
	inline XD3D12RenderTargetView* GetRenderTargetView() { return &RenderTargetView; };
	
	//XD3D12ResourceLocation ResourceLocation;
private:
	//std::shared_ptr<XD3D12ShaderResourceView>ShaderResourceView;
	//std::shared_ptr<XD3D12RenderTargetView>RenderTargetView;
	XD3D12ShaderResourceView ShaderResourceView;
	XD3D12RenderTargetView RenderTargetView;
};

class XD3D12Texture2D :public XRHITexture2D, public XD3D12TextureBase
{
};


