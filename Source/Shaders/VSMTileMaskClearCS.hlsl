RWTexture2D<uint> VirtualSMFlags;

[numthreads(16, 16, 1)]
void VSMTileMaskClearCS(uint2 DispatchThreadID :SV_DispatchThreadID)
{
    VirtualSMFlags[DispatchThreadID] = 0;
}
