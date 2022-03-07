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

//Local Vertex Factory , if create Landscape , then this file chnage to LandSacpeVertextFactory.ush
struct FVertexFactoryInput
{
    float4	Position	: ATTRIBUTE0;
    float3	TangentX	: ATTRIBUTE1;
    float4	TangentZ	: ATTRIBUTE2;
    float2	TexCoord    : ATTRIBUTE4;
};

struct FVertexFactoryInterpolantsVSToPS
{
	float4 TangentToWorld0 : TEXCOORD10; 
    float4 TangentToWorld2 : TEXCOORD11;
	float4 TexCoords : TEXCOORD0;
};



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