
Texture2D TextureSampledInput;
RWTexture2D<float> FurthestHZBOutput;
SamplerState gsamPointWarp  : register(s0,space1000);

cbuffer cbHZB
{
    float4 DispatchThreadIdToBufferUV;
}

[numthreads(16, 16, 1)]
void HZBBuildCS(
	uint2 GroupId : SV_GroupID,
	uint GroupThreadIndex : SV_GroupIndex,
    uint3 dispatchThreadId :SV_DispatchThreadID)
{
    float2 BufferUV=(dispatchThreadId.xy+0.5f)*DispatchThreadIdToBufferUV.xy;
    FurthestHZBOutput[dispatchThreadId.xy]=TextureSampledInput.SampleLevel(gsamPointWarp,BufferUV,0).r;
}    