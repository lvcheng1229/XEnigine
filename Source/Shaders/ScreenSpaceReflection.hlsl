#include "Common.hlsl"

Texture2D SceneColor;

cbuffer cbSSR
{
    float4 cbSSR_SSRParams;
}

void ScreenSpaceReflections(
	float4 SvPosition, out float4 OutColor)
{
    float2 UV = SvPosition.xy * View_BufferSizeAndInvSize.zw;
    OutColor=SceneColor.Sample(gsamLinearWarp,UV);

    float TempRes=cbSSR_SSRParams.x;
    TempRes*=0.0f;
    OutColor.x+=TempRes;
}