#pragma once
#include "Runtime/HAL/PlatformTypes.h"
#include <functional>
//https://zhuanlan.zhihu.com/p/21722362
//template<typename T>
//using UnderlyingType = std::underlying_type<T>::type;


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