#include "Common.hlsl"
cbuffer cbShadowMaskNoCommon
{
    float4x4 ScreenToShadowMatrix;
    float x_offset;
    float3 cbShadowMaskNoCommon_padding0;
}



Texture2D ShadowDepthTexture;
Texture2D SceneTexturesStruct_GBufferATexture;
Texture2D SceneTexturesStruct_SceneDepthTexture;

float CalcShadowFactor(float2 UVToSampleShadow,float depth,float bias)
{

    uint width, height, numMips;
    ShadowDepthTexture.GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float)width;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
    };

    [unroll]
    for(int i = 0; i < 9; ++i)
    {
        float shadow= ShadowDepthTexture.Sample(gsamPointWarp,
            UVToSampleShadow + offsets[i]).r;
        percentLit +=(shadow>depth+bias?1.0f:0.0f);
    }
    
    return percentLit / 9.0f;
}

void PS(	
    in float4 SVPos : SV_POSITION,
    out float4 OutColor : SV_Target0)
{



    float2 ScreenUV = float2( SVPos.xy * View_BufferSizeAndInvSize.zw );// [0,1]

    float3 WorldNormal=SceneTexturesStruct_GBufferATexture.Sample(gsamPointWarp,ScreenUV).xyz;
    WorldNormal=normalize(WorldNormal);

    float DeviceZ=SceneTexturesStruct_SceneDepthTexture.Sample(gsamPointWarp,ScreenUV).r;
	float NDC_Z=ConvertFromDeviceZ_To_NDCZBeforeDivdeW(DeviceZ);
    ScreenUV*=2.0f;
    ScreenUV-=1.0f;
    ScreenUV.y=ScreenUV.y*-1.0f;

    //float2 ScreenPosition = ( ScreenUV.xy - View.ScreenPositionScaleBias.wz ) / View.ScreenPositionScaleBias.xy;
	float4 ShadowPosition = mul(float4(ScreenUV * NDC_Z,NDC_Z,1), ScreenToShadowMatrix);
    ShadowPosition.xy/=ShadowPosition.w;//[-1,1]
    ShadowPosition.xy+=1.0f;
    ShadowPosition.xy*=0.5f;
    ShadowPosition.y=ShadowPosition.y*-1.0f;
    ShadowPosition.x=ShadowPosition.x*0.25f+x_offset;

    float ShadowSpaceDepth=ShadowDepthTexture.Sample(gsamPointWarp,ShadowPosition.xy).r;
    float Shadow;
       float3 LightDir=float3(-1,1,1);LightDir=normalize(LightDir);
    float bias = max(0.005 * (1.0 - dot(WorldNormal, LightDir)), 0.0005);
    Shadow=CalcShadowFactor(ShadowPosition.xy,ShadowPosition.z,bias);
 
    //if(ShadowSpaceDepth>ShadowPosition.z+bias)
    //{
    //    Shadow=1.0f;//in shadow
    //}
    //else
    //{
    //    Shadow=0.0f;
    //}

    OutColor=float4(Shadow,0.0,0.0f,1.0f);
    
    //useless
    float TempRes=ScreenUV.x;
    TempRes+=ViewZ;
    TempRes+=ShadowPosition.z;
    TempRes+=bias;
    TempRes*=0.00f;
    OutColor.x+=TempRes;
}