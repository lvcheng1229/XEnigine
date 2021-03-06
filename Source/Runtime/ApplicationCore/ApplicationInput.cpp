#include "ApplicationInput.h"
#include <memory>
XAppInput::XAppInput()
{
	MouseDelta = { 0,0 };
	MouseX = MouseY = 0;
	bMousePressed[0] = bMousePressed[1] = bMousePressed[2] = false;
	memset(bBoardKeyPressed, 0, sizeof(bBoardKeyPressed));
}

#include <iostream>
void XAppInput::OnRMouseDown(int32 X, int32 Y)
{
	this->SetMousePos(X, Y);
	bMousePressed[(int32)EInputKey::MOUSE_RIGHT] = true;
}

void XAppInput::OnLMouseDown(int32 X, int32 Y)
{
	this->SetMousePos(X, Y);
	bMousePressed[(int32)EInputKey::MOUSE_LEFT] = true;
}

void XAppInput::OnRMouseUp(int32 X, int32 Y)
{
	bMousePressed[(int32)EInputKey::MOUSE_RIGHT] = false;
}

void XAppInput::OnMouseMove(int32 X, int32 Y)
{
	MouseDelta = { X - MouseX,Y - MouseY };
	this->SetMousePos(X, Y);
}

void XAppInput::OnKeyDown(EInputKey InputKey)
{
	bBoardKeyPressed[int32(InputKey)] = true;
}

void XAppInput::OnKeyUp(EInputKey InputKey)
{
	bBoardKeyPressed[int32(InputKey)] = false;
}

void XAppInput::InputMsgProcsss(EInputType InputType, EInputEvent InputEvent, EInputKey InputKey, int32 PosX, int32 PosY, int32 WHEEL)
{
	if (InputType == EInputType::MOUSE)
	{
		if (InputEvent == EInputEvent::DOWN)
		{
			if (InputKey == EInputKey::MOUSE_RIGHT)
			{
				XAppInput::InputPtr->OnRMouseDown(PosX, PosY);
			}
			else if (InputKey == EInputKey::MOUSE_LEFT)
			{
				XAppInput::InputPtr->OnLMouseDown(PosX, PosY);
			}
		}
		else if(InputEvent == EInputEvent::UP)
		{
			if (InputKey == EInputKey::MOUSE_RIGHT)
			{
				XAppInput::InputPtr->OnRMouseUp(PosX, PosY);
			}
		}
		else if (InputEvent == EInputEvent::MOUSE_MOVE)
		{
			XAppInput::InputPtr->OnMouseMove(PosX, PosY);
		}
	}
	else if(InputType == EInputType::KEYBOARD)
	{
		if (InputEvent == EInputEvent::DOWN)
		{
			XAppInput::InputPtr->OnKeyDown(InputKey);
		}
		else if (InputEvent == EInputEvent::UP)
		{
			XAppInput::InputPtr->OnKeyUp(InputKey);
		}
	}

	TempUpdate(TempUpFun);
}

