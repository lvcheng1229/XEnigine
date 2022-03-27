#include "XMath.h"
using namespace DirectX;
static DirectX::XMFLOAT4X4 IdentityMatrix(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);


DirectX::XMFLOAT4X4 XDirectx::GetIdentityMatrix()
{
    return IdentityMatrix;
}

DirectX::XMMATRIX XM_CALLCONV XDirectx::XXMMatrixPerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ)
{
    assert(NearZ > 0.f && FarZ > 0.f);
    assert(!XMScalarNearEqual(FovAngleY, 0.0f, 0.00001f * 2.0f));
    assert(!XMScalarNearEqual(AspectRatio, 0.0f, 0.00001f));
    assert(!XMScalarNearEqual(FarZ, NearZ, 0.00001f));

#if defined(_XM_NO_INTRINSICS_)

    float    SinFov;
    float    CosFov;
    XMScalarSinCos(&SinFov, &CosFov, 0.5f * FovAngleY);

    float Height = CosFov / SinFov;
    float Width = Height / AspectRatio;
    float fRange = NearZ / (NearZ - FarZ);

    XMMATRIX M;
    M.m[0][0] = Width;
    M.m[0][1] = 0.0f;
    M.m[0][2] = 0.0f;
    M.m[0][3] = 0.0f;

    M.m[1][0] = 0.0f;
    M.m[1][1] = Height;
    M.m[1][2] = 0.0f;
    M.m[1][3] = 0.0f;

    M.m[2][0] = 0.0f;
    M.m[2][1] = 0.0f;
    M.m[2][2] = fRange;
    M.m[2][3] = 1.0f;

    M.m[3][0] = 0.0f;
    M.m[3][1] = 0.0f;
    M.m[3][2] = -fRange * FarZ;
    M.m[3][3] = 0.0f;
    return M;

#elif defined(_XM_ARM_NEON_INTRINSICS_)
    float    SinFov;
    float    CosFov;
    XMScalarSinCos(&SinFov, &CosFov, 0.5f * FovAngleY);

    float fRange = NearZ / (NearZ - FarZ);
    float Height = CosFov / SinFov;
    float Width = Height / AspectRatio;
    const XMVECTOR Zero = vdupq_n_f32(0);

    XMMATRIX M;
    M.r[0] = vsetq_lane_f32(Width, Zero, 0);
    M.r[1] = vsetq_lane_f32(Height, Zero, 1);
    M.r[2] = vsetq_lane_f32(fRange, g_XMIdentityR3.v, 2);
    M.r[3] = vsetq_lane_f32(-fRange * FarZ, Zero, 2);
    return M;
#elif defined(_XM_SSE_INTRINSICS_)
    float    SinFov;
    float    CosFov;
    XMScalarSinCos(&SinFov, &CosFov, 0.5f * FovAngleY);

    float fRange = NearZ / (NearZ - FarZ);
    // Note: This is recorded on the stack
    float Height = CosFov / SinFov;
    XMVECTOR rMem = {
        Height / AspectRatio,
        Height,
        fRange,
        -fRange * FarZ
    };
    // Copy from memory to SSE register
    XMVECTOR vValues = rMem;
    XMVECTOR vTemp = _mm_setzero_ps();
    // Copy x only
    vTemp = _mm_move_ss(vTemp, vValues);
    // CosFov / SinFov,0,0,0
    XMMATRIX M;
    M.r[0] = vTemp;
    // 0,Height / AspectRatio,0,0
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp, g_XMMaskY);
    M.r[1] = vTemp;
    // x=fRange,y=-fRange * NearZ,0,1.0f
    vTemp = _mm_setzero_ps();
    vValues = _mm_shuffle_ps(vValues, g_XMIdentityR3, _MM_SHUFFLE(3, 2, 3, 2));
    // 0,0,fRange,1.0f
    vTemp = _mm_shuffle_ps(vTemp, vValues, _MM_SHUFFLE(3, 0, 0, 0));
    M.r[2] = vTemp;
    // 0,0,-fRange * NearZ,0.0f
    vTemp = _mm_shuffle_ps(vTemp, vValues, _MM_SHUFFLE(2, 1, 0, 0));
    M.r[3] = vTemp;
    return M;
#endif
}

