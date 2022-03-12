#pragma once
#include <DirectXMath.h>

namespace XDirectx
{
	DirectX::XMFLOAT4X4 GetIdentityMatrix();

    DirectX::XMMATRIX XM_CALLCONV XXMMatrixPerspectiveFovLH
    (
        float FovAngleY,
        float AspectRatio,
        float NearZ,
        float FarZ
    );
    DirectX::XMMATRIX XM_CALLCONV XXMMatrixPerspectiveOffCenterLH
    (
        float ViewLeft,
        float ViewRight,
        float ViewBottom,
        float ViewTop,
        float NearZ,
        float FarZ
    );
    DirectX::XMMATRIX XM_CALLCONV XXMMatrixOrthographicOffCenterLH
    (
        float ViewLeft,
        float ViewRight,
        float ViewBottom,
        float ViewTop,
        float NearZ,
        float FarZ
    );
   

}