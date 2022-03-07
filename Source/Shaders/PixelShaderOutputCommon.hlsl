Texture2D    BaseColorMap;
SamplerState gsamPointWarp  : register(s0,space1000);

cbuffer cbPerObject
{
    float4x4 gWorld;
}

cbuffer cbPass
{
    float4x4 gViewProj;
}

//cbuffer cbMaterial
//{
//	float4 gDiffuseAlbedo;
//    float3 gFresnelR0;
//    float  gRoughness;
//    float4x4 gMatTransform;
//};



struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	float2 TexC    : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut)0.0f;
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);
    vout.PosH = mul(posW, gViewProj);
    vout.TexC = vin.TexC;
    return vout;
}

void PS(VertexOut pin,
    out float4 OutTarget0 : SV_Target0,
    out float4 OutTarget1 : SV_Target1,
    out float4 OutTarget2 : SV_Target2,
    out float4 OutTarget3 : SV_Target3
    )
{
    float4 BaseColor = BaseColorMap.Sample(gsamPointWarp, pin.TexC);
    OutTarget0=BaseColor;
    OutTarget1=float4(1.0f,0.0f,0.0f,1.0f);
    OutTarget2=float4(1.0f,1.0f,0.0f,1.0f);
    OutTarget3=float4(BaseColor.xyz,1.0f);
}