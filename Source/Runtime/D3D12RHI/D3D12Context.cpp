#include "D3D12Context.h"
#include "D3D12Texture.h"
#include "D3D12AbstractDevice.h"
#include "D3D12PlatformRHI.h"
void XD3DDirectContex::Create(XD3D12AbstractDevice* device_in)
{
	AbsDevice = device_in;
	XD3D12PhysicDevice* PhyDevice = AbsDevice->GetPhysicalDevice();
	cmd_direct_alloc.Create(PhyDevice, D3D12_COMMAND_LIST_TYPE_DIRECT);
	cmd_direct_alloc.Reset();

	cmd_dirrect_list.CreateDirectCmdList(PhyDevice, &cmd_direct_alloc);
	cmd_dirrect_list.Close();

	PassStateManager.Create(PhyDevice, this);
	//cmd_dirrect_list.Reset(&cmd_direct_alloc);
	//direct_cmd_allc_manager = new XD3D12CommandAllocManger(D3D12_COMMAND_LIST_TYPE_DIRECT);
	//direct_cmd_allc_manager->Create(device_in);
}

void XD3DDirectContex::OpenCmdList()
{
	cmd_dirrect_list.Reset(&cmd_direct_alloc);
}

void XD3DDirectContex::CloseCmdList()
{
	cmd_dirrect_list.Close();
}

std::shared_ptr<XRHITexture2D> XD3DDirectContex::CreateD3D12Texture2D(
	uint32 width, uint32 height, uint32 SizeZ, bool bTextureArray, bool bCubeTexture, DXGI_FORMAT format,
	ETextureCreateFlags flag, uint32 NumMipsIn,uint8* tex_data)
{
	return std::shared_ptr<XRHITexture2D>(
		AbsDevice->CreateD3D12Texture2D(&cmd_dirrect_list, width, height, SizeZ, bTextureArray, bCubeTexture, format, flag, NumMipsIn, tex_data));
}

std::shared_ptr<XRHITexture3D> XD3DDirectContex::CreateD3D12Texture3D(uint32 width, uint32 height, uint32 SizeZ, DXGI_FORMAT format, ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data)
{
	return std::shared_ptr<XRHITexture3D>(AbsDevice->CreateD3D12Texture3D(&cmd_dirrect_list, width, height, SizeZ, format, flag, NumMipsIn, tex_data));
}

void XD3DDirectContex::RHISetRenderTargets(uint32 num_rt, XRHIRenderTargetView** rt_array_ptr, XRHIDepthStencilView* ds_ptr)
{
	DSVPtr = static_cast<XD3D12DepthStencilView*>(ds_ptr);
	CurrentNumRT = num_rt;

	for (uint32 i = 0; i < num_rt; i++)
	{
		RTPtrArrayPtr[i] = static_cast<XD3D12RenderTargetView*>(rt_array_ptr[i]);
	}

	PassStateManager.SetRenderTarget(num_rt, RTPtrArrayPtr, DSVPtr);
}

//void XD3DDirectContex::RHISetShaderUAV(XRHIComputeShader* ShaderRHI, uint32 TextureIndex, XRHITexture* NewTextureRHI)
void XD3DDirectContex::RHISetShaderUAV(XRHIComputeShader* ShaderRHI, uint32 TextureIndex, XRHIUnorderedAcessView* UAV)
{
	XD3D12UnorderedAcessView* D3DUAVPtr = static_cast<XD3D12UnorderedAcessView*>(UAV);
	PassStateManager.SetUAV<EShaderType::SV_Compute>(D3DUAVPtr, TextureIndex);
}

void XD3DDirectContex::RHISetShaderTexture(XRHIComputeShader* ShaderRHI, uint32 TextureIndex, XRHITexture* NewTextureRHI)
{
	XD3D12TextureBase* D3DTexturePtr = GetD3D12TextureFromRHITexture(NewTextureRHI);
	XD3D12ShaderResourceView* D3DSRVPtr = D3DTexturePtr->GetShaderResourceView();
	PassStateManager.SetShaderResourceView<EShaderType::SV_Compute>(D3DSRVPtr, TextureIndex);
}

void XD3DDirectContex::RHISetShaderConstantBuffer(XRHIComputeShader* ShaderRHI, uint32 BufferIndex, XRHIConstantBuffer* RHIConstantBuffer)
{
	XD3D12ConstantBuffer* ConstantBuffer = static_cast<XD3D12ConstantBuffer*>(RHIConstantBuffer);
	PassStateManager.SetCBV<EShaderType::SV_Compute>(ConstantBuffer, BufferIndex);
}

