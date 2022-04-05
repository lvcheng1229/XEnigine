#pragma once
#include "Runtime/HAL/PlatformTypes.h"
class FResourceArrayInterface
{
public:
	virtual ~FResourceArrayInterface() {}
	virtual const void* GetResourceData() const = 0;
	virtual uint32 GetResourceDataSize() const = 0;
	virtual void ReleaseData() = 0;
};