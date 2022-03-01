#include "DX12GIAdapter.h"

void XDXGIAdapter::Create()
{
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory)));
	
	//0x10DE : Nvidia | 0x8086 : Intel | 0x1002 : AMD
	int PreferredVendor = 0x10DE;
	
	for (UINT AdapterIndex = 0; dxgi_factory->EnumAdapters(AdapterIndex, &dxgi_adapter) != DXGI_ERROR_NOT_FOUND; ++AdapterIndex)
	{
		if (dxgi_adapter)
		{
			DXGI_ADAPTER_DESC AdapterDesc;
			ThrowIfFailed(dxgi_adapter->GetDesc(&AdapterDesc));
			if (PreferredVendor == AdapterDesc.VendorId) { break; }
		}
	}

	X_Assert(((&dxgi_adapter) != nullptr));
}

