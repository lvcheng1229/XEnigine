#include "Common.hlsl"

RWTexture2D<float>VsmShadowMaskTex;

Texture2D SceneDepthInput;
Texture2D<uint4> PagetableInfos; //64*64
Texture2D<uint>PhysicalShadowDepthTexture; // 1024 * 1024

cbuffer cbShadowViewInfo
{
    row_major float4x4 LightViewProjectMatrix;

    float ClientWidth;
    float ClientHeight;
    float padding0;
    float padding1;
}

#define PageNum 8*8
#define PhysicalTileWidthNum 8
#define PhysicalSize 1024

[numthreads(16, 16, 1)]
void ShadowMaskGenCS(uint2 DispatchThreadID :SV_DispatchThreadID)
{
    float DeviceZ = SceneDepthInput.Load(int3(DispatchThreadID,0));
    float2 UV = DispatchThreadID * View_BufferSizeAndInvSize.zw;

    if(UV.x > 1.0f || UV.y > 1.0f)
    {
        return;
    }

    if(DeviceZ == 0.0)
    {
        VsmShadowMaskTex[DispatchThreadID] =0.0f;
        return;
    }
    
    float2 ScreenPos = UV * 2.0f - 1.0f; ScreenPos.y *= -1.0f;

    float4 NDCPosNoDivdeW = float4(ScreenPos , DeviceZ , 1.0);
    float4 WorldPosition = mul(cbView_ViewPorjectionMatrixInverse,NDCPosNoDivdeW);
    WorldPosition.xyz/=WorldPosition.w;

    float4 ShadowScreenPOS = mul(float4(WorldPosition.xyz,1.0),LightViewProjectMatrix);
    ShadowScreenPOS.xyz/=ShadowScreenPOS.w;

    float2 UVShadowSpace = ShadowScreenPOS.xy; 
    UVShadowSpace.y*=-1.0;
    UVShadowSpace = UVShadowSpace* 0.5 + 0.5f;

    uint2 TileIndexXY = uint2(UVShadowSpace * PageNum /*- 0.5f*/);
    uint PageTableIndex = PagetableInfos[TileIndexXY].x;

    uint PhysicalIndexX = PageTableIndex % uint(PhysicalTileWidthNum);
    uint PhysicalIndexY = PageTableIndex / uint(PhysicalTileWidthNum);

    float2 SubTileUV = (UVShadowSpace * PageNum) - uint2(UVShadowSpace * PageNum);
    float2 ShadowDepthPos = float2(PhysicalIndexX,PhysicalIndexY) + SubTileUV; //float [0,8]
    ShadowDepthPos/=PhysicalTileWidthNum; //[0,1]
    ShadowDepthPos*=PhysicalSize;//[0,1024]
    
    uint ShadowDepth = PhysicalShadowDepthTexture[uint2(ShadowDepthPos)].x;
    uint ObjectShadowDepth = (ShadowScreenPOS.z)* (1<<30);

    //ObjectShadowDepth <= ShadowDepth ,if in shadow

    //inverse z
    if(ObjectShadowDepth < ShadowDepth) // InShadow
    {
        VsmShadowMaskTex[DispatchThreadID] = (ShadowDepth - ObjectShadowDepth) / float( 1 << 10);
    }
    else  //Not In Shadow
    {
        VsmShadowMaskTex[DispatchThreadID] =  (ShadowDepth - ObjectShadowDepth) / float( 1 << 15);
    }
}