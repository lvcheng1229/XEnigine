cbuffer cbPerObject
{
    row_major float4x4 gWorld;
    float3 BoundingBoxCenter;
    float padding0;
    float3 BoundingBoxExtent;
    float padding1;
}

cbuffer LightProjectMatrix
{
    row_major float4x4 LightViewProjectMatrix;
    uint IndexX;
    uint IndexY;
    uint padding2;
    uint padding3;
}

struct FVertexFactoryInput
{
    float4	Position	: ATTRIBUTE0;
};


void VS(FVertexFactoryInput Input,
    //out nointerpolation uint2 IndexXY : ATTRIBUTE1,
    out float4 Position : SV_POSITION)
{
    float4 PositionW=mul(Input.Position, gWorld);
    Position=mul(PositionW, LightViewProjectMatrix);
    //IndexXY = uint2(IndexX,IndexY);
}


