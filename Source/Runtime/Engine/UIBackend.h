#pragma once
#include "Runtime/RHI/RHICommandList.h"
class RHIUI
{
public:
	void ImGui_Impl_RHI_Init();
	void ImGui_Impl_RHI_Shutdown();

	void ImGui_Impl_RHI_NewFrame(XRHICommandList* RHICmdList);

private:

};