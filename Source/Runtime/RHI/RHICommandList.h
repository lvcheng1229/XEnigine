#pragma once
#include "PlatformRHI.h"
#include "RHIContext.h"
#include <array>
class XRHICommandListBase
{
protected:
	struct FPSOContext
	{
		uint32 CachedNumRTs = 0;
		XRHIDepthStencilView CachedDepthStencilTarget;
		std::array<XRHIRenderTargetView, 8>CachedRenderTargets;
	} PSOContext;
public:

	void SetContext(IRHIContext* InContext)
	{
		Context = InContext;
		ComputeContext = InContext;
	}
	IRHIContext* GetContext()const
	{
		return Context;
	}

	inline void SetComputeContext(IRHIContext* Context)
	{
		ComputeContext = Context;
	}
	IRHIContext* GetComputeContext()const
	{
		return ComputeContext;
	}

	void CacheActiveRenderTargets(
		uint32 NewNumRTs,
		const XRHIRenderTargetView* NewRenderTargetsRHI,
		const XRHIDepthStencilView* NewDepthStencilRHI)
	{
		PSOContext.CachedNumRTs = NewNumRTs;
		for (int i = 0; i < NewNumRTs; i++)
		{
			PSOContext.CachedRenderTargets[i] = NewRenderTargetsRHI[i];
		}
		PSOContext.CachedDepthStencilTarget = (NewDepthStencilRHI == nullptr ? XRHIDepthStencilView() : *NewDepthStencilRHI);
	}

	void CacheActiveRenderTargets(const XRHIRenderPassInfo& Info)
	{
		XRHISetRenderTargetsInfo RTInfo;
		Info.ConvertToRenderTargetsInfo(RTInfo);
		CacheActiveRenderTargets(RTInfo.NumColorRenderTargets, RTInfo.ColorRenderTarget, &RTInfo.DepthStencilRenderTarget);
	}

	void ApplyCachedRenderTargets(XGraphicsPSOInitializer& GraphicsPSOInit)
	{
		GraphicsPSOInit.RTNums = PSOContext.CachedNumRTs;
		for (uint32 i = 0; i < PSOContext.CachedNumRTs; i++)
		{
			GraphicsPSOInit.RT_Format[i] = PSOContext.CachedRenderTargets[i].Texture->GetFormat();
		}

		if (PSOContext.CachedDepthStencilTarget.Texture != nullptr)
		{
			GraphicsPSOInit.DS_Format = PSOContext.CachedDepthStencilTarget.Texture->GetFormat();
		}
		else
		{
			GraphicsPSOInit.DS_Format = EPixelFormat::FT_Unknown;
		}
	}
private:
	IRHIContext* Context;
	IRHIContext* ComputeContext;
};

class XRHIComputeCommandList :public XRHICommandListBase
{
public:
	inline void RHIDispatchComputeShader(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ)
	{
		GetComputeContext()->RHIDispatchComputeShader(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
	}

	inline void SetComputePipelineState(class XRHIComputePSO* ComputesPipelineState)
	{
		GetComputeContext()->RHISetComputePipelineState(ComputesPipelineState);
	}

	inline void SetConstantBuffer(EShaderType ShaderType, uint32 BufferIndex, XRHIConstantBuffer* RHICBV)
	{
		GetComputeContext()->RHISetShaderConstantBuffer(ShaderType, BufferIndex, RHICBV);
	}

	inline void SetShaderTexture(EShaderType ShaderType, uint32 TextureIndex, XRHITexture* Texture)
	{
		GetComputeContext()->RHISetShaderTexture(ShaderType, TextureIndex, Texture);
	}

	inline void SetShaderUAV(EShaderType ShaderType, uint32 TextureIndex, XRHIUnorderedAcessView* RHIUAV)
	{
		GetComputeContext()->RHISetShaderUAV(ShaderType, TextureIndex, RHIUAV);
	}
};

class XRHICommandList : public XRHIComputeCommandList
{
public:
	inline void RHIBeginRenderPass(const XRHIRenderPassInfo& InInfo, const wchar_t* InName)
	{
		GetContext()->RHIBeginRenderPass(InInfo, InName);
	}

