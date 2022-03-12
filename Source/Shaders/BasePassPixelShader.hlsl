Texture2D    BaseColorMap;
Texture2D    NormalMap;
Texture2D    RoughnessMap;

SamplerState gsamPointWarp  : register(s0,space1000);
SamplerState gsamLinearWarp  : register(s4,space1000);

cbuffer cbPerObject
{
    float4x4 gWorld;
}

cbuffer cbPass
{
    float4x4 gViewProj;
}

cbuffer cbMaterial
{
	float Metallic;
    float Specular;
    float Roughness;
    float TextureScale;
    float padding[12];
};

//Local Vertex Factory , if create Landscape , then this file chnage to LandSacpeVertextFactory.ush
struct FVertexFactoryInput
{
    float4	Position	: ATTRIBUTE0;
    float3	TangentX	: ATTRIBUTE1;
    float4	TangentZ	: ATTRIBUTE2;
    float2	TexCoord    : ATTRIBUTE3;
};

struct FVertexFactoryInterpolantsVSToPS
{
	float4 TangentToWorld0 : TEXCOORD10; 
    float4 TangentToWorld2 : TEXCOORD11;
	float4 TexCoords : TEXCOORD0;
    float3 TestWorldPosition : TEXCOORD1;
};


void VS(FVertexFactoryInput Input,
    out FVertexFactoryInterpolantsVSToPS Output,
    out float4 Position : SV_POSITION
    )
{
    Output.TangentToWorld0 = float4(mul(Input.TangentX, (float3x3)gWorld),0.0f);
    Output.TangentToWorld2 = float4(mul(Input.TangentZ.xyz, (float3x3)gWorld),1.0f);
    Output.TexCoords=float4(Input.TexCoord.xy,0.0f,0.0f);
    Output.TestWorldPosition= mul(Input.Position,gWorld).xyz;
    float4 PositionW=mul(Input.Position, gWorld);
    Position=mul(PositionW, gViewProj);
}

struct GBufferdataOutput
{
	float3  BaseColor;
    float   Metallic;
	float   Specular;
	float   Roughness;	
    float3  Normal;
	uint    ShadingModel;
};

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f*normalMapSample - 1.0f;

	// Build orthonormal basis.
	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N)*N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}

void PS(FVertexFactoryInterpolantsVSToPS Input,
    out float4 OutTargetA : SV_Target0,
    out float4 OutTargetB : SV_Target1,
    out float4 OutTargetC : SV_Target2,
    out float4 OutTargetD : SV_Target3
    )
{
    float4 BaseColorSample = BaseColorMap.Sample(gsamLinearWarp, Input.TexCoords.xy*TextureScale);
    float4 RoughnessSampleValue=RoughnessMap.Sample(gsamLinearWarp, Input.TexCoords.xy*TextureScale);
    float4 SampleNormalValue = NormalMap.Sample(gsamPointWarp, Input.TexCoords.xy*TextureScale);
    float3 DecodeNormal=NormalSampleToWorldSpace(
        SampleNormalValue.xyz,
    normalize(Input.TangentToWorld2.xyz),
    Input.TangentToWorld0.xyz);
    

    GBufferdataOutput OutputGbufferData=(GBufferdataOutput)0;
    {
        OutputGbufferData.BaseColor=(RoughnessSampleValue.r+0.5f)*float3(0.8f,0.8f,0.8f)*BaseColorSample.xyz;
        OutputGbufferData.Metallic=Metallic;
        OutputGbufferData.Specular=Specular;
        OutputGbufferData.Roughness=(RoughnessSampleValue.r+0.5f)*BaseColorSample.a*Roughness;
        OutputGbufferData.Normal=DecodeNormal;
        OutputGbufferData.ShadingModel=1;
    }

    
    
    OutTargetA=float4(OutputGbufferData.Normal,1.0f);
    OutTargetB=float4(
        OutputGbufferData.Metallic,
        OutputGbufferData.Specular,
        OutputGbufferData.Roughness,
        OutputGbufferData.ShadingModel
        );
    OutTargetC=float4(OutputGbufferData.BaseColor,1.0f);
    OutTargetD=float4(Input.TestWorldPosition,1.0f);
}