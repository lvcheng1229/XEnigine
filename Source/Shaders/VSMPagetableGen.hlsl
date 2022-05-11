Texture2D<uint> VirtualSMFlags;
RWTexture2D<uint4> PagetableInfos;

//64 / 16 = 4
static groupshared uint Index = 0;
[numthreads(16, 16, 1)]
void VSMPageTableGenCS(uint2 DispatchThreadID :SV_DispatchThreadID)
{
    for(uint i = 0 ;i < 4 ; i++)
    {
        for(uint j = 0 ; j < 4; j++)
        {
            uint2 FlagsXY = DispatchThreadID.xy * 4 + uint2(i,j);
            uint ValueOut = 0;
            if(VirtualSMFlags[FlagsXY] == 1)
            {
                InterlockedAdd(Index,1,ValueOut);
            }
            PagetableInfos[FlagsXY] = uint4(ValueOut,0,0,0);
            AllMemoryBarrierWithGroupSync();
        }
    }
}