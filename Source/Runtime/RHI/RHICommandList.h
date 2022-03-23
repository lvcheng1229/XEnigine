#pragma once
#include "PlatformRHI.h"

inline std::shared_ptr<XRHIVertexShader> RHICreateVertexShader(std::string_view Code)
{
	return GPlatformRHI->RHICreateVertexShader(Code);
}

inline std::shared_ptr<XRHIPixelShader> RHICreatePixelShader(std::string_view Code)
{
	return GPlatformRHI->RHICreatePixelShader(Code);
}
