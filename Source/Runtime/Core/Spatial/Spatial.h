#pragma once
#include <vector>
#include "Runtime/CoreGObject/GObjtect/Object.h"
#include "Runtime/Core/Math/Math.h"
#include "Runtime/Core/Spatial/Transform.h"

class GSpatial :public GObject
{
protected:
	XTransform WorldTransform;
	GSpatial* ParentSpatial;
	XBoundingBox BoundingBox;
public:
	inline XTransform& GetWorldTransform()
	{
		return WorldTransform;
	}

	inline void SetWorldTranslate(const XVector3& TranslationIn)
	{
		WorldTransform.SetTranslation(TranslationIn);
	}

	inline void SetWorldScale(const XVector3& WorldScale)
	{
		WorldTransform.SetScale(WorldScale);
	}
};

class GNode :public  GSpatial
{
protected:
	std::vector<GSpatial*> ChildSpatial;
};