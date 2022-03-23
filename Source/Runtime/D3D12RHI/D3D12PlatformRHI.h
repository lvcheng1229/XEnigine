#pragma once

#include "Runtime/RHI/PlatformRHI.h"

#include "D3D12View.h"
#include "D3D12CommandList.h"
class XD3D12PlatformRHI :public XPlatformRHI
{
public:
	inline void Init() override {};
	std::shared_ptr<XRHIDepthStencilState> RHICreateDepthStencilState(const XDepthStencilStateInitializerRHI& Initializer)final override;
	std::shared_ptr<XRHIBlendState> RHICreateBlendState(const XBlendStateInitializerRHI& Initializer)final override;
	std::shared_ptr<XRHIVertexShader> RHICreateVertexShader(XArrayView<uint8> Code)final override;
	std::shared_ptr<XRHIPixelShader> RHICreatePixelShader(XArrayView<uint8> Code)final override;
	void RHIBeginRenderPass(const XRHIRenderPassInfo& InInfo, const wchar_t* InName)override
	{

	}
public:
	static inline void Base_TransitionResource(
		XD3D12DirectCommandList& direct_cmd_list,
		XD3D12Resource* pResource,
		D3D12_RESOURCE_STATES after)
	{
		ResourceState& resource_state = pResource->GetResourceState();
		D3D12_RESOURCE_STATES before = resource_state.GetResourceState();
		direct_cmd_list.CmdListAddTransition(pResource, before, after);
		resource_state.SetResourceState(after);

	}
	static inline void TransitionResource(
		XD3D12DirectCommandList& hCommandList, 
		XD3D12UnorderedAcessView* pView, 
		D3D12_RESOURCE_STATES after)
	{
		XD3D12Resource* pResource = pView->GetResource();
		Base_TransitionResource(hCommandList, pResource, after);
	}
	static inline void TransitionResource(
		XD3D12DirectCommandList& direct_cmd_list,
		XD3D12ShaderResourceView* sr_view,
		D3D12_RESOURCE_STATES after)
	{
		//TODO Some Resource Dont Need Resource State Tracking
		XD3D12Resource* pResource = sr_view->GetResource();
		Base_TransitionResource(direct_cmd_list, pResource, after);
	}

	static inline void TransitionResource(
		XD3D12DirectCommandList& direct_cmd_list,
		XD3D12RenderTargetView* rt_view,
		D3D12_RESOURCE_STATES after)
	{
		XD3D12Resource* pResource = rt_view->GetResource();
		Base_TransitionResource(direct_cmd_list, pResource, after);
	}

	static inline void TransitionResource(
		XD3D12DirectCommandList& direct_cmd_list,
		XD3D12DepthStencilView* ds_view,
		D3D12_RESOURCE_STATES after)
	{
		XD3D12Resource* pResource = ds_view->GetResource();
		Base_TransitionResource(direct_cmd_list, pResource, after);
	}
};

