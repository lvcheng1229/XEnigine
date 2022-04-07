#include "D3D12AbstractDevice.h"
template XD3D12VertexBuffer* XD3D12AbstractDevice::CreateRHIVIBuffer<XD3D12VertexBuffer>(XD3D12DirectCommandList* D3D12CmdList, const D3D12_RESOURCE_DESC& InDesc, uint32 Alignment, uint32 Stride, uint32 Size, EBufferUsage InUsage, XRHIResourceCreateData& CreateData);
template XD3D12IndexBuffer* XD3D12AbstractDevice::CreateRHIVIBuffer<XD3D12IndexBuffer>(XD3D12DirectCommandList* D3D12CmdList, const D3D12_RESOURCE_DESC& InDesc, uint32 Alignment, uint32 Stride, uint32 Size, EBufferUsage InUsage, XRHIResourceCreateData& CreateData);

template<typename BufferType>
BufferType* XD3D12AbstractDevice::CreateRHIVIBuffer(
	XD3D12DirectCommandList* D3D12CmdList,
	const D3D12_RESOURCE_DESC& InDesc,
	uint32 Alignment, uint32 Stride, uint32 Size,
	EBufferUsage InUsage, XRHIResourceCreateData& CreateData)
{
	const bool bDynamic = ((uint32)InUsage & (uint32)EBufferUsage::BUF_AnyDynamic) ? true : false;
	X_Assert(bDynamic == false);//Dont Surpport Dynamic Now
	
	BufferType* BufferRet = new BufferType(Stride, Size);
	VIBufferBufferAllocDefault.Allocate(Size, Alignment, BufferRet->ResourcePtr);

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