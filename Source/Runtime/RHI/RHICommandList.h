#pragma once
#include "PlatformRHI.h"

inline std::shared_ptr<XRHIVertexShader> RHICreateVertexShader(XArrayView<uint8> Code)
{
	return GPlatformRHI->RHICreateVertexShader(Code);
}

inline std::shared_ptr<XRHIPixelShader> RHICreatePixelShader(XArrayView<uint8> Code)
{
	return GPlatformRHI->RHICreatePixelShader(Code);
}