XMMATRIX XM_CALLCONV XDirectx::XXMMatrixPerspectiveOffCenterLH
(
    float ViewLeft,
    float ViewRight,
    float ViewBottom,
    float ViewTop,
    float NearZ,
    float FarZ
)
{
    assert(NearZ > 0.f && FarZ > 0.f);
    assert(!XMScalarNearEqual(ViewRight, ViewLeft, 0.00001f));
    assert(!XMScalarNearEqual(ViewTop, ViewBottom, 0.00001f));
    assert(!XMScalarNearEqual(FarZ, NearZ, 0.00001f));

#if defined(_XM_NO_INTRINSICS_)

    float TwoNearZ = NearZ + NearZ;
    float ReciprocalWidth = 1.0f / (ViewRight - ViewLeft);
    float ReciprocalHeight = 1.0f / (ViewTop - ViewBottom);
    float fRange = NearZ / (NearZ - FarZ);

    XMMATRIX M;
    M.m[0][0] = TwoNearZ * ReciprocalWidth;
    M.m[0][1] = 0.0f;
    M.m[0][2] = 0.0f;
    M.m[0][3] = 0.0f;

    M.m[1][0] = 0.0f;
    M.m[1][1] = TwoNearZ * ReciprocalHeight;
    M.m[1][2] = 0.0f;
    M.m[1][3] = 0.0f;

    M.m[2][0] = -(ViewLeft + ViewRight) * ReciprocalWidth;
    M.m[2][1] = -(ViewTop + ViewBottom) * ReciprocalHeight;
    M.m[2][2] = fRange;
    M.m[2][3] = 1.0f;

    M.m[3][0] = 0.0f;
    M.m[3][1] = 0.0f;
    M.m[3][2] = -fRange * FarZ;
    M.m[3][3] = 0.0f;
    return M;

#elif defined(_XM_ARM_NEON_INTRINSICS_)
    float TwoNearZ = NearZ + NearZ;
    float ReciprocalWidth = 1.0f / (ViewRight - ViewLeft);
    float ReciprocalHeight = 1.0f / (ViewTop - ViewBottom);
    float fRange = NearZ / (NearZ - FarZ);
    const XMVECTOR Zero = vdupq_n_f32(0);

    XMMATRIX M;
    M.r[0] = vsetq_lane_f32(TwoNearZ * ReciprocalWidth, Zero, 0);
    M.r[1] = vsetq_lane_f32(TwoNearZ * ReciprocalHeight, Zero, 1);
    M.r[2] = XMVectorSet(-(ViewLeft + ViewRight) * ReciprocalWidth,
        -(ViewTop + ViewBottom) * ReciprocalHeight,
        fRange,
        1.0f);
    M.r[3] = vsetq_lane_f32(-fRange * FarZ, Zero, 2);
    return M;
#elif defined(_XM_SSE_INTRINSICS_)
    XMMATRIX M;
    float TwoNearZ = NearZ + NearZ;
    float ReciprocalWidth = 1.0f / (ViewRight - ViewLeft);
    float ReciprocalHeight = 1.0f / (ViewTop - ViewBottom);
    float fRange = NearZ / (NearZ - FarZ);
    // Note: This is recorded on the stack
    XMVECTOR rMem = {
        TwoNearZ * ReciprocalWidth,
        TwoNearZ * ReciprocalHeight,
        -fRange * FarZ,
        0
    };
    // Copy from memory to SSE register
    XMVECTOR vValues = rMem;
    XMVECTOR vTemp = _mm_setzero_ps();
    // Copy x only
    vTemp = _mm_move_ss(vTemp, vValues);
    // TwoNearZ*ReciprocalWidth,0,0,0
    M.r[0] = vTemp;
    // 0,TwoNearZ*ReciprocalHeight,0,0
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp, g_XMMaskY);
    M.r[1] = vTemp;
    // 0,0,fRange,1.0f
    M.r[2] = XMVectorSet(-(ViewLeft + ViewRight) * ReciprocalWidth,
        -(ViewTop + ViewBottom) * ReciprocalHeight,
        fRange,
        1.0f);
    // 0,0,-fRange * NearZ,0.0f
    vValues = _mm_and_ps(vValues, g_XMMaskZ);
    M.r[3] = vValues;
    return M;
#endif
}

