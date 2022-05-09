Texture2D<uint> VirtualSMFlags;
RWTexture2D<uint4> PagetableInfos;


//64 / 32 = 2
static groupshared uint Index = 0;
[numthreads(32, 32, 1)]
void VSMPageTableGenCS(uint2 DispatchThreadID :SV_DispatchThreadID)
{
    for(uint i = 0 ;i < 2 ; i++)
    {
        for(uint j = 0 ; j < 2; j++)
        {
            //uint2 FlagsXY = DispatchThreadID.xy * 4 + uint2(i,j);
            uint2 FlagsXY = DispatchThreadID.xy + uint2(i * 32 , j *32);
            uint ValueOut = 0;
            if(VirtualSMFlags[FlagsXY] == 1)
            {
                InterlockedAdd(Index,1,ValueOut);
            }
            PagetableInfos[FlagsXY] = uint4(ValueOut,0,0,0);
        }
    }
}