#pragma once
#include "Runtime/RHI/RHIContext.h"
#include "D3D12CommandList.h"
#include "D3D12PassStateManager.h"

//#include "D3D12AbstractDevice.h"

class XD3D12Context :public IRHIContext
{

};


class XD3D12AbstractDevice;
class XD3DDirectContex :public XD3D12Context
{
public:
	XD3DDirectContex(){};
	void Create(XD3D12AbstractDevice* device_in);
	
	void OpenCmdList()override;
	void CloseCmdList()override;
	
	std::shared_ptr<XRHITexture2D> CreateD3D12Texture2D(
		uint32 width, uint32 height, uint32 SizeZ,
		bool bTextureArray, bool bCubeTexture, DXGI_FORMAT format,
		ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data);

	std::shared_ptr<XRHITexture3D> CreateD3D12Texture3D(
		uint32 width, uint32 height, uint32 SizeZ, DXGI_FORMAT format,
		ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data);

	void RHISetShaderUAV(XRHIComputeShader* ShaderRHI, uint32 TextureIndex, XRHIUnorderedAcessView* UAV)override;
	void RHISetShaderTexture(XRHIComputeShader* ShaderRHI, uint32 TextureIndex, XRHITexture* NewTextureRHI);
	void RHISetShaderConstantBuffer(XRHIComputeShader* ShaderRHI, uint32 BufferIndex, XRHIConstantBuffer* RHIConstantBuffer);
	void RHISetShaderResourceViewParameter(XRHIComputeShader* ComputeShaderRHI, uint32 TextureIndex, XRHIShaderResourceView* SRVRHI);

	void RHISetRenderTargets(uint32 num_rt, XRHIRenderTargetView** rt_array_ptr, XRHIDepthStencilView* ds_ptr);
	
	void RHISetShaderTexture(XRHIGraphicsShader* ShaderRHI, uint32 TextureIndex, XRHITexture* NewTextureRHI)override;
	void RHISetShaderConstantBuffer(XRHIGraphicsShader* ShaderRHI, uint32 BufferIndex, XRHIConstantBuffer* RHIConstantBuffer);
	void RHISetViewport(float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ)override;
	void RHIClearMRT(bool ClearRT, bool ClearDS, float* ColorArray, float DepthValue, uint8 StencilValue);
	void RHIDrawFullScreenQuad();
	//void RHISetScissorRect(uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY)override;

	//TODO
	//void RHIUpdateConstantBufferData(std::shared_ptr<XD3D12ConstantBuffer>CB, void* Data, uint32 size);
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