	inline void RHIDrawIndexedPrimitive(
		XRHIIndexBuffer* IndexBuffer,
		uint32 IndexCountPerInstance,
		uint32 InstanceCount,
		uint32 StartIndexLocation,
		uint32 BaseVertexLocation,
		uint32 StartInstanceLocation)
	{
		GetContext()->RHIDrawIndexedPrimitive(IndexBuffer, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
	}

	inline void RHIDrawIndexedPrimitive()
	{
		GetContext()->RHIDrawIndexedPrimitive();
	}

	inline void SetGraphicsPipelineState(class XRHIGraphicsPSO* GraphicsPipelineState)
	{
		GetContext()->RHISetGraphicsPipelineState(GraphicsPipelineState);
	}

	inline void SetConstantBuffer(EShaderType ShaderType, uint32 BufferIndex, XRHIConstantBuffer* RHICBV)
	{
		GetContext()->RHISetShaderConstantBuffer(ShaderType, BufferIndex, RHICBV);
	}

	inline void SetShaderValue(EShaderType ShaderType, uint32 BufferIndex,uint32 VariableOffsetInBuffer, uint32 NumBytes, const void* NewValue)
	{
		GetContext()->SetShaderValue(ShaderType, BufferIndex, VariableOffsetInBuffer, NumBytes, NewValue);
	}

	inline void SetShaderTexture(EShaderType ShaderType, uint32 TextureIndex, XRHITexture* Texture)
	{
		GetContext()->RHISetShaderTexture(ShaderType, TextureIndex, Texture);
	}

	inline void SetVertexBuffer(XRHIVertexBuffer* RHIVertexBuffer, uint32 VertexBufferSlot, uint32 OffsetFormVBBegin)
	{
		GetContext()->SetVertexBuffer(RHIVertexBuffer, VertexBufferSlot, OffsetFormVBBegin);
	}
};

//TODO


inline void* LockVertexBuffer(XRHIVertexBuffer* VertexBuffer, uint32 Offset, uint32 SizeRHI)
{
	return GPlatformRHI->LockVertexBuffer(VertexBuffer, Offset, SizeRHI);
}
inline void UnLockVertexBuffer(XRHIVertexBuffer* VertexBuffer)
{
	return GPlatformRHI->UnLockVertexBuffer(VertexBuffer);
}

inline void* LockIndexBuffer(XRHIIndexBuffer* IndexBuffer, uint32 Offset, uint32 SizeRHI)
{
	return GPlatformRHI->LockIndexBuffer(IndexBuffer, Offset, SizeRHI);
}
inline void UnLockIndexBuffer(XRHIIndexBuffer* IndexBuffer)
{
	return GPlatformRHI->UnLockIndexBuffer(IndexBuffer);
}

inline std::shared_ptr<XRHIConstantBuffer> RHICreateConstantBuffer(uint32 size)
{
	return GPlatformRHI->RHICreateConstantBuffer(size);
}

inline std::shared_ptr<XRHITexture2D> RHICreateTexture2D(uint32 width, uint32 height, uint32 SizeZ,
	bool bTextureArray, bool bCubeTexture, EPixelFormat Format,
	ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data)
{
	return GPlatformRHI->RHICreateTexture2D(width, height, SizeZ, bTextureArray, bCubeTexture, Format, flag, NumMipsIn, tex_data);
}

inline std::shared_ptr<XRHITexture3D> RHICreateTexture3D(uint32 width, uint32 height, uint32 SizeZ, EPixelFormat Format,
	ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data)
{
	return GPlatformRHI->RHICreateTexture3D(width, height, SizeZ, Format, flag, NumMipsIn, tex_data);
}

inline std::shared_ptr<XRHIVertexBuffer>RHIcreateVertexBuffer(uint32 Stride, uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData)
{
	return GPlatformRHI->RHIcreateVertexBuffer(Stride, Size, Usage, ResourceData);
}

inline void RHIResetStructBufferCounter(XRHIStructBuffer* RHIStructBuffer,uint32 CounterOffset)
{
	GPlatformRHI->RHIResetStructBufferCounter(RHIStructBuffer, CounterOffset);
}

inline std::shared_ptr<XRHIStructBuffer>RHIcreateStructBuffer(uint32 Stride, uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData)
{
	return GPlatformRHI->RHIcreateStructBuffer(Stride, Size, Usage, ResourceData);
}

inline std::shared_ptr<XRHIIndexBuffer>RHICreateIndexBuffer(uint32 Stride, uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData)
{
	return GPlatformRHI->RHICreateIndexBuffer(Stride, Size, Usage, ResourceData);
}

inline std::shared_ptr<XRHIVertexShader> RHICreateVertexShader(XArrayView<uint8> Code)
{
	return GPlatformRHI->RHICreateVertexShader(Code);
}

inline std::shared_ptr<XRHIPixelShader> RHICreatePixelShader(XArrayView<uint8> Code)
{
	return GPlatformRHI->RHICreatePixelShader(Code);
}

inline std::shared_ptr<XRHIComputeShader> RHICreateComputeShader(XArrayView<uint8> Code)
{
	return GPlatformRHI->RHICreateComputeShader(Code);
}
