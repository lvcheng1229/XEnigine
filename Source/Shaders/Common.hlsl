cbuffer cbView
{
    float4x4 View_ScreenToTranslatedWorld;
    float4 View_InvDeviceZToWorldZTransform;
    float3 View_WorldCameraOrigin;
    float padding0;
    float4 View_BufferSizeAndInvSize;//Texture Size
    //float4 View_ScreenPositionScaleBias;//match Buffer size to ScreenSize
};

#define PI 3.141592653589

SamplerState gsamPointWarp  : register(s0,space1000);
SamplerState gsamLinearWarp  : register(s4,space1000);

float Pow5( float x )
{
	float xx = x*x;
	return xx * xx * x;
}

float ConvertFromDeviceZ(float DeviceZ)
{
	return DeviceZ * View_InvDeviceZToWorldZTransform[0] + View_InvDeviceZToWorldZTransform[1] + 1.0f / (DeviceZ * View_InvDeviceZToWorldZTransform[2] - View_InvDeviceZToWorldZTransform[3]);
}