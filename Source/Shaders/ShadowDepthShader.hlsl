cbuffer cbPerObject
{
    float4x4 gWorld;
}

cbuffer cbPass
{
    float4x4 gView;
    float4x4 gProj;
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
    float4 PositionW=mul(Input.Position, gWorld);
    float4 PositionV=mul(PositionW, gView);
    Position=mul(PositionV, gProj);
}

void PS()
{

}