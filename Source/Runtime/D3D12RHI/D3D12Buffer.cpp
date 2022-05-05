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

	//const XD3D12ResourcePtr_CPUGPU& SrcResourcePtr = static_cast<XD3D12StructBuffer*>(D3D12ZeroStructBuffer.get())->ResourcePtr;
	//ID3D12Resource* SrcResource = SrcResourcePtr.GetBackResource()->GetResource();
	//uint64 SrcOffset = SrcResourcePtr.GetOffsetByteFromBaseResource();

	D3D12CmdList->GetDXCmdList()->CopyBufferRegion(DestResource, DestOffset + CounterOffset, ID3D12ZeroStructBuffer.Get(), 0, sizeof(UINT));
}

void XD3D12PlatformRHI::RHIResetStructBufferCounter(XRHIStructBuffer* RHIStructBuffer, uint32 CounterOffset)
{
	AbsDevice->DeviceResetStructBufferCounter(AbsDevice->GetDirectContex(0)->GetCmdList(), RHIStructBuffer, CounterOffset);
}

XD3D12ShaderResourceView* XD3D12AbstractDevice::RHICreateShaderResourceView(XRHIStructBuffer* StructuredBuffer)
{
	XD3D12StructBuffer* D3D12StructuredBuffer = static_cast<XD3D12StructBuffer*>(StructuredBuffer);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = StructuredBuffer->GetSize() / StructuredBuffer->GetStride();
	srvDesc.Buffer.StructureByteStride = StructuredBuffer->GetStride();
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	XD3D12Resource* BackResource = static_cast<XD3D12StructBuffer*>(StructuredBuffer)->ResourcePtr.GetBackResource();

	uint32 index_of_desc_in_heap;
	uint32 index_of_heap;
	CBV_SRV_UAVDescArrayManager.AllocateDesc(index_of_desc_in_heap, index_of_heap);
	D3D12_CPU_DESCRIPTOR_HANDLE CPU_PTR = CBV_SRV_UAVDescArrayManager.compute_cpu_ptr(index_of_desc_in_heap, index_of_heap);

	XD3D12ShaderResourceView* ShaderResourceView = new  XD3D12ShaderResourceView();
	ShaderResourceView->Create(PhysicalDevice, BackResource, srvDesc, CPU_PTR);
	return ShaderResourceView;
}

XD3D12UnorderedAcessView* XD3D12AbstractDevice::RHICreateUnorderedAccessView(XRHIStructBuffer* StructuredBuffer, bool bUseUAVCounter, bool bAppendBuffer,uint64 CounterOffsetInBytes)
{
	XD3D12StructBuffer* D3D12StructBufferStructuredBuffer =static_cast<XD3D12StructBuffer*>(StructuredBuffer);
	XD3D12ResourcePtr_CPUGPU& Ptr = D3D12StructBufferStructuredBuffer->ResourcePtr;
	
	XD3D12Resource* D3D12Resource = Ptr.GetBackResource();
	
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = Ptr.GetOffsetByteFromBaseResource() / StructuredBuffer->GetStride();
	uavDesc.Buffer.NumElements = StructuredBuffer->GetSize()/ StructuredBuffer->GetStride();
	uavDesc.Buffer.StructureByteStride = StructuredBuffer->GetStride();
	uavDesc.Buffer.CounterOffsetInBytes = CounterOffsetInBytes;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	uint32 index_of_desc_in_heap;
	uint32 index_of_heap;
	CBV_SRV_UAVDescArrayManager.AllocateDesc(index_of_desc_in_heap, index_of_heap);
	D3D12_CPU_DESCRIPTOR_HANDLE CPU_PTR = CBV_SRV_UAVDescArrayManager.compute_cpu_ptr(index_of_desc_in_heap, index_of_heap);
	XD3D12UnorderedAcessView* UnorderedAcessView = new  XD3D12UnorderedAcessView();
	UnorderedAcessView->Create(PhysicalDevice, D3D12Resource, D3D12Resource, uavDesc, CPU_PTR);
	
	X_Assert(bUseUAVCounter == true);
	X_Assert(bAppendBuffer == true);
	return UnorderedAcessView;
}
std::shared_ptr<XRHIUnorderedAcessView> XD3D12PlatformRHI::RHICreateUnorderedAccessView(XRHIStructBuffer* StructuredBuffer, bool bUseUAVCounter, bool bAppendBuffer, uint64 CounterOffsetInBytes)
{
	XD3D12UnorderedAcessView* UnorderedAcessView = AbsDevice->RHICreateUnorderedAccessView(StructuredBuffer, bUseUAVCounter, bAppendBuffer, CounterOffsetInBytes);
	return std::shared_ptr<XRHIUnorderedAcessView>(UnorderedAcessView);
}

std::shared_ptr<XRHIShaderResourceView> XD3D12PlatformRHI::RHICreateShaderResourceView(XRHIStructBuffer* StructuredBuffer)
{
	XD3D12ShaderResourceView* ShaderResourceView=AbsDevice->RHICreateShaderResourceView(StructuredBuffer);
	return std::shared_ptr<XRHIShaderResourceView>(ShaderResourceView);
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
		BufferDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	const uint32 Alignment = 
		(((int)Usage & ((int)EBufferUsage::BUF_DrawIndirect)) == 0) ? 
		Stride
		: 4;
	XD3D12StructBuffer* IndexBuffer = AbsDevice->DeviceCreateRHIBuffer<XD3D12StructBuffer>(AbsDevice->GetDirectContex(0)->GetCmdList(), BufferDesc, Alignment, Stride, Size, Usage, ResourceData);
	return std::shared_ptr<XRHIStructBuffer>(IndexBuffer);
}



