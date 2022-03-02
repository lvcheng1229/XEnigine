#include "D3D12PhysicDevice.h"
#include "D3D12CommandQueue.h"

void XD3D12PhysicDevice::Create(XD3D12Adapter* adapter_in)
{
	adapter = adapter_in;
	ThrowIfFailed(D3D12CreateDevice(
		adapter_in->GetDXAdapter(),
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&d3d12_device)));
}


