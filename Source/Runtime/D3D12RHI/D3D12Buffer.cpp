#include "D3D12AbstractDevice.h"

template<typename BufferType>
BufferType* XD3D12AbstractDevice::CreateRHIBuffer(
	XD3D12DirectCommandList* D3D12CmdList,
	const D3D12_RESOURCE_DESC& InDesc,
	uint32 Alignment, uint32 Stride, uint32 Size,
	EBufferUsage InUsage, XRHIResourceCreateData& CreateData)
{
	const bool bDynamic = ((uint32)InUsage & (uint32)EBufferUsage::BUF_AnyDynamic) ? true : false;
	
	if (bDynamic)
	{
		X_Assert(false);
	}
	else
	{
		//VIBufferBufferAllocDefault.Allocate(Size, Alignment,)
	}
}