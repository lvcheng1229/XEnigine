#pragma once
#include "Runtime/HAL/PlatformTypes.h"
enum class EInputType
{
	KEYBOARD,
	MOUSE,
	MAX
};

enum class EInputEvent
{
	DOWN,
	UP,
	MOUSE_MOVE,
	WHEEL_MOVE,
	MAX
};

enum class EInputKey
{
	MOUSE_RIGHT = 0,
	MOUSE_LEFT,
	MOUSE_MIDDLE,
	MOUSE_MAX,

	ESCAPE,
	KEY_MAX
};

struct XMousePos
{
	int32 X;
	int32 Y;
};

class XAppInput
{
	friend class XApplication;
public:
	static XAppInput* InputPtr;
	XAppInput();
	inline int32 GetMouseX() { return MouseX; };
	inline int32 GetMouseY() { return MouseY; };
	inline XMousePos& GetMouseDelta() { return MouseDelta; }
	inline bool GetMousePressed(EInputKey InputKey) { return bMousePressed[int32(InputKey)]; }
	static void InputMsgProcsss(EInputType InputType, EInputEvent InputEvent, EInputKey InputKey, int32 PosX, int32 PosY, int32 WHEEL);
	
	typedef void (*FunPtr)();
	static void TempUpdate(FunPtr fun) { (*fun)(); }
	static FunPtr TempUpFun;
private:
	inline void SetMousePos(int32 X, int32 Y) { MouseX = X; MouseY = Y; }
	void OnRMouseDown(int32 X, int32 Y);
	void OnLMouseDown(int32 X, int32 Y);
	void OnRMouseUp(int32 X, int32 Y);
	void OnMouseMove(int32 X, int32 Y);
	
	bool bMousePressed[int32(EInputKey::KEY_MAX)];
	XMousePos MouseDelta;
	int32 MouseX;
	int32 MouseY;
};