#pragma once

#include "Runtime/RHI/PlatformRHI.h"

#include "D3D12View.h"
#include "D3D12CommandList.h"
class XD3D12PlatformRHI :XPlatformRHI
{
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
		XD3D12DirectCommandList& direct_cmd_list,
		XD3D12RenderTargetView* rt_view,
		D3D12_RESOURCE_STATES after)
	{
		XD3D12Resource* pResource = rt_view->GetResource();
		Base_TransitionResource(direct_cmd_list, pResource, after);
	}

	static inline void TransitionResource(
		XD3D12DirectCommandList& direct_cmd_list,
		XD3D12DepthStenciltView* ds_view,
		D3D12_RESOURCE_STATES after)
	{
		XD3D12Resource* pResource = ds_view->GetResource();
		Base_TransitionResource(direct_cmd_list, pResource, after);
	}
};