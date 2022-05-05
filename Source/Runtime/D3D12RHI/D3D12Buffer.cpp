#include "d3dx12.h"
#include "D3D12PlatformRHI.h"
#include "D3D12AbstractDevice.h"
#include "D3D12Buffer.h"
template XD3D12VertexBuffer* XD3D12AbstractDevice::DeviceCreateRHIBuffer<XD3D12VertexBuffer>(XD3D12DirectCommandList* D3D12CmdList, const D3D12_RESOURCE_DESC& InDesc, uint32 Alignment, uint32 Stride, uint32 Size, EBufferUsage InUsage, XRHIResourceCreateData& CreateData);
template XD3D12IndexBuffer* XD3D12AbstractDevice::DeviceCreateRHIBuffer<XD3D12IndexBuffer>(XD3D12DirectCommandList* D3D12CmdList, const D3D12_RESOURCE_DESC& InDesc, uint32 Alignment, uint32 Stride, uint32 Size, EBufferUsage InUsage, XRHIResourceCreateData& CreateData);
template XD3D12StructBuffer* XD3D12AbstractDevice::DeviceCreateRHIBuffer<XD3D12StructBuffer>(XD3D12DirectCommandList* D3D12CmdList, const D3D12_RESOURCE_DESC& InDesc, uint32 Alignment, uint32 Stride, uint32 Size, EBufferUsage InUsage, XRHIResourceCreateData& CreateData);

template<typename BufferType>
BufferType* XD3D12AbstractDevice::DeviceCreateRHIBuffer(
	XD3D12DirectCommandList* D3D12CmdList,
	const D3D12_RESOURCE_DESC& InDesc,
	uint32 Alignment, uint32 Stride, uint32 Size,
	EBufferUsage InUsage, XRHIResourceCreateData& CreateData)
{
	const bool bDynamic = ((uint32)InUsage & (uint32)EBufferUsage::BUF_AnyDynamic) ? true : false;
	//X_Assert(bDynamic == false);//Dont Surpport Dynamic Now
	
	BufferType* BufferRet = new BufferType(Stride, Size);
	if (bDynamic)
	{
		ConstantBufferUploadHeapAlloc.Allocate(Size, Alignment, BufferRet->ResourcePtr);
	}
	else
	{
		VIBufferBufferAllocDefault.Allocate(Size, Alignment, BufferRet->ResourcePtr);
	}

	
	

	if (CreateData.ResourceArray != nullptr && bDynamic == false)
	{
		XD3D12ResourcePtr_CPUGPU UpLoadResourcePtr;
		bool res = UploadHeapAlloc.Allocate(Size, Alignment, UpLoadResourcePtr);
		X_Assert(res == true);

		memcpy(UpLoadResourcePtr.GetMappedCPUResourcePtr(), CreateData.ResourceArray->GetResourceData(), CreateData.ResourceArray->GetResourceDataSize());

		D3D12CmdList->GetDXCmdList()->CopyBufferRegion(
			BufferRet->ResourcePtr.GetBackResource()->GetResource(),
			BufferRet->ResourcePtr.GetOffsetByteFromBaseResource(),

			UpLoadResourcePtr.GetBackResource()->GetResource(),
			UpLoadResourcePtr.GetOffsetByteFromBaseResource(),

			CreateData.ResourceArray->GetResourceDataSize());

		CreateData.ResourceArray->ReleaseData();
	}
	return BufferRet;
}


void XD3D12AbstractDevice::DeviceResetStructBufferCounter(XD3D12DirectCommandList* D3D12CmdList, XRHIStructBuffer* RHIStructBuffer, uint32 CounterOffset)
{
	const XD3D12ResourcePtr_CPUGPU& DestResourcePtr = static_cast<XD3D12StructBuffer*>(RHIStructBuffer)->ResourcePtr;
	ID3D12Resource* DestResource = DestResourcePtr.GetBackResource()->GetResource();
	uint64 DestOffset = DestResourcePtr.GetOffsetByteFromBaseResource();

	const XD3D12ResourcePtr_CPUGPU& SrcResourcePtr = static_cast<XD3D12StructBuffer*>(D3D12ZeroStructBuffer.get())->ResourcePtr;
	ID3D12Resource* SrcResource = SrcResourcePtr.GetBackResource()->GetResource();
	uint64 SrcOffset = SrcResourcePtr.GetOffsetByteFromBaseResource();

	D3D12CmdList->GetDXCmdList()->CopyBufferRegion(DestResource, DestOffset + CounterOffset, SrcResource, SrcOffset, sizeof(UINT));
}

void XD3D12PlatformRHI::RHIResetStructBufferCounter(XRHIStructBuffer* RHIStructBuffer, uint32 CounterOffset)
{
	AbsDevice->DeviceResetStructBufferCounter(AbsDevice->GetDirectContex(0)->GetCmdList(), RHIStructBuffer, CounterOffset);
}

std::shared_ptr<XRHIVertexBuffer> XD3D12PlatformRHI::RHIcreateVertexBuffer(uint32 Stride, uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData)
{
	const D3D12_RESOURCE_DESC Desc = CD3DX12_RESOURCE_DESC::Buffer(Size);
	const uint32 Alignment = 4;
	XD3D12VertexBuffer* VertexBuffer = AbsDevice->DeviceCreateRHIBuffer<XD3D12VertexBuffer>(AbsDevice->GetDirectContex(0)->GetCmdList(), Desc, Alignment, Stride, Size, Usage, ResourceData);
	return std::shared_ptr<XRHIVertexBuffer>(VertexBuffer);
}

std::shared_ptr<XRHIIndexBuffer> XD3D12PlatformRHI::RHICreateIndexBuffer(uint32 Stride, uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData)
{
	const D3D12_RESOURCE_DESC Desc = CD3DX12_RESOURCE_DESC::Buffer(Size);
	const uint32 Alignment = 4;
	XD3D12IndexBuffer* IndexBuffer = AbsDevice->DeviceCreateRHIBuffer<XD3D12IndexBuffer>(AbsDevice->GetDirectContex(0)->GetCmdList(), Desc, Alignment, Stride, Size, Usage, ResourceData);
	return std::shared_ptr<XRHIIndexBuffer>(IndexBuffer);
}

std::shared_ptr<XRHIStructBuffer> XD3D12PlatformRHI::RHIcreateStructBuffer(uint32 Stride, uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData)
{
	D3D12_RESOURCE_DESC BufferDesc = CD3DX12_RESOURCE_DESC::Buffer(Size);
	if ((int)Usage & ((int)EBufferUsage::BUF_UnorderedAccess))
	{
		BufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	const uint32 Alignment = (((int)Usage & ((int)EBufferUsage::BUF_DrawIndirect)) == 0) ? Stride : 4;
	XD3D12StructBuffer* IndexBuffer = AbsDevice->DeviceCreateRHIBuffer<XD3D12StructBuffer>(AbsDevice->GetDirectContex(0)->GetCmdList(), BufferDesc, Alignment, Stride, Size, Usage, ResourceData);
	return std::shared_ptr<XRHIStructBuffer>(IndexBuffer);
}



