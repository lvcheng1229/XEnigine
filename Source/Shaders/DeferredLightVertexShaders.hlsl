#include "Common.hlsl"


void DeferredLightVertexMain(
    float2 PosIn    : POSITION,
	float2 TexC : TEXCOORD,
	out float2 OutTexCoord : TEXCOORD0,
	out float3 OutScreenVector : TEXCOORD1,
	out float4 OutPosition : SV_POSITION
)
{
    OutTexCoord=TexC;
    OutPosition=float4(PosIn,0.0f,1.0f);
    OutScreenVector=mul(float4(OutPosition.xy,1.0f,0.0f),View_ScreenToTranslatedWorld);
}

 