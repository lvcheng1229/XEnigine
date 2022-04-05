#include "D3D12AbstractDevice.h"
template XD3D12VertexBuffer* XD3D12AbstractDevice::CreateRHIVIBuffer<XD3D12VertexBuffer>(XD3D12DirectCommandList* D3D12CmdList, const D3D12_RESOURCE_DESC& InDesc, uint32 Alignment, uint32 Stride, uint32 Size, EBufferUsage InUsage, XRHIResourceCreateData& CreateData);

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
	BufferRet = new BufferType(Stride, Size);
	VIBufferBufferAllocDefault.Allocate(Size, Alignment, BufferRet->ResourcePtr);

	if (CreateData.ResourceArray != nullptr && bDynamic == false)
	{
		XD3D12ResourcePtr_CPUGPU UpLoadResourcePtr;
		bool res = UploadHeapAlloc.Allocate(Size, Alignment, UpLoadResourcePtr);
		X_Assert(res == true);

		D3D12_SUBRESOURCE_DATA ResourceData = {};
		ResourceData.pData = CreateData.ResourceArray->GetResourceData();
		ResourceData.RowPitch = CreateData.ResourceArray->GetResourceDataSize();
		ResourceData.SlicePitch = ResourceData.RowPitch;
		
		D3D12CmdList->GetDXCmdList()->ResourceBarrier(1, GetRValuePtr((CD3DX12_RESOURCE_BARRIER::Transition(
			UpLoadResourcePtr.GetBackResource()->GetResource(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COPY_DEST))));

		CopyDataFromUploadToDefaulHeap(D3D12CmdList->GetDXCmdList(), UpLoadResourcePtr.GetBackResource()->GetResource(),
			UploadHeapAlloc.GetDXResource(),
			UploadHeapAlloc.GetAllocationOffsetInBytes(UpLoadResourcePtr.GetBuddyAllocData()),
			0, 1, &ResourceData);
		
		D3D12CmdList->GetDXCmdList()->ResourceBarrier(1, GetRValuePtr((CD3DX12_RESOURCE_BARRIER::Transition(
			UpLoadResourcePtr.GetBackResource()->GetResource(),
			D3D12_RESOURCE_STATE_COPY_DEST, 
			D3D12_RESOURCE_STATE_GENERIC_READ))));

		CreateData.ResourceArray->ReleaseData();
	}
	return BufferRet;
}