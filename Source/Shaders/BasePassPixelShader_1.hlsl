//cbuffer cbPerObject
//{
//    float4x4 gWorld;
//}
//
//cbuffer cbPass
//{
//    float4x4 gViewProj;
//}
//
////Local Vertex Factory , if create Landscape , then this file chnage to LandSacpeVertextFactory.ush
//struct FVertexFactoryInput
//{
//    float4	Position	: ATTRIBUTE0;
//    float3	TangentX	: ATTRIBUTE1;
//    float4	TangentY	: ATTRIBUTE2;
//    float2	TexCoord    : ATTRIBUTE3;
//};

struct FVertexFactoryInterpolantsVSToPS
{
	float4 TangentToWorld0 : TEXCOORD10; 
    float4 TangentToWorld1 : TEXCOORD11;
	float4 TexCoords : TEXCOORD0;
    float3 TestWorldPosition : TEXCOORD1;
};


//void VS(FVertexFactoryInput Input,
//    out FVertexFactoryInterpolantsVSToPS Output,
//    out float4 Position : SV_POSITION
//    )
//{
//    Output.TangentToWorld0 = float4(mul(Input.TangentX, (float3x3)gWorld),0.0f);
//    Output.TangentToWorld1 = float4(mul(Input.TangentY.xyz, (float3x3)gWorld),1.0f);
//    Output.TexCoords=float4(Input.TexCoord.xy,0.0f,0.0f);
//    Output.TestWorldPosition= mul(Input.Position,gWorld).xyz;
//    float4 PositionW=mul(Input.Position, gWorld);
//    Position=mul(PositionW, gViewProj);
//}


#include "Generated/Material.hlsl"

void PS(FVertexFactoryInterpolantsVSToPS Input,
    out float4 OutTargetA : SV_Target0,
    out float4 OutTargetB : SV_Target1,
    out float4 OutTargetC : SV_Target2,
    out float4 OutTargetD : SV_Target3
    )
{
    ParametersIn Parameters=(ParametersIn)0;
    Parameters.TexCoords = Input.TexCoords.xy;
    Parameters.TangentToWorld0 = Input.TangentToWorld0.xyz;
    Parameters.TangentToWorld1 = Input.TangentToWorld1.xyz;

    GBufferdataOutput OutputGbufferData=(GBufferdataOutput)0;
    CalcMaterialParameters(Parameters,OutputGbufferData);
    
    OutTargetA=float4(OutputGbufferData.Normal,1.0f);
    OutTargetB=float4(OutputGbufferData.Metallic,0,OutputGbufferData.Roughness,OutputGbufferData.ShadingModel);
    OutTargetC=float4(OutputGbufferData.BaseColor,1.0f);
    OutTargetD=float4(Input.TestWorldPosition,1.0f);
}