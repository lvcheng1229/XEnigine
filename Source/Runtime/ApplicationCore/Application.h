#pragma once
#include "Runtime/HAL/PlatformTypes.h"
#include "Runtime/ApplicationCore/ApplicationInput.h"


class XApplication
{
	friend class TestMain;
public:
	uint32 ClientWidth;
	uint32 ClientHeight;
	static XApplication* APPPtr;
	XApplication();
	virtual ~XApplication();
	virtual void* GetPlatformHandle();
protected:
	virtual void PreInitial();
	virtual bool CreateAppWindow();
	virtual bool InitRHI();

	virtual bool UISetup() = 0;
	virtual bool UINewFrame() = 0;
};