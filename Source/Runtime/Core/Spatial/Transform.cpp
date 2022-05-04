#include "Transform.h"

XTransform::XTransform()
{
	bNeedRecombine = true;
	Translation = XVector3::Zero;
	Scale = XVector3::One;
	CombinedMatrix = XMatrix::Identity;
}

void XTransform::Combine()
{
	if (bNeedRecombine)
	{
		CombinedMatrix = XMatrix::CreateScale(Scale) * XMatrix::CreateTranslation(Translation);
		bNeedRecombine = false;
	}
}
