#pragma once
#include "Runtime/HAL/PlatformTypes.h"
#include "Runtime/ApplicationCore/ApplicationInput.h"

class XApplication
{
public:
	uint32 ClientWidth;
	uint32 ClientHeight;
	XAppInput AppInput;
	static XApplication* APPPtr;
	XApplication();
	virtual ~XApplication();
	virtual void* GetPlatformHandle();

	virtual bool CreateAppWindow();
	virtual bool InitRHI();

	virtual bool UISetup() = 0;
	virtual bool UINewFrame() = 0;
};