#pragma once

#include "UnitTest/d3dUtil.h"
#include "UnitTest/MathHelper.h"
#include "Runtime/D3D12RHI/D3D12Resource.h"

struct ObjectConstants
{
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
    DirectX::XMFLOAT3 BoundingBoxCenter;
    float padding0;
    DirectX::XMFLOAT3 BoundingBoxExtent;
    float padding1;
};



struct PassConstants
{
    DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
};

struct Vertex
{
    DirectX::XMFLOAT4 Position;
    DirectX::XMFLOAT3 TangentX;
	DirectX::XMFLOAT4 TangentY;
	DirectX::XMFLOAT2 TexCoord;
};

struct FullScreenVertex
{
    DirectX::XMFLOAT2 Pos;
    //DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 TexC;
};

struct FullScreenVertexOnlyPos
{
    DirectX::XMFLOAT2 Pos;
};

struct FrameResource
{
public:
    FrameResource() {};
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource() {};

    //std::shared_ptr<XRHIConstantBuffer>PassConstantBuffer;
    std::vector<std::shared_ptr<XRHIConstantBuffer>>MaterialConstantBuffer;
    std::vector<std::shared_ptr<XRHIConstantBuffer>>ObjectConstantBuffer;
};