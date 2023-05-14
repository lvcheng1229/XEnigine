float2 positions[3] = {
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
};

float3 colors[3] = {
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0)
};

void VS(
    in uint VID : SV_VertexID,
    out float4 fragColor : TEXCOORD0,
    out float4 Position : SV_POSITION
    )
{
    Position = float4(positions[VID], 0.0, 1.0);
    fragColor = float4(colors[VID],1.0);
}

void PS(
	in float4 fragColor		: TEXCOORD0,
	out float4 OutColor		: SV_Target0
) 
{
    OutColor = float4(fragColor);
}
//dxc G:\XEngineF\XEnigine\Source\Shaders\VulkanShaderTest\hlsl2spirtest.hlsl -E VS -T vs_6_1 -Zi -Qstrip_reflect -spirv -fspv-target-env=vulkan1.1