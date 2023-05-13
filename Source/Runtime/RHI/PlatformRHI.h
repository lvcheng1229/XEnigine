#pragma once
#include <memory>
#include "RHIResource.h"

#define USE_DX12 1

class XRHICommandList;
class XPlatformRHI
{
public:
	virtual ~XPlatformRHI() {}
	virtual void Init() = 0;
	
	//CreateVertexLayout
	virtual std::shared_ptr<XRHIVertexLayout> RHICreateVertexDeclaration(const XRHIVertexLayoutArray& Elements) = 0;
	
	//Create Buffer
	virtual std::shared_ptr<XRHIVertexBuffer>RHICreateVertexBuffer(uint32 Stride, uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData) = 0;
	virtual std::shared_ptr<XRHIIndexBuffer>RHICreateIndexBuffer(uint32 Stride, uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData) = 0;
	virtual std::shared_ptr<XRHIStructBuffer>RHICreateStructBuffer(uint32 Stride, uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData) = 0;
	virtual std::shared_ptr<XRHIUnorderedAcessView> RHICreateUnorderedAccessView(XRHIStructBuffer* StructuredBuffer, bool bUseUAVCounter, bool bAppendBuffer, uint64 CounterOffsetInBytes) = 0;
	virtual std::shared_ptr<XRHIShaderResourceView> RHICreateShaderResourceView(XRHIStructBuffer* StructuredBuffer) = 0;
	virtual void RHIResetStructBufferCounter(XRHIStructBuffer* RHIStructBuffer, uint32 CounterOffset) = 0;
	virtual void RHICopyTextureRegion(XRHITexture* RHITextureDst, XRHITexture* RHITextureSrc, uint32 DstX, uint32 DstY, uint32 DstZ, uint32 OffsetX, uint32 OffsetY, uint32 OffsetZ) = 0;

	//Command Signature
	virtual XRHIUnorderedAcessView* GetRHIUAVFromTexture(XRHITexture* RHITexture, uint32 MipIndex = 0) = 0;
	virtual XRHIShaderResourceView* GetRHISRVFromTexture(XRHITexture* RHITexture, uint32 MipIndex = 0) = 0;
	virtual uint64 RHIGetCmdBufferOffset(XRHIStructBuffer* RHIStructBuffer) = 0;
	virtual void* RHIGetCommandDataPtr(std::vector<XRHICommandData>&RHICmdData, uint32& OutCmdDataSize) = 0;
	virtual std::shared_ptr<XRHICommandSignature> RHICreateCommandSignature(XRHIIndirectArg* RHIIndirectArg, uint32 ArgCount, XRHIVertexShader* VertexShader, XRHIPixelShader* PixelShader) = 0;

	//Create State
	virtual std::shared_ptr<XRHIRasterizationState> RHICreateRasterizationStateState(const XRasterizationStateInitializerRHI& Initializer) = 0;
	virtual std::shared_ptr<XRHIDepthStencilState> RHICreateDepthStencilState(const XDepthStencilStateInitializerRHI& Initializer) = 0;
	virtual std::shared_ptr<XRHIBlendState> RHICreateBlendState(const XBlendStateInitializerRHI& Initializer) = 0;
	
	//Create Shader
	virtual std::shared_ptr<XRHIVertexShader> RHICreateVertexShader(XArrayView<uint8> Code) = 0;
	virtual std::shared_ptr<XRHIPixelShader> RHICreatePixelShader(XArrayView<uint8> Code) = 0;
	virtual std::shared_ptr<XRHIComputeShader> RHICreateComputeShader(XArrayView<uint8> Code) = 0;
	
	//CreatePSO
	virtual std::shared_ptr<XRHIGraphicsPSO> RHICreateGraphicsPipelineState(const  XGraphicsPSOInitializer& PSOInit) = 0;
	virtual std::shared_ptr<XRHIComputePSO> RHICreateComputePipelineState(const XRHIComputeShader* RHIComputeShader) = 0;

	virtual std::shared_ptr<XRHITexture2D> RHICreateTexture2D(uint32 width, uint32 height, uint32 SizeZ, bool bTextureArray,bool bCubeTexture, EPixelFormat Format, ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data) = 0;
	virtual std::shared_ptr<XRHITexture3D> RHICreateTexture3D(uint32 width, uint32 height, uint32 SizeZ, EPixelFormat Format,ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data) = 0;

	virtual std::shared_ptr<XRHIConstantBuffer> RHICreateConstantBuffer(uint32 size) = 0;

	virtual XRHITexture* RHIGetCurrentBackTexture() = 0;
	
	//Lock/UnLock Vertex Buffer
	virtual void* LockVertexBuffer(XRHIVertexBuffer* VertexBuffer, uint32 Offset, uint32 SizeRHI) = 0;
	virtual void UnLockVertexBuffer(XRHIVertexBuffer* VertexBuffer) = 0;

	virtual void* LockIndexBuffer(XRHIIndexBuffer* IndexBuffer, uint32 Offset, uint32 SizeRHI) = 0;
	virtual void UnLockIndexBuffer(XRHIIndexBuffer* IndexBuffer) = 0;

};


extern XPlatformRHI* GPlatformRHI;
extern bool GIsRHIInitialized;

inline std::shared_ptr<XRHIVertexLayout> RHICreateVertexLayout(const XRHIVertexLayoutArray& Elements)
{
	return GPlatformRHI->RHICreateVertexDeclaration(Elements);
}

inline std::shared_ptr<XRHIRasterizationState> RHICreateRasterizationStateState(const XRasterizationStateInitializerRHI& Initializer)
{
	return GPlatformRHI->RHICreateRasterizationStateState(Initializer);
}

inline std::shared_ptr<XRHIDepthStencilState> RHICreateDepthStencilState(const XDepthStencilStateInitializerRHI& Initializer)
{
	return GPlatformRHI->RHICreateDepthStencilState(Initializer);
}

inline std::shared_ptr<XRHIBlendState> RHICreateBlendState(const XBlendStateInitializerRHI& Initializer)
{
	return GPlatformRHI->RHICreateBlendState(Initializer);
}

inline std::shared_ptr<XRHIGraphicsPSO> RHICreateGraphicsPipelineState(const XGraphicsPSOInitializer& PSOInit)
{
	return GPlatformRHI->RHICreateGraphicsPipelineState(PSOInit);
}

inline std::shared_ptr<XRHIComputePSO> RHICreateComputePipelineState(const XRHIComputeShader* RHIComputeShader)
{
	return GPlatformRHI->RHICreateComputePipelineState(RHIComputeShader);
}