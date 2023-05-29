#pragma once
#include <vulkan\vulkan_core.h>
#include <Runtime\HAL\Mch.h>
#include "Runtime/RHI/RHIContext.h"
#include "VulkanBarriers.h"

class XVulkanCommandBufferManager;
class XVulkanPlatformRHI;
class XVulkanDevice;
class XVulkanQueue;
class XVulkanCommandListContext :public IRHIContext
{
public:
	XVulkanCommandListContext(XVulkanPlatformRHI* InRHI, XVulkanDevice* InDevice, XVulkanQueue* InQueue);
	~XVulkanCommandListContext();

	void ReseizeViewport(uint32 Width, uint32 Height)override { XASSERT(false)};

	void Execute()override { XASSERT(false) };
	void OpenCmdList()override { XASSERT(false) };
	void CloseCmdList()override { XASSERT(false) };

	//SetShaderParameter
	void RHISetShaderUAV(EShaderType ShaderType, uint32 TextureIndex, XRHIUnorderedAcessView* UAV)override { XASSERT(false) };
	void RHISetShaderTexture(EShaderType ShaderType, uint32 TextureIndex, XRHITexture* NewTextureRHI)override { XASSERT(false) };
	void RHISetShaderSRV(EShaderType ShaderType, uint32 SRVIndex, XRHIShaderResourceView* SRV)override { XASSERT(false) };
	void RHISetShaderConstantBuffer(EShaderType ShaderType, uint32 BufferIndex, XRHIConstantBuffer* RHIConstantBuffer)override { XASSERT(false) };
	void SetShaderValue(EShaderType ShaderType, uint32 BufferIndex, uint32 VariableOffsetInBuffer, uint32 NumBytes, const void* NewValue)override { XASSERT(false) };

	//SetPSO
	void RHISetGraphicsPipelineState(XRHIGraphicsPSO* GraphicsState)override;
	void RHISetComputePipelineState(XRHIComputePSO* ComputeState)override { XASSERT(false) };

	void RHIExecuteIndirect(XRHICommandSignature* RHICmdSig, uint32 CmdCount, XRHIStructBuffer* ArgumentBuffer, uint64 ArgumentBufferOffset, XRHIStructBuffer* CountBuffer, uint64 CountBufferOffset)override { XASSERT(false) };
	void RHIDrawIndexedPrimitive(XRHIIndexBuffer* IndexBuffer, uint32 IndexCountPerInstance, uint32 InstanceCount, uint32 StartIndexLocation, uint32 BaseVertexLocation, uint32 StartInstanceLocation) final override { XASSERT(false) };
	void RHIDispatchComputeShader(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ) final override { XASSERT(false) };

	//SetVB IB
	void SetVertexBuffer(XRHIBuffer* RHIVertexBuffer, uint32 VertexBufferSlot, uint32 OffsetFormVBBegin) final override { XASSERT(false) };

	//Misc
	void RHISetViewport(float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ)override { XASSERT(false) };
	void SetRenderTargetsAndViewPort(uint32 NumRTs, const XRHIRenderTargetView* RTViews, const XRHIDepthStencilView* DSView)override { XASSERT(false) };

	//DrawCall/DisPatch
	void RHIEventBegin(uint32 Metadata, const void* pData, uint32 Size)override { XASSERT(false) };
	void RHIEventEnd()override { XASSERT(false) };
	void RHIEndFrame()override;
	void RHIBeginFrame()override { XASSERT(false) };
	void RHIEndRenderPass()override { XASSERT(false) };
	void RHIBeginRenderPass(const XRHIRenderPassInfo& InInfo, const char* InName, uint32 Size)override;

	//Vulkan
	XVulkanQueue* GetQueue() { return Queue; }
	XVulkanRenderPass* PrepareRenderPassForPSOCreation(const XGraphicsPSOInitializer& Initializer);
private:
	

	friend class VkHack;

	XVulkanPlatformRHI* RHI;
	XVulkanDevice* Device;
	XVulkanQueue* Queue;
	XVulkanCommandBufferManager* CmdBufferManager;

	class XVulkanPendingGfxState* PendingGfxState;
	static XVulkanLayoutManager GlobalLayoutManager;
};