#pragma once

#include "Runtime/RHI/PlatformRHI.h"

#include "D3D12View.h"
#include "D3D12CommandList.h"

class XD3D12AbstractDevice;
class XD3D12PlatformRHI :public XPlatformRHI
{
public:
	XD3D12PlatformRHI(XD3D12AbstractDevice* InPhyDevice);
	inline void Init() override {};

	//CreateVertexLayout
	std::shared_ptr<XRHIVertexLayout> RHICreateVertexDeclaration(const XRHIVertexLayoutArray& Elements) final override;
	
	//Create buffer
	std::shared_ptr<XRHIVertexBuffer>RHIcreateVertexBuffer(uint32 Stride, uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData)final override;
	std::shared_ptr<XRHIIndexBuffer>RHICreateIndexBuffer(uint32 Stride, uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData)final override;

	//Create State
	std::shared_ptr<XRHIDepthStencilState> RHICreateDepthStencilState(const XDepthStencilStateInitializerRHI& Initializer)final override;
	std::shared_ptr<XRHIBlendState> RHICreateBlendState(const XBlendStateInitializerRHI& Initializer)final override;
	
	//Create Shader
	std::shared_ptr<XRHIVertexShader> RHICreateVertexShader(XArrayView<uint8> Code)final override;
	std::shared_ptr<XRHIPixelShader> RHICreatePixelShader(XArrayView<uint8> Code)final override;
	std::shared_ptr<XRHIComputeShader> RHICreateComputeShader(XArrayView<uint8> Code)final override;
	
	//CreatePSO
	std::shared_ptr<XRHIGraphicsPSO> RHICreateGraphicsPipelineState(const XGraphicsPSOInitializer& PSOInit)final override;
	std::shared_ptr<XRHIComputePSO> RHICreateComputePipelineState(const XRHIComputeShader* RHIComputeShader)final override;

	std::shared_ptr<XRHITexture2D> RHICreateTexture2D(uint32 width, uint32 height, uint32 SizeZ, bool bTextureArray,
		bool bCubeTexture, EPixelFormat Format, ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data) override;

	std::shared_ptr<XRHITexture3D> RHICreateTexture3D(uint32 width, uint32 height, uint32 SizeZ, EPixelFormat Format,
		ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data)override;
	
public:
	XD3D12PhysicDevice* PhyDevice;
	XD3D12AbstractDevice* AbsDevice;
	
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

