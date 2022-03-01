//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
Texture2D<float4>    gDiffuseMap : register(t0);

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj; 
};

//SamplerState MeshTextureSampler
//{
//    Filter = MIN_MAG_MIP_LINEAR;
//    AddressU = Wrap;
//    AddressV = Wrap;
//};

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

struct PSInput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float2 TexC : TEXCOORD;
};

PSInput VSMain(float3 position : POSITION, float4 color : COLOR,float2 uv : TEXCOORD)
{
    PSInput result;

    result.position =mul(float4(position,1.0), gWorldViewProj);
    result.color = color;
    result.TexC=uv;
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    
    float4 color_o = gDiffuseMap.Sample(gsamPointWrap, input.TexC)*float4(input.color,1.0);
    //float4 color_o = float4(input.TexC, 0.0, 1.0);
    //float4 color_o = float4(input.color,1.0);
    return float4(color_o.xyz,1.0);
}