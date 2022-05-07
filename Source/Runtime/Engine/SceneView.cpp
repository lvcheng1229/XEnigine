#include "SceneView.h"

using namespace DirectX;

XVector4 CreateInvDeviceZToWorldZTransform(const XMatrix ProjMatrix)
{
	// DeviceZ = A + B / WorldZ

	float DepthMul = ProjMatrix.m[2][2];
	float DepthAdd = ProjMatrix.m[3][2];

	if (DepthAdd == 0.f) { DepthAdd = 0.00000001f; }

	// combined perspective and ortho equation in shader to handle either
	//SceneView.cpp line 416

	bool bIsPerspectiveProjection = ProjMatrix.m[3][3] < 1.0f;
	if (bIsPerspectiveProjection)
	{
		float SubtractValue = DepthMul / DepthAdd;
		SubtractValue -= 0.00000001f;
		return XMFLOAT4(0.0f, 0.0f, 1.0f / DepthAdd, SubtractValue);
	}
	else
	{
		return XMFLOAT4(
			1.0f / ProjMatrix.m[2][2],
			-ProjMatrix.m[3][2] / ProjMatrix.m[2][2] + 1.0f,
			0.0f, 1.0f);
	}
}

static void StoreMat_Transpose_Inverse(
	XMFLOAT4X4* Common, 
	XMFLOAT4X4* Tranpose, 
	XMFLOAT4X4* Inverse, 
	XMMATRIX& MatrixIn)
{
	XMStoreFloat4x4(Common, MatrixIn);
	XMStoreFloat4x4(Tranpose, XMMatrixTranspose(MatrixIn));

	DirectX::XMVECTOR Determinant = XMMatrixDeterminant(MatrixIn);
	XMStoreFloat4x4(Inverse, XMMatrixInverse(&Determinant, MatrixIn));
}

void XViewMatrices::Create(const XMatrix& ProjectionMatrixIn, const XVector3& ViewLocation, const XVector3& ViewTargetPosition)
{
	ProjectionMatrix = ProjectionMatrixIn;
	UpdateViewMatrix(ViewLocation, ViewTargetPosition);
}

