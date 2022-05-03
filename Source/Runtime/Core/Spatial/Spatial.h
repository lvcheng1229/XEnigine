#pragma once
#include <vector>
#include "Runtime/CoreGObject/GObjtect/Object.h"
#include "Runtime/Core/Math/Math.h"

class GSpatial :public GObject
{
protected:
	GSpatial* ParentSpatial;
	XBoundingBox BoundingBox;
public:
};

class GNode :public  GSpatial
{
protected:
	std::vector<GSpatial*> ChildSpatial;
};