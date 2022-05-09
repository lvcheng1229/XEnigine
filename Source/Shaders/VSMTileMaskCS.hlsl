#include "Common.hlsl"
#include "Math.hlsl"
Texture2D TextureSampledInput;
RWTexture2D<uint> VirtualSMFlags;

cbuffer cbShadowViewInfo
{
    row_major float4x4 LightViewProjectMatrix;

    float ClientWidth;
    float ClientHeight;
    float padding0;
    float padding1;
}

[numthreads(16, 16, 1)]
void VSMTileMaskCS(
	uint2 GroupId : SV_GroupID,
	uint GroupThreadIndex : SV_GroupIndex,
    uint2 DispatchThreadID :SV_DispatchThreadID)
{
    float DeviceZ = TextureSampledInput.Load(int3(DispatchThreadID,0));
    if(DeviceZ == 0.0)
    {
        return;
    }

    
    float NDCZ=ConvertFromDeviceZ_To_NDCZBeforeDivdeW(DeviceZ);
    float2 UV = DispatchThreadID * View_BufferSizeAndInvSize.zw;
    float2 ScreenPos = UV * 2.0f - 1.0f; ScreenPos.y *= -1.0f;

    float4 NDCPosNoDivdeW = float4(ScreenPos * NDCZ, NDCZ, 1);
    float4 WorldPosition = mul_x(NDCPosNoDivdeW,cbView_ViewPorjectionMatrixInverse);
    float4 ShadowScreenPOS = mul(float4(WorldPosition.xyz,1.0),LightViewProjectMatrix);
    float2 UVOut = ShadowScreenPOS.xy * 0.5 +0.5f;
    UVOut.y*=-1.0;
    //ShadowScreenPOS.xyz/=ShadowScreenPOS.w;
    InterlockedCompareStore(VirtualSMFlags[uint2(UVOut * 8 * 8)],0,1);
}