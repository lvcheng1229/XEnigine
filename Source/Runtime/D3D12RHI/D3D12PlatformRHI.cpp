#include "Runtime/RHI/RHICommandList.h"
#include "D3D12PlatformRHI.h"
#include "D3D12AbstractDevice.h"
#include "D3D12Viewport.h"
#include "Runtime/ApplicationCore/Windows/WindowsApplication.h"

static XD3D12PhysicDevice* PhyDevice = nullptr;
static XD3D12AbstractDevice* AbsDevice = nullptr;
static XD3D12Viewport* D3DViewPort;
void XD3D12RHIModule::ReleaseRHI()
{
	delete D3DViewPort;
	delete AbsDevice;
	delete PhyDevice;
	D3DAdapterPtr.reset();
}


XRHITexture* XD3D12PlatformRHI::RHIGetCurrentBackTexture()
{
	return D3DViewPort->GetCurrentBackTexture();
}

XPlatformRHI* XD3D12RHIModule::CreateRHI()
{
	XDxRefCount<ID3D12Debug> debugController;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();

	D3DAdapterPtr = std::make_shared<XD3D12Adapter>();
	D3DAdapterPtr->Create();
	
	PhyDevice = new XD3D12PhysicDevice();
	PhyDevice->Create(D3DAdapterPtr.get());

	AbsDevice = new XD3D12AbstractDevice();
	AbsDevice->Create(PhyDevice);

	D3DViewPort = new XD3D12Viewport();
	AbsDevice->SetViewPort(D3DViewPort);

	XPlatformRHI* RHIRet = new XD3D12PlatformRHI(AbsDevice);
	D3DViewPort->Create(AbsDevice, XWindowsApplication::APPPtr->ClientWidth, XWindowsApplication::APPPtr->ClientHeight,
		EPixelFormat::FT_R8G8B8A8_UNORM, (HWND)XWindowsApplication::APPPtr->GetPlatformHandle());
	XD3DDirectContex* DirectCtx = AbsDevice->GetDirectContex(0);
	
	GRHICmdList.SetContext(DirectCtx);

	return RHIRet;
}

void* XD3D12PlatformRHI::LockVertexBuffer(XRHIVertexBuffer* VertexBuffer, uint32 Offset, uint32 SizeRHI)
{
	XD3D12VertexBuffer* D3DVertexBuffer = static_cast<XD3D12VertexBuffer*>(VertexBuffer);
	return D3DVertexBuffer->ResourcePtr.GetMappedCPUResourcePtr();
}

void* XD3D12PlatformRHI::LockIndexBuffer(XRHIIndexBuffer* IndexBuffer, uint32 Offset, uint32 SizeRHI)
{
	XD3D12IndexBuffer* D3DIndexBuffer = static_cast<XD3D12IndexBuffer*>(IndexBuffer);
	return D3DIndexBuffer->ResourcePtr.GetMappedCPUResourcePtr();
}

void XD3D12PlatformRHI::UnLockIndexBuffer(XRHIIndexBuffer* IndexBuffer)
{

}
void XD3D12PlatformRHI::UnLockVertexBuffer(XRHIVertexBuffer* VertexBuffer)
{

}
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
	GPixelFormats[(int)EPixelFormat::FT_R32G32B32A32_UINT].PlatformFormat = DXGI_FORMAT_R32G32B32A32_UINT;
	GPixelFormats[(int)EPixelFormat::FT_R32_TYPELESS].PlatformFormat = DXGI_FORMAT_R32_TYPELESS;
}

