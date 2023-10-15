#pragma once
#include "Runtime/HAL/PlatformTypes.h"
#include <functional>

#include <type_traits>
template <typename T>
inline constexpr T AlignArbitrary(T Val, uint64 Alignment)
{
	static_assert(std::is_integral_v<T> || std::is_pointer_v<T>, "AlignArbitrary expects an integer or pointer type");
	return (T)((((uint64)Val + Alignment - 1) / Alignment) * Alignment);
}

template<typename T>
class XArrayView
{
public:
	XArrayView(const T* ptr_in, uint32 size_in) :ptr(ptr_in), size_v(size_in) {}
	const T* data()const { return ptr; }
	uint32 size()const { return size_v; }
private:
	const T* ptr;
	uint32 size_v;
};

template <class T>
inline void THashCombine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename Enum>
constexpr bool EnumHasAllFlags(Enum Flags, Enum Contains)
{
	return (((__underlying_type(Enum))Flags) & (__underlying_type(Enum))Contains) == ((__underlying_type(Enum))Contains);
}

template<typename Enum>
constexpr bool EnumHasAnyFlags(Enum Flags, Enum Contains)
{
	return (((__underlying_type(Enum))Flags) & (__underlying_type(Enum))Contains) != 0;
}

