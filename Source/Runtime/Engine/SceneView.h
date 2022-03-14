#pragma once
#include "Runtime/Core/XMath.h"

DirectX::XMFLOAT4 CreateInvDeviceZToWorldZTransform(const DirectX::XMFLOAT4X4 ProjMatrix);

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
		const DirectX::XMFLOAT4X4& ProjectionMatrixIn,
		const DirectX::XMFLOAT3& ViewLocation,
		const DirectX::XMFLOAT3& ViewTargetPosition);
private:
	//TODO Align
	DirectX::XMFLOAT3		ViewOrigin;
	DirectX::XMFLOAT3		PreViewTranslation;

	DirectX::XMFLOAT4X4		ProjectionMatrix;
	DirectX::XMFLOAT4X4		ViewMatrix;
	DirectX::XMFLOAT4X4		ViewProjectionMatrix;
	DirectX::XMFLOAT4X4		TranslatedViewMatrix;
	DirectX::XMFLOAT4X4		TranslatedViewProjectionMatrix;
	DirectX::XMFLOAT4X4		TranslatedWorldToClip;

	//Transpose
	DirectX::XMFLOAT4X4		ProjectionMatrixTranspose;
	DirectX::XMFLOAT4X4		ViewMatrixTranspose;
	DirectX::XMFLOAT4X4		ViewProjectionMatrixTranspose;
	DirectX::XMFLOAT4X4		TranslatedViewMatrixTranspose;
	DirectX::XMFLOAT4X4		TranslatedViewProjectionMatrixTranspose;

	//Inverse
	//DirectX::XMFLOAT4X4		ProjectionMatrixInverse;
	//DirectX::XMFLOAT4X4		ViewMatrixInverse;
	DirectX::XMFLOAT4X4		ViewProjectionMatrixInverse;
	//DirectX::XMFLOAT4X4		TranslatedViewMatrixInverse;
	DirectX::XMFLOAT4X4		TranslatedViewProjectionMatrixInverse;

	//misc
	//DirectX::XMFLOAT4X4		ScreenToTranslatedWorld;
public:
	void UpdateViewMatrix(const DirectX::XMFLOAT3& ViewLocation, const DirectX::XMFLOAT3& ViewTargetPosition);

//TODO
//#define GetMatrix_COMMON_TRANSPOSE_INVERSE(MAT_NAME)\
//	inline const DirectX::XMFLOAT4X4& GetMAT_NAME()const { return MAT_NAME; };\
//	inline const DirectX::XMFLOAT4X4& GetMAT_NAMETranspose()const { return MAT_NAMETranspose; };\
//	inline const DirectX::XMFLOAT4X4& GetMAT_NAMEInverse()const { return MAT_NAMEInverse; };\

	inline const DirectX::XMFLOAT3& GetViewOrigin()const { return ViewOrigin; };
	inline const DirectX::XMFLOAT3& GetPreViewTranslation()const { return PreViewTranslation; };

	//common
	inline const DirectX::XMFLOAT4X4& GetProjectionMatrix()const				{ return ProjectionMatrix; };
	inline const DirectX::XMFLOAT4X4& GetViewMatrix()const						{ return ViewMatrix; };
	inline const DirectX::XMFLOAT4X4& GetViewProjectionMatrix()const			{ return ViewProjectionMatrix; };
	inline const DirectX::XMFLOAT4X4& GetTranslatedViewMatrix()const			{ return TranslatedViewMatrix; };
	inline const DirectX::XMFLOAT4X4& GetTranslatedViewProjectionMatrix()const	{ return TranslatedViewProjectionMatrix; };

	//transpose
	inline const DirectX::XMFLOAT4X4& GetProjectionMatrixTranspose()const		{ return ProjectionMatrixTranspose; };
	inline const DirectX::XMFLOAT4X4& GetViewMatrixTranspose()const				{ return ViewMatrixTranspose; };
	inline const DirectX::XMFLOAT4X4& GetViewProjectionMatrixTranspose()const	{ return ViewProjectionMatrixTranspose; };
	inline const DirectX::XMFLOAT4X4& GetTranslatedViewMatrixTranspose()const	{ return TranslatedViewMatrixTranspose; };
	inline const DirectX::XMFLOAT4X4& GetTranslatedViewProjectionMatrixTranspose()const { return TranslatedViewProjectionMatrixTranspose; };

	//inverse
	inline const DirectX::XMFLOAT4X4& GetViewProjectionMatrixInverse()const { return ViewProjectionMatrixInverse; };
	inline const DirectX::XMFLOAT4X4& GetTranslatedViewProjectionMatrixInverse()const { return TranslatedViewProjectionMatrixInverse; };

	//misc
	DirectX::XMFLOAT4X4 GetScreenToTranslatedWorldTranPose();
	
};