#pragma once
#include "Runtime/RHI/RHICommandList.h"

class ImDrawData;
class RHIUI
{
public:
	void ImGui_Impl_RHI_Init();
	void ImGui_Impl_RHI_Shutdown();

	void ImGui_Impl_RHI_NewFrame(XRHICommandList* RHICmdList);
	void ImGui_Impl_RHI_RenderDrawData(ImDrawData* draw_data, XRHICommandList* RHICmdList,XRHITexture* SceneColorTex);
private:

};