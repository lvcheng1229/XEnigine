#include "MainInit.h"
#include "Runtime/CoreGObject/GObjtect/PermanentMemAlloc.h"

static std::vector<InitPropertyFunPtr>InitPropertyFunArray;

void MainInit::PushToInitPropertyFunArray(InitPropertyFunPtr FunPtrIn)
{
	InitPropertyFunArray.push_back(FunPtrIn);
}

void MainInit::Init()
{
	GlobalGObjectPermanentMemAlloc.AllocatePermanentMemPool(1 << 20);
	for (auto& t : InitPropertyFunArray)
	{
		(*t)(nullptr);
	}
}
