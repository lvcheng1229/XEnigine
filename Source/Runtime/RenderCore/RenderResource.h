#pragma once
#include "Runtime/HAL/PlatformTypes.h"
#include <vector>
class XRenderResource
{
public:
	XRenderResource() : ListIndex(-1) {}

	template<typename FunctionType>
	static void ForAllResources(const FunctionType& Function)
	{
		const std::vector<XRenderResource*>ResourceArray = GetResourceArray();
		for (int Index = 0; Index < ResourceArray.size(); Index++)
		{
			XRenderResource* Resource = ResourceArray[Index];
			if (Resource)
			{
				Function(Resource);
			}
		}
	}

	static void InitRHIForAllResources()
	{
		ForAllResources([](XRenderResource* Resource) { Resource->InitRHI(); });
	}

	virtual void InitResource();
	virtual void ReleaseResource() {};
	virtual void InitRHI() {}
	virtual void ReleaseRHI() {}
private:
	static std::vector<XRenderResource*>& GetResourceArray();
	int32 ListIndex;
};

extern void BeginInitResource(XRenderResource* Resource);

template<typename ResourceClass>
class TGlobalResource : public ResourceClass
{
public:
	TGlobalResource()
	{
		InitGlobalResource();
	}
private:
private:


	void InitGlobalResource()
	{
		//if (IsInRenderingThread())
		//{
		//	((ResourceType*)this)->InitResource();
		//}
		//else
		{
			BeginInitResource((ResourceClass*)this);
		}
	}
};