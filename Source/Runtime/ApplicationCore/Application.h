#pragma once
#include "Runtime/HAL/PlatformTypes.h"
#include "Runtime/ApplicationCore/ApplicationInput.h"


class XApplication
{
	friend class D3DApp;
public:
	uint32 ClientWidth;
	uint32 ClientHeight;
	static XApplication* APPPtr;
	XApplication();
	virtual ~XApplication();
protected:
	virtual void PreInitial();
	virtual bool CreateAppWindow();
	virtual void* GetPlatformHandle();
};