DirectX::XMMATRIX XM_CALLCONV XDirectx::XXMMatrixOrthographicOffCenterLH
(
    float ViewLeft,
    float ViewRight,
    float ViewBottom,
    float ViewTop,
    float NearZ,
    float FarZ
)
{
    assert(!XMScalarNearEqual(ViewRight, ViewLeft, 0.00001f));
    assert(!XMScalarNearEqual(ViewTop, ViewBottom, 0.00001f));
    assert(!XMScalarNearEqual(FarZ, NearZ, 0.00001f));

#if defined(_XM_NO_INTRINSICS_)

    float ReciprocalWidth = 1.0f / (ViewRight - ViewLeft);
    float ReciprocalHeight = 1.0f / (ViewTop - ViewBottom);
    float fRange = 1.0f / (NearZ - FarZ);

    XMMATRIX M;
    M.m[0][0] = ReciprocalWidth + ReciprocalWidth;
    M.m[0][1] = 0.0f;
    M.m[0][2] = 0.0f;
    M.m[0][3] = 0.0f;

    M.m[1][0] = 0.0f;
    M.m[1][1] = ReciprocalHeight + ReciprocalHeight;
    M.m[1][2] = 0.0f;
    M.m[1][3] = 0.0f;

    M.m[2][0] = 0.0f;
    M.m[2][1] = 0.0f;
    M.m[2][2] = fRange;
    M.m[2][3] = 0.0f;

    M.m[3][0] = -(ViewLeft + ViewRight) * ReciprocalWidth;
    M.m[3][1] = -(ViewTop + ViewBottom) * ReciprocalHeight;
    M.m[3][2] = -fRange * FarZ;
    M.m[3][3] = 1.0f;
    return M;

#elif defined(_XM_ARM_NEON_INTRINSICS_)
    float ReciprocalWidth = 1.0f / (ViewRight - ViewLeft);
    float ReciprocalHeight = 1.0f / (ViewTop - ViewBottom);
    float fRange = 1.0f / (NearZ - FarZ);
    const XMVECTOR Zero = vdupq_n_f32(0);
    XMMATRIX M;
    M.r[0] = vsetq_lane_f32(ReciprocalWidth + ReciprocalWidth, Zero, 0);
    M.r[1] = vsetq_lane_f32(ReciprocalHeight + ReciprocalHeight, Zero, 1);
    M.r[2] = vsetq_lane_f32(fRange, Zero, 2);
    M.r[3] = XMVectorSet(-(ViewLeft + ViewRight) * ReciprocalWidth,
        -(ViewTop + ViewBottom) * ReciprocalHeight,
        -fRange * FarZ,
        1.0f);
    return M;
#elif defined(_XM_SSE_INTRINSICS_)
    XMMATRIX M;
    float fReciprocalWidth = 1.0f / (ViewRight - ViewLeft);
    float fReciprocalHeight = 1.0f / (ViewTop - ViewBottom);
    float fRange = 1.0f / (NearZ - FarZ);
    // Note: This is recorded on the stack
    XMVECTOR rMem = {
        fReciprocalWidth,
        fReciprocalHeight,
        fRange,
        1.0f
    };
    XMVECTOR rMem2 = {
        -(ViewLeft + ViewRight),
        -(ViewTop + ViewBottom),
        -FarZ,
        1.0f
    };
    // Copy from memory to SSE register
    XMVECTOR vValues = rMem;
    XMVECTOR vTemp = _mm_setzero_ps();
    // Copy x only
    vTemp = _mm_move_ss(vTemp, vValues);
    // fReciprocalWidth*2,0,0,0
    vTemp = _mm_add_ss(vTemp, vTemp);
    M.r[0] = vTemp;
    // 0,fReciprocalHeight*2,0,0
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp, g_XMMaskY);
    vTemp = _mm_add_ps(vTemp, vTemp);
    M.r[1] = vTemp;
    // 0,0,fRange,0.0f
    vTemp = vValues;
    vTemp = _mm_and_ps(vTemp, g_XMMaskZ);
    M.r[2] = vTemp;
    // -(ViewLeft + ViewRight)*fReciprocalWidth,-(ViewTop + ViewBottom)*fReciprocalHeight,fRange*-NearZ,1.0f
    vValues = _mm_mul_ps(vValues, rMem2);
    M.r[3] = vValues;
    return M;
#endif
}
