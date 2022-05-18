#include "Application.h"
XAppInput* XAppInput::InputPtr = nullptr;

XApplication::XApplication()
{
	XAppInput::InputPtr = new XAppInput();

	ClientWidth = 1540;
	ClientHeight = 845;
}

XApplication::~XApplication()
{
	delete XAppInput::InputPtr;
}

void XApplication::PreInitial()
{

}

bool XApplication::CreateAppWindow()
{
	return false;
}

bool XApplication::InitRHI()
{
	return false;
}

void* XApplication::GetPlatformHandle()
{
	return nullptr;
}


