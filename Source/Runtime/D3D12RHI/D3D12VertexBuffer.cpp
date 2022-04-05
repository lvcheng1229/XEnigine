#include "D3D12PlatformRHI.h"
#include "D3D12AbstractDevice.h"
#include "d3dx12.h"

std::shared_ptr<XRHIVertexBuffer> XD3D12PlatformRHI::RHIcreateVertexBuffer(uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData)
{
	const D3D12_RESOURCE_DESC Desc = CD3DX12_RESOURCE_DESC::Buffer(Size);
	const uint32 Alignment = 4;
	XD3D12VertexBuffer* VertexBuffer = AbsDevice->CreateRHIVIBuffer<XD3D12VertexBuffer>(
		AbsDevice->GetDirectContex(0)->GetCmdList(), Desc, Alignment, 0, Size, Usage, ResourceData);
	return std::shared_ptr<XRHIVertexBuffer>(VertexBuffer);
}