void XViewMatrices::UpdateViewMatrix(const XVector3& ViewLocation, const XVector3& ViewTargetPosition)
{
	ViewOrigin = ViewLocation;
	PreViewTranslation = XMFLOAT3(-ViewOrigin.x, -ViewOrigin.y, -ViewOrigin.z);

	XMVECTOR ViewOriginVec = XMVectorSet(ViewOrigin.x, ViewOrigin.y, ViewOrigin.z, 1.0f);
	XMVECTOR ViewTargetVec = XMVectorSet(ViewTargetPosition.x, ViewTargetPosition.y, ViewTargetPosition.z, 1.0f);
	DirectX::XMVECTOR EyeDirection = DirectX::XMVectorSubtract(ViewTargetVec, ViewOriginVec);
	XMVECTOR UpDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//Cartesian coordinates is right hand

	//compute uvw
	XMVECTOR WNormalize = XMVector3Normalize(EyeDirection);
	XMVECTOR UNormalize = XMVector3Cross(UpDirection, WNormalize);//??
	//XMVECTOR UNormalize = XMVector3Cross(WNormalize,UpDirection);//??
	UNormalize = XMVector3Normalize(UNormalize);
	XMVECTOR VNormalize = XMVector3Cross(WNormalize, UNormalize);
	//XMVECTOR VNormalize = XMVector3Cross(UNormalize, WNormalize);

	//compute -qu
	XMVECTOR NegEyePosition = XMVectorNegate(ViewOriginVec);
	XMVECTOR NegQU = XMVector3Dot(UNormalize, NegEyePosition);
	XMVECTOR NegQV = XMVector3Dot(VNormalize, NegEyePosition);
	XMVECTOR NegQW = XMVector3Dot(WNormalize, NegEyePosition);

	//
	XMMATRIX TranslatedViewMatrixCom;
	TranslatedViewMatrixCom.r[0] = XMVectorSelect(g_XMZero, UNormalize, g_XMSelect1110.v);
	TranslatedViewMatrixCom.r[1] = XMVectorSelect(g_XMZero, VNormalize, g_XMSelect1110.v);
	TranslatedViewMatrixCom.r[2] = XMVectorSelect(g_XMZero, WNormalize, g_XMSelect1110.v);
	TranslatedViewMatrixCom.r[3] = g_XMIdentityR3.v;
	TranslatedViewMatrixCom = XMMatrixTranspose(TranslatedViewMatrixCom);
	XMStoreFloat4x4(&TranslatedViewMatrix, TranslatedViewMatrixCom);//NOTE
	XMStoreFloat4x4(&TranslatedViewMatrixTranspose, TranslatedViewMatrixCom);

	XMMATRIX ViewMatrixCom;
	ViewMatrixCom.r[0] = XMVectorSelect(NegQU, UNormalize, g_XMSelect1110.v);
	ViewMatrixCom.r[1] = XMVectorSelect(NegQV, VNormalize, g_XMSelect1110.v);
	ViewMatrixCom.r[2] = XMVectorSelect(NegQW, WNormalize, g_XMSelect1110.v);
	ViewMatrixCom.r[3] = g_XMIdentityR3.v;
	XMStoreFloat4x4(&ViewMatrixTranspose, ViewMatrixCom);
	ViewMatrixCom = XMMatrixTranspose(ViewMatrixCom);
	XMStoreFloat4x4(&ViewMatrix, ViewMatrixCom);

	XMMATRIX ProjectionMatrixCom = XMLoadFloat4x4(&ProjectionMatrix);
	XMStoreFloat4x4(&ProjectionMatrixTranspose, XMMatrixTranspose(ProjectionMatrixCom));

	XMMATRIX ViewProjectionMatrixCom = XMMatrixMultiply(ViewMatrixCom, ProjectionMatrixCom);
	XMMATRIX TranslatedViewProjectionMatrixCom = XMMatrixMultiply(TranslatedViewMatrixCom, ProjectionMatrixCom);


	StoreMat_Transpose_Inverse(
		&ViewProjectionMatrix,
		&ViewProjectionMatrixTranspose,
		&ViewProjectionMatrixInverse,
		ViewProjectionMatrixCom
	);
	
	StoreMat_Transpose_Inverse(
		&TranslatedViewProjectionMatrix,
		&TranslatedViewProjectionMatrixTranspose,
		&TranslatedViewProjectionMatrixInverse,
		TranslatedViewProjectionMatrixCom
	);
}

XMatrix XViewMatrices::GetScreenToTranslatedWorldTranPose()
{
	XMFLOAT4X4 ScreenToClip = XDirectx::GetIdentityMatrix();
	ScreenToClip.m[2][2] = ProjectionMatrix.m[2][2];
	ScreenToClip.m[3][2] = ProjectionMatrix.m[3][2];
	ScreenToClip.m[2][3] = 1.0f;
	ScreenToClip.m[3][3] = 0.0f;

	XMMATRIX ScreenToTranslatedWorldCom = XMLoadFloat4x4(&ScreenToClip);
	ScreenToTranslatedWorldCom = XMMatrixMultiply(ScreenToTranslatedWorldCom, XMLoadFloat4x4(&TranslatedViewProjectionMatrixInverse));
	
	DirectX::XMFLOAT4X4 Ret;
	XMStoreFloat4x4(&Ret, XMMatrixTranspose(ScreenToTranslatedWorldCom));
	return Ret;
}

XMatrix XViewMatrices::GetScreenToWorldTranPose()
{
	XMFLOAT4X4 ScreenToClip = XDirectx::GetIdentityMatrix();
	ScreenToClip.m[2][2] = ProjectionMatrix.m[2][2];
	ScreenToClip.m[3][2] = ProjectionMatrix.m[3][2];
	ScreenToClip.m[2][3] = 1.0f;
	ScreenToClip.m[3][3] = 0.0f;

	XMMATRIX ScreenToWorldCom = XMLoadFloat4x4(&ScreenToClip);
	ScreenToWorldCom = XMMatrixMultiply(ScreenToWorldCom, XMLoadFloat4x4(&ViewProjectionMatrixInverse));

	DirectX::XMFLOAT4X4 Ret;
	XMStoreFloat4x4(&Ret, XMMatrixTranspose(ScreenToWorldCom));
	return Ret;
}

