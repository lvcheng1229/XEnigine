#pragma once


//NOTE!!!!!!!!!!!!!!!!
//Removed TO Runtime/Core/Template

#include "Runtime/HAL/PlatformTypes.h"
#include <type_traits>
template <typename T>
inline constexpr T AlignArbitrary(T Val, uint64 Alignment)
{
	static_assert(std::is_integral_v<T> || std::is_pointer_v<T>, "AlignArbitrary expects an integer or pointer type");
	return (T)((((uint64)Val + Alignment - 1) / Alignment) * Alignment);
}