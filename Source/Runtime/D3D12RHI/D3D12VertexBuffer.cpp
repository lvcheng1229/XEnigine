#include "D3D12PlatformRHI.h"
#include "d3dx12.h"

std::shared_ptr<XRHIVertexBuffer> XD3D12PlatformRHI::RHIcreateVertexBuffer(uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData)
{
	const D3D12_RESOURCE_DESC Desc = CD3DX12_RESOURCE_DESC::Buffer(Size);
	return std::shared_ptr<XRHIVertexBuffer>();
}
