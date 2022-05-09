cbuffer cbPerObject
{
    row_major float4x4 gWorld;
    float3 BoundingBoxCenter;
    float padding0;
    float3 BoundingBoxExtent;
    float padding1;
}

cbuffer cbPass
{
    //float4x4 gView;
    row_major float4x4 gViewProj;
}

struct FVertexFactoryInput
{
    float4	Position	: ATTRIBUTE0;
    float3	TangentX	: ATTRIBUTE1;
    float4	TangentZ	: ATTRIBUTE2;
    float2	TexCoord    : ATTRIBUTE3;
};



void VS(FVertexFactoryInput Input,
    out float4 Position : SV_POSITION)
{
    //float4 PositionW=mul_x(Input.Position, gWorld);
    //float4 PositionW=mul(gWorld,Input.Position);
    float4 PositionW=mul(Input.Position,gWorld);
    //float4 PositionV=mul_x(float4(PositionW.xyz,1.0), gView);
    //Position=mul_x(float4(PositionW.xyz,1.0), gViewProj);
    Position=mul(float4(PositionW.xyz,1.0),gViewProj);
}

void PS()
{

}