void XD3DDirectContex::RHISetShaderResourceViewParameter(XRHIComputeShader* ComputeShaderRHI, uint32 TextureIndex, XRHIShaderResourceView* SRVRHI)
{
	XD3D12ShaderResourceView* D3DSRVPtr = static_cast<XD3D12ShaderResourceView*>(SRVRHI);
	PassStateManager.SetShaderResourceView<EShaderType::SV_Compute>(D3DSRVPtr, TextureIndex);
}

void XD3DDirectContex::RHISetShaderTexture(XRHIGraphicsShader* ShaderRHI, uint32 TextureIndex, XRHITexture* NewTextureRHI)
{
	EShaderType ShaderType = ShaderRHI->GetShaderType();
	XD3D12TextureBase* D3DTexturePtr = GetD3D12TextureFromRHITexture(NewTextureRHI);
	XD3D12ShaderResourceView* D3DSRVPtr = D3DTexturePtr->GetShaderResourceView();
	switch (ShaderType)
	{
	case EShaderType::SV_Vertex:
		PassStateManager.SetShaderResourceView<EShaderType::SV_Vertex>(D3DSRVPtr, TextureIndex);
		break;
	case EShaderType::SV_Pixel:
		PassStateManager.SetShaderResourceView<EShaderType::SV_Pixel>(D3DSRVPtr, TextureIndex);
		break;
	case EShaderType::SV_Compute:
		PassStateManager.SetShaderResourceView<EShaderType::SV_Compute>(D3DSRVPtr, TextureIndex);
		break;
	default:
		X_Assert(false);
		break;
	}
}

void XD3DDirectContex::RHISetShaderConstantBuffer(XRHIGraphicsShader* ShaderRHI, uint32 BufferIndex, XRHIConstantBuffer* RHIConstantBuffer)
{
	EShaderType ShaderType = ShaderRHI->GetShaderType();
	XD3D12ConstantBuffer* ConstantBuffer = static_cast<XD3D12ConstantBuffer*>(RHIConstantBuffer);
	switch (ShaderType)
	{
	case EShaderType::SV_Vertex:
		PassStateManager.SetCBV<EShaderType::SV_Vertex>(ConstantBuffer, BufferIndex);
		break;
	case EShaderType::SV_Pixel:
		PassStateManager.SetCBV<EShaderType::SV_Pixel>(ConstantBuffer, BufferIndex);
		break;
	case EShaderType::SV_Compute:
		PassStateManager.SetCBV<EShaderType::SV_Compute>(ConstantBuffer, BufferIndex);
		break;
	default:
		X_Assert(false);
		break;
	}
}

void XD3DDirectContex::RHISetViewport(float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ)
{
	Viewport = { MinX, MinY, (MaxX - MinX), (MaxY - MinY), MinZ, MaxZ };
	ScissorRect = { static_cast<long>(MinX), static_cast<long>(MinY), static_cast<long>(MaxX), static_cast<long>(MaxY) };
	cmd_dirrect_list->RSSetViewports(1, &Viewport);
	cmd_dirrect_list->RSSetScissorRects(1, &ScissorRect);
}

void XD3DDirectContex::RHIClearMRT(bool ClearRT, bool ClearDS, float* ColorArray, float DepthValue, uint8 StencilValue)
{
	uint32 numRT = CurrentNumRT;

	if (ClearRT)
	{
		for (uint32 i = 0; i < numRT; i++) {
			XD3D12PlatformRHI::TransitionResource(
				cmd_dirrect_list,
				RTPtrArrayPtr[i],
				D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
	}
	
	if (ClearDS)
	{
		XD3D12PlatformRHI::TransitionResource(
			cmd_dirrect_list,
			DSVPtr,
			D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	cmd_dirrect_list.CmdListFlushBarrier();

	if (ClearRT)
	{
		for (uint32 i = 0; i < numRT; i++)
		{
			cmd_dirrect_list->ClearRenderTargetView(
				RTPtrArrayPtr[i]->GetCPUPtr(),
				ColorArray, 0, nullptr);
		}
	}
	
	if (ClearDS)
	{
		cmd_dirrect_list->ClearDepthStencilView(
			DSVPtr->GetCPUPtr(),
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			DepthValue,
			StencilValue,
			0,
			nullptr);
	}
}

void XD3DDirectContex::RHIDrawFullScreenQuad()
{
	///mCommandList.Get()->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
	///mCommandList.Get()->IASetIndexBuffer(&ri->Geo->IndexBufferView());
	///cmd_dirrect_list->IASetVertexBuffers(0, 1, );
	///cmd_dirrect_list->DrawInstanced(3, 1, 0, 0);
}


void XD3DDirectContex::ResetCmdAlloc()
{
	cmd_direct_alloc.Reset();
}
