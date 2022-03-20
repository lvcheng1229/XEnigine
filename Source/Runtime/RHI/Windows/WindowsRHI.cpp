#include "Runtime/RHI/PlatformRHI.h"
#include "Runtime/D3D12RHI/D3D12PlatformRHI.h"

XPlatformRHI* PlatformCreateDynamicRHI()
{
	return new XD3D12PlatformRHI();
}