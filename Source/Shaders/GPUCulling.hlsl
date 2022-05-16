#include "Math.hlsl"
#define ThreadBlockSize 128

struct DepthIndirectCommand
{
    uint2 CbWorldAddress;
    //uint2 CbGlobalShadowViewProjectAddress; // Used For VSM
    
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
    uint Padding;//fuck!!!!! sizeof(DepthIndirectCommand) == 64
};

cbuffer cbCullingParameters
{
    float4x4 ShadowViewProject;
    float4 Planes[6];
    float commandCount; 
}

struct SceneConstantBuffer
{
    float4x4 gWorld;
    float3 BoundingBoxCenter;
    float a;
    float3 BoundingBoxExtent;
    float b;
};

StructuredBuffer<SceneConstantBuffer> SceneConstantBufferIN;
StructuredBuffer<DepthIndirectCommand> inputCommands;            //: register(t1);    // SRV: Indirect commands
AppendStructuredBuffer<DepthIndirectCommand> outputCommands;    //: register(u0);    // UAV: Processed indirect commands

bool BoundingBoxInPlane(float3 BoundingBoxCenter,float3 BoundingBoxExtent , float4 Plane)
{
    float3 MinBoundingBox = BoundingBoxCenter - BoundingBoxExtent;
    float3 MaxBoundingBox = BoundingBoxCenter + BoundingBoxExtent;
    
    float3 MinTemp,MaxTemp;

    if(Plane.x>0)
    {
        MinTemp.x = MinBoundingBox.x;
        //MaxTemp.x = MaxBoundingBox.x;
    }
    else
    {
        MinTemp.x = MaxBoundingBox.x;
        //MaxTemp.x = MinBoundingBox.x;
    }

    
    if(Plane.y>0)
    {
        MinTemp.y = MinBoundingBox.y;
        //MaxTemp.y = MaxBoundingBox.y;
    }
    else
    {
        MinTemp.y = MaxBoundingBox.y;
        //MaxTemp.y = MinBoundingBox.y;
    }


    if(Plane.z>0)
    {
        MinTemp.z = MinBoundingBox.z;
        //MaxTemp.z = MaxBoundingBox.z;
    }
    else
    {
        MinTemp.z = MaxBoundingBox.z;
        //MaxTemp.z = MinBoundingBox.z;
    }

    float D = dot(Plane.xyz,MinTemp) + Plane.w;
    
    if(D>0.0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

[numthreads(ThreadBlockSize, 1, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    uint index = (groupId.x * ThreadBlockSize) + groupIndex;
    if (index < commandCount)
    {
        bool InFrustum = true;
        for(int i=0; i<6; i++)
        {
            InFrustum = InFrustum & BoundingBoxInPlane(SceneConstantBufferIN[index].BoundingBoxCenter,SceneConstantBufferIN[index].BoundingBoxExtent,Planes[i]);
        }

        if(InFrustum == true)
        {
            outputCommands.Append(inputCommands[index]);
        }
    }
}