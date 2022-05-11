RWTexture2D<uint>PhysicalShadowDepthTexture; // 1024 * 1024
Texture2D<uint4> PagetableInfos;

cbuffer LightProjectMatrix
{
    row_major float4x4 LightViewProjectMatrix;
    uint IndexX;
    uint IndexY;
    uint padding2;
    uint padding3;
}

#define PhysicalTileWidthNum 8
#define PhysicalSize 1024
#define TileSize 128



void PS(
    //nointerpolation uint2 IndexXY : ATTRIBUTE1,
    in float4 PositionIn: SV_POSITION,
    out float4 PlaceHolderTarget : SV_TARGET)
{
    if(PositionIn.z<0.0f || PositionIn.z>1.0f)
    {
        PlaceHolderTarget = float4(0.0,1,0,0);
        return;
    }

    //uint2 IndexXY = IndexXY;
    uint2 IndexXY = uint2(IndexX,IndexY);

    uint PageTableIndex = PagetableInfos[IndexXY].x;//TODO : Move To Vertex Shader
    uint PhysicalIndexX = PageTableIndex % uint(PhysicalTileWidthNum);
    uint PhysicalIndexY = PageTableIndex / uint(PhysicalTileWidthNum);
    
    float2 UVWtrite = PositionIn.xy / TileSize;

    float2 WritePos = float2(PhysicalIndexX,PhysicalIndexY) + UVWtrite;
    WritePos/=PhysicalTileWidthNum;
    WritePos*=PhysicalSize;

    uint UintDepth = PositionIn.z * (1<<32);
    InterlockedMax(PhysicalShadowDepthTexture[uint2(WritePos)],UintDepth);//TODO Need A Clear Pass
    ////PhysicalShadowDepthTexture[uint2(WritePos)]=UintDepth;
    //float TempValue = UintDepth + WritePos.x + WritePos.y;
    //TempValue*=0.0f; 
    PlaceHolderTarget = float4(IndexXY,PageTableIndex,PositionIn.z);
    return;
}