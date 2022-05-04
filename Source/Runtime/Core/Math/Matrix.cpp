#include "Matrix.h"
namespace DirectX
{
    namespace SimpleMath
    {
        LMatrix LMatrix::Transpose() const noexcept
        {
            using namespace DirectX;
            const XMMATRIX M = XMLoadFloat4x4(this);
            LMatrix R;
            XMStoreFloat4x4(&R, XMMatrixTranspose(M));
            return R;
        }
        LMatrix LMatrix::CreateScale(const Vector3& scales) noexcept
        {
            using namespace DirectX;
            LMatrix R;
            XMStoreFloat4x4(&R, XMMatrixScaling(scales.x, scales.y, scales.z));
            return R;
        }

        LMatrix LMatrix::CreateTranslation(const Vector3& position) noexcept
        {
            using namespace DirectX;
            LMatrix R;
            XMStoreFloat4x4(&R, XMMatrixTranslation(position.x, position.y, position.z));
            return R;
        }
    }
}