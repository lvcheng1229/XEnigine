#pragma once
#include "Runtime/Core/XMath.h"
#include "Runtime/Core/Math/Math.h"
#include "Runtime/RHI/RHIResource.h"
XVector4 CreateInvDeviceZToWorldZTransform(const XMatrix ProjMatrix);

struct XViewMatrices
{
	//3D Game Programming With DirectX12 Page163

	//Local - > World -> Translated World -> View -> Clip Space -> Screen
	//World -> Translated : World Translation
	//Translated World -> View : TranslatedViewMatrix
	//World -> View : ViewMatrix
	//View -> Clip ProjectionMatrix
public:
	void Create(
		const XMatrix& ProjectionMatrixIn,
		const XVector3& ViewLocation,
		const XVector3& ViewTargetPosition);
private:
	//TODO Align
	XVector3		ViewOrigin;
	XVector3		PreViewTranslation;

	XMatrix		ProjectionMatrix;
	XMatrix		ViewMatrix;
	XMatrix		ViewProjectionMatrix;
	XMatrix		TranslatedViewMatrix;
	XMatrix		TranslatedViewProjectionMatrix;
	XMatrix		TranslatedWorldToClip;

	//Transpose
	//XMatrix		ProjectionMatrixTranspose;
	//XMatrix		ViewMatrixTranspose;
	//XMatrix		ViewProjectionMatrixTranspose;
	//XMatrix		TranslatedViewMatrixTranspose;
	//XMatrix		TranslatedViewProjectionMatrixTranspose;

	//Inverse
	//DirectX::XMFLOAT4X4		ProjectionMatrixInverse;
	//DirectX::XMFLOAT4X4		ViewMatrixInverse;
	XMatrix		ViewProjectionMatrixInverse;
	//DirectX::XMFLOAT4X4		TranslatedViewMatrixInverse;
	XMatrix		TranslatedViewProjectionMatrixInverse;

	//misc
	//DirectX::XMFLOAT4X4		ScreenToTranslatedWorld;
public:
	void UpdateViewMatrix(const XVector3& ViewLocation, const XVector3& ViewTargetPosition);

//TODO
//#define GetMatrix_COMMON_TRANSPOSE_INVERSE(MAT_NAME)\
//	inline const DirectX::XMFLOAT4X4& GetMAT_NAME()const { return MAT_NAME; };\
//	inline const DirectX::XMFLOAT4X4& GetMAT_NAMETranspose()const { return MAT_NAMETranspose; };\
//	inline const DirectX::XMFLOAT4X4& GetMAT_NAMEInverse()const { return MAT_NAMEInverse; };\

	inline const XVector3& GetViewOrigin()const { return ViewOrigin; };
	inline const XVector3& GetPreViewTranslation()const { return PreViewTranslation; };

	//common
	inline const XMatrix& GetProjectionMatrix()const				{ return ProjectionMatrix; };
	inline const XMatrix& GetViewMatrix()const						{ return ViewMatrix; };
	inline const XMatrix& GetViewProjectionMatrix()const			{ return ViewProjectionMatrix; };
	inline const XMatrix& GetTranslatedViewMatrix()const			{ return TranslatedViewMatrix; };
	inline const XMatrix& GetTranslatedViewProjectionMatrix()const	{ return TranslatedViewProjectionMatrix; };

	//transpose
	//inline const XMatrix& GetProjectionMatrixTranspose()const		{ return ProjectionMatrixTranspose; };
	//inline const XMatrix& GetViewMatrixTranspose()const				{ return ViewMatrixTranspose; };
	//inline const XMatrix& GetViewProjectionMatrixTranspose()const	{ return ViewProjectionMatrixTranspose; };
	//inline const XMatrix& GetTranslatedViewMatrixTranspose()const	{ return TranslatedViewMatrixTranspose; };
	//inline const XMatrix& GetTranslatedViewProjectionMatrixTranspose()const { return TranslatedViewProjectionMatrixTranspose; };

	//inverse
	inline const XMatrix& GetViewProjectionMatrixInverse()const { return ViewProjectionMatrixInverse; };
	inline const XMatrix& GetTranslatedViewProjectionMatrixInverse()const { return TranslatedViewProjectionMatrixInverse; };

	//misc
	//XMatrix GetScreenToTranslatedWorldTranPose();
	XMatrix GetScreenToTranslatedWorld();
	//XMatrix GetScreenToWorldTranPose();
	XMatrix GetScreenToWorld();
};

struct ViewConstantBufferData
{
	XMatrix TranslatedViewProjectionMatrix;
	XMatrix ScreenToTranslatedWorld;
	XMatrix ViewToClip;
	XMatrix ScreenToWorld;
	XMatrix ViewProjectionMatrix;

	XVector4 InvDeviceZToWorldZTransform;
	XVector3 WorldCameraOrigin;
	uint32 StateFrameIndexMod8 = 0;

	XVector4 BufferSizeAndInvSize;
	XVector4 AtmosphereLightDirection;

	XVector3 SkyWorldCameraOrigin;
	float padding1 = 0.0;

	XVector4 SkyPlanetCenterAndViewHeight;
	XMatrix SkyViewLutReferential;

	XVector4 ViewSizeAndInvSize;
};

class XSceneView
{
public:
	std::shared_ptr<XRHIConstantBuffer>ViewConstantBuffer;
	ViewConstantBufferData ViewCBCPUData;
	XViewMatrices ViewMat;
};