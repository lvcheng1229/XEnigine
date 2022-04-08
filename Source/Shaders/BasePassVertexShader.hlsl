

cbuffer cbPerObject
{
    float4x4 gWorld;
}

cbuffer cbPass
{
    float4x4 gViewProj;
}


//#include "Mapping/VertexFactory.hlsl"
#include "LocalVertexFactory.hlsl"

void VS(FVertexFactoryInput Input,
    out FVertexFactoryInterpolantsVSToPS Output,
    out float4 Position : SV_POSITION
    )
{
    VsToPsCompute(Input,Output,Position);
}