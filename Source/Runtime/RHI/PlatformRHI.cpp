#include "PlatformRHI.h"

XPlatformRHI* GPlatformRHI = nullptr;

void RHIInit()
{
	if (GPlatformRHI == nullptr)
	{
		GPlatformRHI = PlatformCreateDynamicRHI();
	}
}
