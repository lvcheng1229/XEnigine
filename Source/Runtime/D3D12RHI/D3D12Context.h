#pragma once
#include "Runtime/RHI/RHIContext.h"
#include "D3D12CommandList.h"
#include "D3D12PassStateManager.h"



class XD3D12Context :public IRHIContext
{

};

class XD3D12AbstractDevice;
class XD3DDirectContex :public XD3D12Context
{
public:
	std::shared_ptr<XD3D12GlobalConstantBuffer>VSGlobalConstantBuffer;
	std::shared_ptr<XD3D12GlobalConstantBuffer>PSGlobalConstantBuffer;
	std::shared_ptr<XD3D12GlobalConstantBuffer>CSGlobalConstantBuffer;

	XD3DDirectContex(){};
	void Create(XD3D12AbstractDevice* device_in);
	
	void OpenCmdList()override;
	void CloseCmdList()override;
	
	std::shared_ptr<XRHITexture2D> CreateD3D12Texture2D(
		uint32 width, uint32 height, uint32 SizeZ,
		bool bTextureArray, bool bCubeTexture, EPixelFormat Format,
		ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data);

	std::shared_ptr<XRHITexture3D> CreateD3D12Texture3D(
		uint32 width, uint32 height, uint32 SizeZ, EPixelFormat Format,
		ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data);

	void RHIDispatchComputeShader(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ) final override;

	void RHISetGraphicsPipelineState(XRHIGraphicsPSO* GraphicsState)override;
	void RHISetComputePipelineState(XRHIComputePSO* ComputeState)override;

	//SetShaderParameter
	void RHISetShaderUAV(EShaderType ShaderType, uint32 TextureIndex, XRHIUnorderedAcessView* UAV)override;
	void RHISetShaderTexture(EShaderType ShaderType, uint32 TextureIndex, XRHITexture* NewTextureRHI)override;
	void RHISetShaderConstantBuffer(XRHIComputeShader* ShaderRHI, uint32 BufferIndex, XRHIConstantBuffer* RHIConstantBuffer);
	void SetShaderValue(EShaderType ShaderType, uint32 BufferIndex, uint32 VariableOffsetInBuffer, uint32 NumBytes, const void* NewValue)override;
	
	//void RHISetShaderResourceViewParameter(XRHIComputeShader* ComputeShaderRHI, uint32 TextureIndex, XRHIShaderResourceView* SRVRHI);
	//void RHISetShaderTexture(XRHIComputeShader* ShaderRHI, uint32 TextureIndex, XRHITexture* NewTextureRHI);
	
	
	void RHISetRenderTargets(uint32 num_rt, XRHIRenderTargetView** rt_array_ptr, XRHIDepthStencilView* ds_ptr);
	
	//void RHISetShaderTexture(XRHIGraphicsShader* ShaderRHI, uint32 TextureIndex, XRHITexture* NewTextureRHI)override;

	void RHISetShaderConstantBuffer(XRHIGraphicsShader* ShaderRHI, uint32 BufferIndex, XRHIConstantBuffer* RHIConstantBuffer);
	void RHISetShaderConstantBuffer(EShaderType ShaderType, uint32 BufferIndex, XRHIConstantBuffer* RHIConstantBuffer)override;


	void RHISetViewport(float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ)override;


	void RHIClearMRT(bool ClearRT, bool ClearDS, float* ColorArray, float DepthValue, uint8 StencilValue);
	//void RHIDrawFullScreenQuad();
	void RHIDrawIndexedPrimitive() final override;

	void SetRenderTargetsAndViewPort(uint32 NumRTs,const XRHIRenderTargetView* RTViews, const XRHIDepthStencilView* DSView)override;
	void SetRenderTargetsAndClear(const XRHISetRenderTargetsInfo& RTInfos);
	void RHIBeginRenderPass(const XRHIRenderPassInfo& InInfo, const wchar_t* InName)override
	{
		cmd_dirrect_list->BeginEvent(1, InName,sizeof(InName));
		XRHISetRenderTargetsInfo OutRTInfo;
		InInfo.ConvertToRenderTargetsInfo(OutRTInfo);
		SetRenderTargetsAndClear(OutRTInfo);
	}

	void ResetCmdAlloc();
private:
public:
	inline XD3D12PassStateManager* GetPassStateManager() { return &PassStateManager; }
	inline XD3D12DirectCommandList* GetCmdList() { return &cmd_dirrect_list; };
	inline XD3D12CommandAllocator* GetCmdAlloc() { return &cmd_direct_alloc; };
private:
	XD3D12RenderTargetView* RTPtrArrayPtr[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
	XD3D12DepthStencilView* DSVPtr = nullptr;
	uint32 CurrentNumRT = 0;

	XD3D12AbstractDevice* AbsDevice;
	XD3D12CommandAllocator cmd_direct_alloc;
	XD3D12DirectCommandList cmd_dirrect_list;

	XD3D12PassStateManager PassStateManager;
private://TODO
	D3D12_VIEWPORT Viewport;
	D3D12_RECT ScissorRect;
};

