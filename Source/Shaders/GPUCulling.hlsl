
#define ThreadBlockSize 128

struct DepthIndirectCommand
{
    uint2 CbWorldAddress;
    
    uint2 VertexBufferLoacation;
    uint VertexSizeInBytes;
    uint VertexStrideInBytes;
    
    uint2 IndexBufferLoacation;
    uint IndexSizeInBytes;
    uint IndexFormat;

    uint IndexCountPerInstance;
    uint InstanceCount;
    uint StartIndexLocation;
    int BaseVertexLocation;
    
    uint StartInstanceLocation;
};

cbuffer cbCullingParameters
{
    float commandCount; 
}

StructuredBuffer<DepthIndirectCommand> inputCommands;            //: register(t1);    // SRV: Indirect commands
AppendStructuredBuffer<DepthIndirectCommand> outputCommands;    //: register(u0);    // UAV: Processed indirect commands

[numthreads(ThreadBlockSize, 1, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    uint index = (groupId.x * ThreadBlockSize) + groupIndex;
    if (index < commandCount)
    {
        outputCommands.Append(inputCommands[index]);
    }
}