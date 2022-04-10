#pragma once

#define X_STR(x) X_STR_IMPL(x)
#define X_STR_IMPL(x) #x

#define X_TEXT(x) L##x

#ifdef X_PLATFORM_WIN

#include <windows.h>

    #ifndef ThrowIfFailed
    #define ThrowIfFailed(x)                                              \
    {                                                                     \
        HRESULT hr__ = (x);                                               \
        if(FAILED(hr__)) { __debugbreak(); } \
    }
    #endif
    
    #define X_Assert(condition , ...){if (!(condition)) { __debugbreak(); }}

    #ifdef X_RHI_DX12
    #include <wrl.h>
    #include <dxgi1_4.h>
    #include <d3d12.h>
    template<typename T>
    using XDxRefCount = Microsoft::WRL::ComPtr<T>;
    #endif

#endif
    
#include <iostream>
#define XLog(x) {std::cout << (x) << std::endl; }

#include <type_traits>
template<typename T>
requires(!std::is_lvalue_reference_v<T>)
T* GetRValuePtr(T&& v) 
{
    return &v;
}





