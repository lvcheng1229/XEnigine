cbuffer cbPerObject
{
    float4x4 gWorld;
}

#include "common.hlsl"

struct FVertexFactoryInput
{
    float4	Position	: ATTRIBUTE0;
};


void VS(FVertexFactoryInput Input,
    out float4 Position : SV_POSITION)
{
    float4 PositionW=mul(Input.Position, gWorld);
    Position=mul(PositionW, View_ViewPorjectionMatrix);
}

void PS()
{

}
    