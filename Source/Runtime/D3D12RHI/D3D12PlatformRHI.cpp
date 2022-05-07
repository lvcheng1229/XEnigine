#include "D3D12PlatformRHI.h"
#include "D3D12AbstractDevice.h"
XD3D12PlatformRHI::XD3D12PlatformRHI(XD3D12AbstractDevice* InAbsDevice)
{
	AbsDevice = InAbsDevice;
	PhyDevice = AbsDevice->GetPhysicalDevice();
	GPixelFormats[(int)EPixelFormat::FT_Unknown].PlatformFormat = DXGI_FORMAT_UNKNOWN;
	GPixelFormats[(int)EPixelFormat::FT_R16G16B16A16_FLOAT].PlatformFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	GPixelFormats[(int)EPixelFormat::FT_R8G8B8A8_UNORM].PlatformFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	GPixelFormats[(int)EPixelFormat::FT_R8G8B8A8_UNORM_SRGB].PlatformFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	GPixelFormats[(int)EPixelFormat::FT_R24G8_TYPELESS].PlatformFormat = DXGI_FORMAT_R24G8_TYPELESS;
	GPixelFormats[(int)EPixelFormat::FT_R11G11B10_FLOAT].PlatformFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	GPixelFormats[(int)EPixelFormat::FT_R16_FLOAT].PlatformFormat = DXGI_FORMAT_R16_FLOAT;
	GPixelFormats[(int)EPixelFormat::FT_R32_UINT].PlatformFormat = DXGI_FORMAT_R32_UINT;
}

void* XD3D12PlatformRHI::LockVertexBuffer(XRHIVertexBuffer* VertexBuffer, uint32 Offset, uint32 SizeRHI)
{
	XD3D12VertexBuffer* D3DVertexBuffer = static_cast<XD3D12VertexBuffer*>(VertexBuffer);
	return D3DVertexBuffer->ResourcePtr.GetMappedCPUResourcePtr();
}
void XD3D12PlatformRHI::UnLockVertexBuffer(XRHIVertexBuffer* VertexBuffer)
{

}

void* XD3D12PlatformRHI::LockIndexBuffer(XRHIIndexBuffer* IndexBuffer, uint32 Offset, uint32 SizeRHI)
{
	XD3D12IndexBuffer* D3DIndexBuffer = static_cast<XD3D12IndexBuffer*>(IndexBuffer);
	return D3DIndexBuffer->ResourcePtr.GetMappedCPUResourcePtr();
}

void XD3D12PlatformRHI::UnLockIndexBuffer(XRHIIndexBuffer* IndexBuffer)
{
}

