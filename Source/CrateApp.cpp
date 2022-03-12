//***************************************************************************************
// CrateApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************


#include "UnitTest/d3dApp.h"
#include "UnitTest/MathHelper.h"
#include "UnitTest/GeometryGenerator.h"
#include "FrameResource.h"
#include <memory>

#include <windows.h>

#include "Runtime/RenderCore/Shader.h"
#include "Runtime/D3D12RHI/D3D12PipelineCurrentDescArray.h"
#include "Runtime/D3D12RHI/D3D12PipelineCurrentDescArrayManager.h"
#include "Runtime/D3D12RHI/D3D12PlatformRHI.h"
#include "Runtime/D3D12RHI/D3D12Texture.h"

#include "File/LoadTGATexture.h"

#include  "Runtime/Engine/SceneView.h"
#include "Runtime/Core/XMath.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "File/stb_image.h"

struct RenderItem
{
	RenderItem() = default; 
    XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

    // DrawIndexedInstanced parameters.
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;
};

struct BoundSphere
{
	DirectX::XMFLOAT3 Center;
	float Radius;
};

class CrateApp : public D3DApp
{
public:
    CrateApp();
    CrateApp(const CrateApp& rhs) = delete;
    CrateApp& operator=(const CrateApp& rhs) = delete;
    ~CrateApp();

    virtual bool Initialize()override;

private:
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Renderer(const GameTimer& gt)override;

    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

    void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void UpdateShadowTransform(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	void LoadTextures();
    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    void BuildShapeGeometry();
    void BuildPSOs();
    void BuildMaterials();
    void BuildRenderItems();
	
	BoundSphere BuildBoundSphere(float FoVAngleY, float WHRatio, float SplirNear, float SplitFar);
	
private:
	float Far = 1000.0f;
	float Near = 1.0f;
	float FoVAngleY = 0.25f * MathHelper::Pi;

	XD3D12RootSignature d3d12_root_signature;
	std::shared_ptr<XRHITexture2D>TextureMetalBaseColor;
	std::shared_ptr<XRHITexture2D>TextureMetalNormal;
	std::shared_ptr<XRHITexture2D>TextureRoughness;

	std::shared_ptr<XRHITexture2D>TextureWoodBaseColor;
	std::shared_ptr<XRHITexture2D>TextureWoodNormal;

	std::shared_ptr<XRHITexture2D>TextureGBufferA;
	std::shared_ptr<XRHITexture2D>TextureGBufferB;
	std::shared_ptr<XRHITexture2D>TextureGBufferC;
	std::shared_ptr<XRHITexture2D>TextureGBufferD;
	std::shared_ptr<XRHITexture2D>TextureSceneColorDeffered;

	XRHIRenderTargetView* RTViews[8];
private:
	struct ViewConstantBufferTable
	{
		DirectX::XMFLOAT4X4 ScreenToTranslatedWorld;
		DirectX::XMFLOAT4 InvDeviceZToWorldZTransform;
		DirectX::XMFLOAT3 WorldCameraOrigin;
		float padding0 = 0.0;
		DirectX::XMFLOAT4 BufferSizeAndInvSize;
	};

	//Full Screen Pass
	XD3D12RootSignature FullScreenRootSig;
	ComPtr<ID3D12PipelineState>FullScreenPSO = nullptr;
	std::unique_ptr<RenderItem> fullScreenItem;
	XViewMatrices ViewMatrix;
	
	ViewConstantBufferTable ViewCB;
//Shadow Pass
private:
	struct ShadowPassConstants
	{
		DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
	};

	
	XViewMatrices LightMatrix[4];

	ShadowPassConstants ShadowPassConstant[4];
	ComPtr<ID3D12PipelineState> ShadowPSO = nullptr;
	XD3D12RootSignature ShadowPassRootSig;
	std::shared_ptr<XRHIConstantBuffer>ShadowPassConstantBuffer[4];
	ViewConstantBufferTable ShadowCB;
	std::shared_ptr<XRHITexture2D>ShadowTexture0;
	
	float ShadowMapHeight = 1024;
	float ShadowMapWidth = 1024 * 4;
	float ShadowViewportWidth = 1024;
	//float ShadowRadius = 256;

	//HZBPass 
private:
	ComPtr<ID3D12PipelineState> HZBPSO = nullptr;
	XD3D12RootSignature HZBPassRootSig;
	struct cbHZB
	{
		DirectX::XMFLOAT4 DispatchThreadIdToBufferUV;
	};
	cbHZB cbHZBins;
	std::shared_ptr<XRHIConstantBuffer>RHICbbHZB;
	std::shared_ptr<XRHITexture2D> FurthestHZBOutput0;
//Shadow Mask Pass 
private:
	ComPtr<ID3D12PipelineState> ShadowMaskPSO = nullptr;
	XD3D12RootSignature ShadowMaskPassRootSig;
	std::shared_ptr<XRHIConstantBuffer>ShadowMaskPassViewConstantBuffer;
	struct cbShadowMaskNoCommonBuffer
	{
		XMFLOAT4X4 ScreenToShadowMatrix;
		float x_offset;
		XMFLOAT3 padding0;
	};
	cbShadowMaskNoCommonBuffer cbShadowMaskNoCommon[4];
	std::shared_ptr<XRHIConstantBuffer>ShadowMaskNoCommonConstantBuffer[4];
	std::shared_ptr<XRHITexture2D>ShadowMaskTexture;
//Depth Only
private:
	XD3D12RootSignature PrePassRootSig;
	ComPtr<ID3D12PipelineState> mDepthOnlyPSO = nullptr;
private:

	std::shared_ptr<XRHIConstantBuffer>ViewConstantBuffer;
	std::unique_ptr<RenderItem> LightPassItem;
	ComPtr<ID3D12PipelineState>LightPassPso = nullptr;
	XD3D12RootSignature LightPassRootSig;
private:
	float clear_color[4] = { 0, 0, 0, 0 };
	std::unique_ptr<FrameResource>mFrameResource;
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, XShader> mShaders;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mFullScreenInputLayout;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mBasePassLayout;

    ComPtr<ID3D12PipelineState> mOpaquePSO = nullptr;
 
	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	

	// Render items divided by PSO.
	std::vector<RenderItem*> mOpaqueRitems;

    PassConstants mMainPassCB;

	XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 mTargetPos = { 0.0f, 0.0f, 0.0f };
	//XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();
	XMFLOAT4X4 mLightProj = MathHelper::Identity4x4();
	float mTheta = 1.3f*XM_PI;
	float mPhi = 0.4f*XM_PI;
	float mRadius = 2.5f;

    POINT mLastMousePos;
};

CrateApp::CrateApp()
{
}

CrateApp::~CrateApp()
{
    if(md3dDevice != nullptr)
		direct_cmd_queue->CommandQueueWaitFlush();
}

//-------------------
bool CrateApp::Initialize()
{
	
    if(!D3DApp::Initialize())
        return false;
	direct_ctx->OpenCmdList();
	

	LoadTextures();
    BuildShadersAndInputLayout();
    BuildRootSignature();
    BuildShapeGeometry();
	BuildMaterials();
    BuildRenderItems();
	mFrameResource = std::make_unique<FrameResource>();
	
	mFrameResource.get()->PassConstantBuffer =
		abstrtact_device.CreateUniformBuffer(d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants)));
	
	for (int i = 0; i < mMaterials.size();i++)
	{
		mFrameResource.get()->MaterialConstantBuffer.push_back(
			abstrtact_device.CreateUniformBuffer(d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants))));
	}

	for (uint32 i = 0; i < mAllRitems.size();++i)
	{
		mFrameResource.get()->ObjectConstantBuffer.push_back(
			abstrtact_device.CreateUniformBuffer(d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants))));
	}
	ViewConstantBuffer = 
		abstrtact_device.CreateUniformBuffer(
			d3dUtil::CalcConstantBufferByteSize(sizeof(ViewConstantBufferTable)));

	for (uint32 i = 0; i < 4; i++)
	{
		ShadowPassConstantBuffer[i] =
			abstrtact_device.CreateUniformBuffer(
				d3dUtil::CalcConstantBufferByteSize(sizeof(ViewConstantBufferTable)));
	}

	ShadowMaskPassViewConstantBuffer =
		abstrtact_device.CreateUniformBuffer(
			d3dUtil::CalcConstantBufferByteSize(sizeof(ViewConstantBufferTable)));

	for (uint32 i = 0; i < 4; i++)
	{
		ShadowMaskNoCommonConstantBuffer[i] =
			abstrtact_device.CreateUniformBuffer(
				d3dUtil::CalcConstantBufferByteSize(sizeof(cbShadowMaskNoCommonBuffer)));
	}

	RHICbbHZB= abstrtact_device.CreateUniformBuffer(
		d3dUtil::CalcConstantBufferByteSize(sizeof(cbHZB)));
	BuildPSOs();

	OutputDebugString(L"1111\n");

	direct_ctx->CloseCmdList();
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	direct_cmd_queue->CommandQueueWaitFlush();
	OutputDebugString(L"2222\n");
    return true;
}
 
BoundSphere CrateApp::BuildBoundSphere(float FoVAngleY, float WHRatio, float SplitNear, float SplitFar)
{
	float SinVDiv2, CosVDiv2;
	XMScalarSinCos(&SinVDiv2, &CosVDiv2, 0.5f * FoVAngleY);
	float TanVDiv2 = SinVDiv2 / CosVDiv2;

	float ViewNearHeight = SplitNear * TanVDiv2;
	float ViewNearWidth = AspectRatio() * ViewNearHeight;
	float ViewFarHeight = SplitFar * TanVDiv2;
	float ViewFarWidth = AspectRatio() * ViewFarHeight;

	XMFLOAT3 NearHOffset = { 0,ViewNearHeight,0 };
	XMFLOAT3 NearVOffset = { ViewNearWidth,0,0 };
	XMFLOAT3 FarHOffset = { 0,ViewFarHeight,0 };
	XMFLOAT3 FarVOffset = { ViewFarWidth,0,0 };
	XMVECTOR NearHOffsetCom = XMLoadFloat3(&NearHOffset);
	XMVECTOR NearVOffsetCom = XMLoadFloat3(&NearVOffset);
	XMVECTOR FarHOffsetCom = XMLoadFloat3(&FarHOffset);
	XMVECTOR FarVOffsetCom = XMLoadFloat3(&FarVOffset);
	

	XMVECTOR EyePosCom = XMLoadFloat3(&mEyePos);
	XMVECTOR TargetPosCom = XMLoadFloat3(&mTargetPos);
	XMVECTOR CameraDirection = DirectX::XMVector3Normalize(TargetPosCom - EyePosCom);
	
	XMVECTOR CascadeFrustumVerts[8];
	CascadeFrustumVerts[0] = EyePosCom + CameraDirection * SplitNear + NearHOffsetCom + NearVOffsetCom;//Near Top Right
	CascadeFrustumVerts[1] = EyePosCom + CameraDirection * SplitNear + NearHOffsetCom - NearVOffsetCom;//Near Top Left
	CascadeFrustumVerts[2] = EyePosCom + CameraDirection * SplitNear - NearHOffsetCom + NearVOffsetCom;//Near Bottom Right
	CascadeFrustumVerts[3] = EyePosCom + CameraDirection * SplitNear - NearHOffsetCom - NearVOffsetCom;//Near Bottom Left
	CascadeFrustumVerts[4] = EyePosCom + CameraDirection * SplitFar + FarHOffsetCom + FarVOffsetCom;//Far Top Right
	CascadeFrustumVerts[5] = EyePosCom + CameraDirection * SplitFar + FarHOffsetCom - FarVOffsetCom;//Far Top Left
	CascadeFrustumVerts[6] = EyePosCom + CameraDirection * SplitFar - FarHOffsetCom + FarVOffsetCom;//Far Bottom Right
	CascadeFrustumVerts[7] = EyePosCom + CameraDirection * SplitFar - FarHOffsetCom - FarVOffsetCom;//Far Bottom Left

	float FrustumLength = SplitFar - SplitNear;
	float DiagonalNear = ViewNearHeight * ViewNearHeight + ViewNearWidth * ViewNearWidth;
	float DiagonalFar = ViewFarHeight * ViewFarHeight + ViewFarWidth * ViewFarWidth;
	float OptimalOffset = (DiagonalNear - DiagonalFar) / (2.0f * FrustumLength) + FrustumLength * 0.5f;
	float CentreZ = SplitFar - OptimalOffset;
	CentreZ = min(max(CentreZ, SplitNear), SplitFar);
	
	
	XMFLOAT3 Center;
	XMVECTOR CenterCom = EyePosCom + CameraDirection * CentreZ;
	XMStoreFloat3(&Center, CenterCom);
	BoundSphere BoundSphereRet = { Center ,0 };
	
	XMFLOAT3 Temp;
	for (uint32 i = 0; i < 8; i++)
	{
		XMStoreFloat3(&Temp, XMVector3Length(CascadeFrustumVerts[i] - CenterCom));
		BoundSphereRet.Radius = max(BoundSphereRet.Radius, Temp.x);
	}
	BoundSphereRet.Radius = max(BoundSphereRet.Radius, 1.0f);
	return BoundSphereRet;
}

void CrateApp::OnResize()
{
    D3DApp::OnResize();
	

	XMMATRIX P = XDirectx::XXMMatrixPerspectiveFovLH(FoVAngleY, AspectRatio(), Near, Far);
	
	XMStoreFloat4x4(&mProj, P);
	ViewMatrix.Create(mProj, mEyePos, mTargetPos);
	
	
}

void CrateApp::Update(const GameTimer& gt)
{
    OnKeyboardInput(gt);
	UpdateCamera(gt);

	AnimateMaterials(gt);
	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);
}

void CrateApp::Renderer(const GameTimer& gt)
{
	//static int i = 0;
	//std::wstring str = L"xx" + std::to_wstring(i) + L"xx\n";
	//OutputDebugString(str.c_str());
	
	//Pass1 DepthPrePass
	{
		direct_ctx->ResetCmdAlloc();
		direct_ctx->OpenCmdList();
		direct_ctx->RHISetViewport(0.0f, 0.0f, 0.0f, mClientWidth, mClientHeight, 1.0f);
		mCommandList->SetPipelineState(mDepthOnlyPSO.Get());
		pass_state_manager->SetRootSignature(&PrePassRootSig);
		pass_state_manager->SetHeapDesc();
		direct_ctx->RHISetRenderTargets(0, nullptr,
			static_cast<XD3D12Texture2D*>(TextureDepthStencil.get())->GeDepthStencilView());
		direct_ctx->RHIClearMRT(false, true, nullptr, 0.0f, 0);

		for (size_t i = 0; i < mOpaqueRitems.size(); ++i)
		{
			auto& ri = mOpaqueRitems[i];
			direct_ctx->RHISetShaderConstantBuffer(
				mShaders["PrePassVS"].GetRHIGraphicsShader().get(), 0,
				mFrameResource.get()->ObjectConstantBuffer[ri->ObjCBIndex].get());

			direct_ctx->RHISetShaderConstantBuffer(
				mShaders["PrePassVS"].GetRHIGraphicsShader().get(), 1,
				mFrameResource.get()->PassConstantBuffer.get());

			mCommandList.Get()->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
			mCommandList.Get()->IASetIndexBuffer(&ri->Geo->IndexBufferView());
			pass_state_manager->ApplyCurrentStateToPipeline<ED3D12PipelineType::D3D12PT_Graphics>();

			direct_ctx->GetCmdList()->CmdListFlushBarrier();
			mCommandList.Get()->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		}

		direct_ctx->CloseCmdList();

		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		direct_cmd_queue->CommandQueueWaitFlush();
		pass_state_manager->ResetState();
	}
	
	//Pass2 HZB Pass
	{
		direct_ctx->ResetCmdAlloc();
		direct_ctx->OpenCmdList();
		mCommandList->SetPipelineState(HZBPSO.Get());
		pass_state_manager->SetRootSignature(&HZBPassRootSig);
	
		direct_ctx->RHISetShaderTexture(mShaders["HZBPassCS"].GetRHIComputeShader().get(), 0,
			TextureDepthStencil.get());
	
		direct_ctx->RHISetShaderConstantBuffer(
			mShaders["HZBPassCS"].GetRHIComputeShader().get(),
			0,
			RHICbbHZB.get());
		direct_ctx->RHISetShaderUAV(mShaders["HZBPassCS"].GetRHIComputeShader().get(), 0, FurthestHZBOutput0.get());
	
		pass_state_manager->ApplyCurrentStateToPipeline<ED3D12PipelineType::D3D12PT_Compute>();
		
		direct_ctx->GetCmdList()->CmdListFlushBarrier();
		mCommandList.Get()->Dispatch(512 / 16, 512 / 16, 1);
		
		direct_ctx->CloseCmdList();
		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		direct_cmd_queue->CommandQueueWaitFlush();
		pass_state_manager->ResetState();
	}

	//Pass3 ShadowPass
	{
		direct_ctx->ResetCmdAlloc();
		direct_ctx->OpenCmdList();
		
		mCommandList->SetPipelineState(ShadowPSO.Get());
		pass_state_manager->SetRootSignature(&ShadowPassRootSig);
		direct_ctx->RHISetRenderTargets(0, nullptr,
			static_cast<XD3D12Texture2D*>(ShadowTexture0.get())->GeDepthStencilView());
		direct_ctx->RHIClearMRT(false, true, nullptr, 0.0f, 0);
		
		for (uint32 CSMIndex = 0; CSMIndex < 4; CSMIndex++)
		{
			direct_ctx->RHISetViewport(CSMIndex * ShadowViewportWidth, 0.0f, 0.0f, 
				CSMIndex * ShadowViewportWidth+ShadowViewportWidth, 
				ShadowViewportWidth, 1.0f);

			for (size_t i = 0; i < mOpaqueRitems.size(); ++i)
			{
				auto& ri = mOpaqueRitems[i];
				direct_ctx->RHISetShaderConstantBuffer(
					mShaders["ShadowPassVS"].GetRHIGraphicsShader().get(), 0,
					mFrameResource.get()->ObjectConstantBuffer[ri->ObjCBIndex].get());

				direct_ctx->RHISetShaderConstantBuffer(
					mShaders["ShadowPassVS"].GetRHIGraphicsShader().get(), 1,
					ShadowPassConstantBuffer[CSMIndex].get());

				mCommandList.Get()->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
				mCommandList.Get()->IASetIndexBuffer(&ri->Geo->IndexBufferView());
				pass_state_manager->ApplyCurrentStateToPipeline<ED3D12PipelineType::D3D12PT_Graphics>();

				direct_ctx->GetCmdList()->CmdListFlushBarrier();
				mCommandList.Get()->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
			}

		}

		direct_ctx->CloseCmdList();

		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		direct_cmd_queue->CommandQueueWaitFlush();
		pass_state_manager->ResetState();
	}

	//Pass4 GBufferPass BasePass
	{
		direct_ctx->ResetCmdAlloc();
		direct_ctx->OpenCmdList();
		direct_ctx->RHISetViewport(0.0f, 0.0f, 0.0f, mClientWidth, mClientHeight, 1.0f);

		mCommandList->SetPipelineState(mOpaquePSO.Get());
		pass_state_manager->SetRootSignature(&d3d12_root_signature);
		//pass_state_manager->SetHeapDesc();

		RTViews[0] = static_cast<XD3D12Texture2D*>(TextureGBufferA.get())->GetRenderTargetView();
		RTViews[1] = static_cast<XD3D12Texture2D*>(TextureGBufferB.get())->GetRenderTargetView();
		RTViews[2] = static_cast<XD3D12Texture2D*>(TextureGBufferC.get())->GetRenderTargetView();
		RTViews[3] = static_cast<XD3D12Texture2D*>(TextureGBufferD.get())->GetRenderTargetView();

		direct_ctx->RHISetRenderTargets(4, 
			RTViews,
			static_cast<XD3D12Texture2D*>(TextureDepthStencil.get())->GeDepthStencilView());
		direct_ctx->RHIClearMRT(true, false, clear_color, 0.0f, 0);
		
		for (size_t i = 0; i < mOpaqueRitems.size(); ++i)
		//for (size_t i =1; i < 2; ++i)
		{
			auto& ri = mOpaqueRitems[i];

			direct_ctx->RHISetShaderConstantBuffer(
				mShaders["standardVS"].GetRHIGraphicsShader().get(),0,
				mFrameResource.get()->ObjectConstantBuffer[ri->ObjCBIndex].get());
			
			direct_ctx->RHISetShaderConstantBuffer(
				mShaders["standardVS"].GetRHIGraphicsShader().get(),1,
				mFrameResource.get()->PassConstantBuffer.get());
			
			//direct_ctx->RHISetShaderTexture(mShaders["opaquePS"].GetRHIGraphicsShader().get(), 0,
			//	TextureMetalBaseColor.get());
			//direct_ctx->RHISetShaderTexture(mShaders["opaquePS"].GetRHIGraphicsShader().get(), 1,
			//	TextureMetalNormal.get());
			//direct_ctx->RHISetShaderTexture(mShaders["opaquePS"].GetRHIGraphicsShader().get(), 2,
			//	TextureRoughness.get());

			direct_ctx->RHISetShaderTexture(mShaders["opaquePS"].GetRHIGraphicsShader().get(), 0, 
				ri->Mat->TextureBaseColor.get());
			direct_ctx->RHISetShaderTexture(mShaders["opaquePS"].GetRHIGraphicsShader().get(), 1, 
				ri->Mat->TextureNormal.get());
			direct_ctx->RHISetShaderTexture(mShaders["opaquePS"].GetRHIGraphicsShader().get(), 2, 
				ri->Mat->TextureRoughness.get());
			
			direct_ctx->RHISetShaderConstantBuffer(
				mShaders["opaquePS"].GetRHIGraphicsShader().get(),
				0,
				mFrameResource.get()->MaterialConstantBuffer[ri->Mat->MatCBIndex].get());


			mCommandList.Get()->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
			mCommandList.Get()->IASetIndexBuffer(&ri->Geo->IndexBufferView());
			pass_state_manager->ApplyCurrentStateToPipeline<ED3D12PipelineType::D3D12PT_Graphics>();

			direct_ctx->GetCmdList()->CmdListFlushBarrier();
			mCommandList.Get()->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		}


		
		direct_ctx->CloseCmdList();

		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		direct_cmd_queue->CommandQueueWaitFlush();
		pass_state_manager->ResetState();
	}
	
	//Pass5 Shadow Mask Pass
	{
		direct_ctx->ResetCmdAlloc();
		direct_ctx->OpenCmdList();
		mCommandList->SetPipelineState(ShadowMaskPSO.Get());
		pass_state_manager->SetRootSignature(&ShadowMaskPassRootSig);
		direct_ctx->RHISetViewport(0.0f, 0.0f, 0.0f, mClientWidth, mClientHeight, 1.0f);
		RTViews[0] = static_cast<XD3D12Texture2D*>(ShadowMaskTexture.get())->GetRenderTargetView();
		direct_ctx->RHISetRenderTargets(1, RTViews, nullptr);//TODO
		direct_ctx->RHIClearMRT(true, false, clear_color, 0.0f, 0);

		direct_ctx->RHISetShaderConstantBuffer(
			mShaders["ShadowMaskPS"].GetRHIGraphicsShader().get(),
			0,
			ShadowMaskPassViewConstantBuffer.get());

		direct_ctx->RHISetShaderTexture(
			mShaders["ShadowMaskPS"].GetRHIGraphicsShader().get()
			, 0
			, ShadowTexture0.get());

		direct_ctx->RHISetShaderTexture(
			mShaders["ShadowMaskPS"].GetRHIGraphicsShader().get()
			, 1
			, TextureGBufferA.get());

		direct_ctx->RHISetShaderTexture(
			mShaders["ShadowMaskPS"].GetRHIGraphicsShader().get()
			, 2
			, TextureDepthStencil.get());
		for (int CSMindex = 3; CSMindex >= 0; CSMindex--)
		{
			direct_ctx->RHISetShaderConstantBuffer(
				mShaders["ShadowMaskPS"].GetRHIGraphicsShader().get(),
				1,
				ShadowMaskNoCommonConstantBuffer[CSMindex].get());

			mCommandList.Get()->IASetVertexBuffers(0, 1, &fullScreenItem->Geo->VertexBufferView());
			mCommandList.Get()->IASetIndexBuffer(&fullScreenItem->Geo->IndexBufferView());
			pass_state_manager->ApplyCurrentStateToPipeline<ED3D12PipelineType::D3D12PT_Graphics>();
			mCommandList.Get()->DrawIndexedInstanced(
				fullScreenItem->IndexCount, 1,
				fullScreenItem->StartIndexLocation,
				fullScreenItem->BaseVertexLocation, 0);
		}

		direct_ctx->CloseCmdList();

		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		direct_cmd_queue->CommandQueueWaitFlush();
		pass_state_manager->ResetState();
	}

	//Pass6 LightPass
	{
		direct_ctx->ResetCmdAlloc();
		direct_ctx->OpenCmdList();
		mCommandList->SetPipelineState(LightPassPso.Get());
		pass_state_manager->SetRootSignature(&LightPassRootSig);

		direct_ctx->RHISetViewport(0.0f, 0.0f, 0.0f, mClientWidth, mClientHeight, 1.0f);
		RTViews[0] = static_cast<XD3D12Texture2D*>(TextureSceneColorDeffered.get())->GetRenderTargetView();
		direct_ctx->RHISetRenderTargets(1, RTViews, nullptr);//TODO
		direct_ctx->RHIClearMRT(true, false, clear_color, 0.0f, 0);

		direct_ctx->RHISetShaderConstantBuffer(
			mShaders["LightPassVS"].GetRHIGraphicsShader().get(),
			0,ViewConstantBuffer.get());

		direct_ctx->RHISetShaderConstantBuffer(mShaders["LightPassPS"].GetRHIGraphicsShader().get(), 
			0,ViewConstantBuffer.get());


		direct_ctx->RHISetShaderTexture(mShaders["LightPassPS"].GetRHIGraphicsShader().get(), 0,
			TextureGBufferA.get());
		direct_ctx->RHISetShaderTexture(mShaders["LightPassPS"].GetRHIGraphicsShader().get(), 1,
			TextureGBufferB.get());
		direct_ctx->RHISetShaderTexture(mShaders["LightPassPS"].GetRHIGraphicsShader().get(), 2,
			TextureGBufferC.get());
		direct_ctx->RHISetShaderTexture(mShaders["LightPassPS"].GetRHIGraphicsShader().get(), 3,
			TextureGBufferD.get());
		direct_ctx->RHISetShaderTexture(mShaders["LightPassPS"].GetRHIGraphicsShader().get(), 4,
			TextureDepthStencil.get());
		direct_ctx->RHISetShaderTexture(mShaders["LightPassPS"].GetRHIGraphicsShader().get(), 5,
			ShadowMaskTexture.get());

		mCommandList.Get()->IASetVertexBuffers(0, 1, &fullScreenItem->Geo->VertexBufferView());
		mCommandList.Get()->IASetIndexBuffer(&fullScreenItem->Geo->IndexBufferView());
		pass_state_manager->ApplyCurrentStateToPipeline<ED3D12PipelineType::D3D12PT_Graphics>();
		
		direct_ctx->GetCmdList()->CmdListFlushBarrier();
		mCommandList.Get()->DrawIndexedInstanced(
			fullScreenItem->IndexCount, 1,
			fullScreenItem->StartIndexLocation,
			fullScreenItem->BaseVertexLocation, 0);

		direct_ctx->CloseCmdList();

		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		direct_cmd_queue->CommandQueueWaitFlush();
		pass_state_manager->ResetState();
	}

	//Pass7 FinalPass ToneMapping
	{
		direct_ctx->ResetCmdAlloc();
		direct_ctx->OpenCmdList();
		mCommandList->SetPipelineState(FullScreenPSO.Get());
		pass_state_manager->SetRootSignature(&FullScreenRootSig);

		direct_ctx->RHISetViewport(0.0f, 0.0f, 0.0f, mClientWidth, mClientHeight, 1.0f);
		
		RTViews[0] = viewport.GetCurrentBackRTView();
		direct_ctx->RHISetRenderTargets(1, RTViews, nullptr);//TODO
		direct_ctx->RHIClearMRT(true, false, clear_color, 0.0f, 0);

		direct_ctx->RHISetShaderTexture(
			mShaders["fullScreenPS"].GetRHIGraphicsShader().get()
			,0
			, TextureSceneColorDeffered.get());

		mCommandList.Get()->IASetVertexBuffers(0, 1, &fullScreenItem->Geo->VertexBufferView());
		mCommandList.Get()->IASetIndexBuffer(&fullScreenItem->Geo->IndexBufferView());
		pass_state_manager->ApplyCurrentStateToPipeline<ED3D12PipelineType::D3D12PT_Graphics>();
		mCommandList.Get()->DrawIndexedInstanced(
			fullScreenItem->IndexCount, 1, 
			fullScreenItem->StartIndexLocation, 
			fullScreenItem->BaseVertexLocation, 0);

		//fullScreenItem
		XD3D12PlatformRHI::TransitionResource(
			*direct_ctx->GetCmdList(),
			viewport.GetCurrentBackRTView(),
			D3D12_RESOURCE_STATE_PRESENT);

		direct_ctx->GetCmdList()->CmdListFlushBarrier();
		direct_ctx->CloseCmdList();
		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		direct_cmd_queue->CommandQueueWaitFlush();

		viewport.Present();

		pass_state_manager->ResetState();
		
	}

	{
		
	}
	
}

void CrateApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void CrateApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void CrateApp::OnMouseMove(WPARAM btnState, int x, int y)
{
    if((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

        // Update angles based on input to orbit camera around box.
        mTheta += dx;
        mPhi += dy;

        // Restrict the angle mPhi.
        mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if((btnState & MK_RBUTTON) != 0)
    {
        // Make each pixel correspond to 0.2 unit in the scene.
        float dx = 0.05f*static_cast<float>(x - mLastMousePos.x);
        float dy = 0.05f*static_cast<float>(y - mLastMousePos.y);

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp(mRadius, 5.0f, 150.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}
 
void CrateApp::OnKeyboardInput(const GameTimer& gt)
{
}
 
void CrateApp::UpdateShadowTransform(const GameTimer& gt)
{

}
void CrateApp::UpdateCamera(const GameTimer& gt)
{
	// Convert Spherical to Cartesian coordinates.
	mEyePos.x = mRadius*sinf(mPhi)*cosf(mTheta);
	mEyePos.z = mRadius*sinf(mPhi)*sinf(mTheta);
	mEyePos.y = mRadius*cosf(mPhi);
	ViewMatrix.UpdateViewMatrix(mEyePos,mTargetPos);

	//------------------------------------
	float NearFarLength = Far - Near;
	
	int CascadeCount = 4;
	float  Exp = 4.0f;
	float Scale = 1.2;
	float Piece = (1.0 - pow(Exp, CascadeCount)) / (1.0 - Exp);

	float FarPre = Near;
	float FrustumScale = 1.0 / Piece;
	for (int i = 0; i < CascadeCount; ++i)
	{
		
		float NearNew = FarPre;
		float FarNew = NearFarLength * FrustumScale * Scale;
		FarPre = FarNew;

		FrustumScale = FrustumScale * Exp;

		//float Far0 = NearFarLength * (1.0 / 85.0) * Scale;
		BoundSphere BoundSphere0 = BuildBoundSphere(FoVAngleY, AspectRatio(), NearNew, FarNew);

		//Compute Project Matrix
		float l = BoundSphere0.Center.x - BoundSphere0.Radius;
		float r = BoundSphere0.Center.x + BoundSphere0.Radius;
		float t = BoundSphere0.Center.y + BoundSphere0.Radius;
		float b = BoundSphere0.Center.y - BoundSphere0.Radius;
		float n = BoundSphere0.Center.z - BoundSphere0.Radius;
		float f = BoundSphere0.Center.z + BoundSphere0.Radius;
		XMMATRIX lightProj = XDirectx::XXMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
		XMStoreFloat4x4(&mLightProj, lightProj);

		//Compute Light Pos
		XMFLOAT3 LightDirStore = { -1,1,1 };
		XMVECTOR LightDir = DirectX::XMVector3Normalize(XMLoadFloat3(&LightDirStore));
		XMVECTOR lightPos = XMLoadFloat3(&BoundSphere0.Center) + LightDir * BoundSphere0.Radius * 1.1;
		XMFLOAT3 lightPosStore; XMStoreFloat3(&lightPosStore, lightPos);
		LightMatrix[i].Create(mLightProj, lightPosStore, BoundSphere0.Center);
	}

	
}

void CrateApp::AnimateMaterials(const GameTimer& gt)
{
	
}

void CrateApp::UpdateObjectCBs(const GameTimer& gt)
{
	for(auto& e : mAllRitems)
	{
		XMMATRIX world = XMLoadFloat4x4(&e->World);
		//XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
		//XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
		mFrameResource.get()->ObjectConstantBuffer[e->ObjCBIndex].get()->
			UpdateData(&objConstants, sizeof(ObjectConstants), 0);
	}

}

void CrateApp::UpdateMaterialCBs(const GameTimer& gt)
{
	for(auto& e : mMaterials)
	{
		Material* mat = e.second.get();
	
		MaterialConstants matConstants;
		matConstants.Metallic = mat->Metallic;
		matConstants.Specular = mat->Specular;
		matConstants.Roughness = mat->Roughness;
		matConstants.TextureScale = mat->TextureScale;
		mFrameResource.get()->MaterialConstantBuffer[mat->MatCBIndex].get()->
			UpdateData(&matConstants, sizeof(MaterialConstants), 0);
	}
}

void CrateApp::UpdateMainPassCB(const GameTimer& gt)
{
	//BasePass
	memcpy(&mMainPassCB.ViewProj, &ViewMatrix.GetViewProjectionMatrixTranspose(), sizeof(DirectX::XMFLOAT4X4));
	mFrameResource->PassConstantBuffer.get()->UpdateData(&mMainPassCB, sizeof(PassConstants), 0);
	
	//Current For LightPass , will combine to BasePass for future
	ViewCB.ScreenToTranslatedWorld = ViewMatrix.GetScreenToTranslatedWorldTranPose();
	ViewCB.InvDeviceZToWorldZTransform = CreateInvDeviceZToWorldZTransform(ViewMatrix.GetProjectionMatrix());
	ViewCB.WorldCameraOrigin = ViewMatrix.GetViewOrigin();
	ViewConstantBuffer.get()->UpdateData(
		&ViewCB,
		sizeof(ViewConstantBufferTable), 0);
	
	cbHZBins.DispatchThreadIdToBufferUV = XMFLOAT4(1.0 / 512.0, 1.0 / 512.0, 1.0, 1.0);
	RHICbbHZB.get()->UpdateData(&cbHZBins, sizeof(cbHZB), 0);

	//Shadow Pass
	for (uint32 i = 0; i < 4; i++)
	{
		memcpy(&ShadowPassConstant[i].View, &LightMatrix[i].GetViewMatrixTranspose(), sizeof(DirectX::XMFLOAT4X4));
		memcpy(&ShadowPassConstant[i].Proj, &LightMatrix[i].GetProjectionMatrixTranspose(), sizeof(DirectX::XMFLOAT4X4));
		ShadowPassConstantBuffer[i].get()->UpdateData(&ShadowPassConstant[i], sizeof(ShadowPassConstants), 0);
	}



	//ShadowMaskPass
	//TODO combine ShadowMaskPassViewConstantBuffer with ViewConstantBuffer
	ViewCB.BufferSizeAndInvSize =
		XMFLOAT4(mClientWidth, mClientHeight,
			1.0f / mClientWidth, 1.0f / mClientHeight);
	ShadowMaskPassViewConstantBuffer.get()->UpdateData(&ViewCB, sizeof(ViewConstantBufferTable), 0);

	XMFLOAT4X4 TempProject = ViewMatrix.GetProjectionMatrix();
	//XMFLOAT4X4 TempView = ViewMatrix.GetViewMatrix();
	XMFLOAT4X4 ScreenToClip = XDirectx::GetIdentityMatrix();
	ScreenToClip.m[2][2] = TempProject.m[2][2];
	ScreenToClip.m[3][2] = TempProject.m[3][2];
	ScreenToClip.m[2][3] = 1.0f;
	ScreenToClip.m[3][3] = 0.0f;
	
	for (uint32 i = 0; i < 4; i++)
	{
		XMMATRIX ScreenToShadowMatrix = XMLoadFloat4x4(&ScreenToClip);
		ScreenToShadowMatrix = XMMatrixMultiply(ScreenToShadowMatrix, XMLoadFloat4x4(&ViewMatrix.GetViewProjectionMatrixInverse()));
		ScreenToShadowMatrix = XMMatrixMultiply(ScreenToShadowMatrix, XMLoadFloat4x4(&LightMatrix[i].GetViewProjectionMatrix()));
		XMStoreFloat4x4(&cbShadowMaskNoCommon[i].ScreenToShadowMatrix, XMMatrixTranspose(ScreenToShadowMatrix));
		cbShadowMaskNoCommon[i].x_offset = 0.25 * i;
		ShadowMaskNoCommonConstantBuffer[i].get()->UpdateData(&cbShadowMaskNoCommon[i], sizeof(cbShadowMaskNoCommonBuffer), 0);
	}

}


void CrateApp::LoadTextures()
{
	{
		int w, h, n;
		unsigned char* BaseColorData = stbi_load("E:/XEngine/XEnigine/Source/Shaders/T_Metal_Gold_D.TGA", &w, &h, &n, 0);
		if (n == 3) { X_Assert(false); }
		
		TextureMetalBaseColor = direct_ctx->CreateD3D12Texture2D(w, h,
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
			,ETextureCreateFlags(TexCreate_SRGB)
			, BaseColorData);
		stbi_image_free(BaseColorData);
	}

	{
		int w_n, h_n, n_n;
		unsigned char* NormalMapData = stbi_load("E:/XEngine/XEnigine/Source/Shaders/T_Metal_Gold_N.TGA", &w_n, &h_n, &n_n, 0);
		int NormalTexSize = w_n * h_n * 4;
		unsigned char* FourChannelData = new unsigned char[NormalTexSize];
		X_Assert(n_n == 3);
		for (uint32 i = 0, k = 0; i < NormalTexSize; i += 4, k += 3)
		{
			FourChannelData[i + 0] = NormalMapData[k + 0];
			FourChannelData[i + 1] = NormalMapData[k + 1];
			FourChannelData[i + 2] = NormalMapData[k + 2];
			FourChannelData[i + 3] = 0b11111111;
		}
		TextureMetalNormal = direct_ctx->CreateD3D12Texture2D(w_n, h_n,
			DXGI_FORMAT_R8G8B8A8_UNORM
			,ETextureCreateFlags(TexCreate_None)
			, FourChannelData);
		stbi_image_free(NormalMapData);
		delete[] FourChannelData;
	}

	{
		int w_r, h_r, n_r;
		unsigned char* RoughnessMapData = stbi_load("E:/XEngine/XEnigine/Source/Shaders/T_MacroVariation.TGA", &w_r, &h_r, &n_r, 0);
		int RoughnessTexSize = w_r * h_r * 4;
		unsigned char* FourChannelData = new unsigned char[RoughnessTexSize];
		X_Assert(n_r == 3);
		for (uint32 i = 0, k = 0; i < RoughnessTexSize; i += 4, k += 3)
		{
			FourChannelData[i + 0] = RoughnessMapData[k + 0];
			FourChannelData[i + 1] = RoughnessMapData[k + 1];
			FourChannelData[i + 2] = RoughnessMapData[k + 2];
			FourChannelData[i + 3] = 0b11111111;
		}
		TextureRoughness = direct_ctx->CreateD3D12Texture2D(w_r, h_r,
			DXGI_FORMAT_R8G8B8A8_UNORM
			, ETextureCreateFlags(TexCreate_None)
			, FourChannelData);
		stbi_image_free(RoughnessMapData);
		delete[] FourChannelData;
	}

	{
		int w, h, n;
		unsigned char* BaseColorData = stbi_load("E:/XEngine/XEnigine/Source/Shaders/T_Rock_Sandstone_D.TGA", &w, &h, &n, 0);
		if (n == 3) { X_Assert(false); }
		
		TextureWoodBaseColor = direct_ctx->CreateD3D12Texture2D(w, h,
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
			, ETextureCreateFlags(TexCreate_SRGB)
			, BaseColorData);
		stbi_image_free(BaseColorData);
	}
	
	{
		int w_n, h_n, n_n;
		unsigned char* NormalMapData = stbi_load("E:/XEngine/XEnigine/Source/Shaders/T_Rock_Sandstone_N.TGA", &w_n, &h_n, &n_n, 0);
		int NormalTexSize = w_n * h_n * 4;
		unsigned char* FourChannelData = new unsigned char[NormalTexSize];
		X_Assert(n_n == 3);
		for (uint32 i = 0, k = 0; i < NormalTexSize; i += 4, k += 3)
		{
			FourChannelData[i + 0] = NormalMapData[k + 0];
			FourChannelData[i + 1] = NormalMapData[k + 1];
			FourChannelData[i + 2] = NormalMapData[k + 2];
			FourChannelData[i + 3] = 0b11111111;
		}
		TextureWoodNormal = direct_ctx->CreateD3D12Texture2D(w_n, h_n,
			DXGI_FORMAT_R8G8B8A8_UNORM
			, ETextureCreateFlags(TexCreate_None)
			, FourChannelData);
		stbi_image_free(NormalMapData);
		delete[] FourChannelData;
	}


	{
		TextureGBufferA = direct_ctx->CreateD3D12Texture2D(mClientWidth, mClientHeight,
			DXGI_FORMAT_R32G32B32A32_FLOAT
			,ETextureCreateFlags(TexCreate_RenderTargetable)
			, nullptr);

		TextureGBufferB = direct_ctx->CreateD3D12Texture2D(mClientWidth, mClientHeight,
			DXGI_FORMAT_R32G32B32A32_FLOAT
			, ETextureCreateFlags(TexCreate_RenderTargetable)
			, nullptr);

		TextureGBufferC = direct_ctx->CreateD3D12Texture2D(mClientWidth, mClientHeight,
			DXGI_FORMAT_R32G32B32A32_FLOAT
			, ETextureCreateFlags(TexCreate_RenderTargetable)
			, nullptr);

		TextureGBufferD = direct_ctx->CreateD3D12Texture2D(mClientWidth, mClientHeight,
			DXGI_FORMAT_R8G8B8A8_UNORM
			, ETextureCreateFlags(TexCreate_RenderTargetable)
			, nullptr);

		TextureSceneColorDeffered = direct_ctx->CreateD3D12Texture2D(mClientWidth, mClientHeight,
			DXGI_FORMAT_R32G32B32A32_FLOAT
			, ETextureCreateFlags(TexCreate_RenderTargetable)
			, nullptr);

		ShadowTexture0 = direct_ctx->CreateD3D12Texture2D(ShadowMapWidth, ShadowMapHeight,
			DXGI_FORMAT_R24G8_TYPELESS
			, ETextureCreateFlags(TexCreate_DepthStencilTargetable | TexCreate_ShaderResource)
			, nullptr);

		ShadowMaskTexture = direct_ctx->CreateD3D12Texture2D(mClientWidth, mClientHeight,
			DXGI_FORMAT_R8G8B8A8_UNORM
			, ETextureCreateFlags(TexCreate_RenderTargetable | TexCreate_ShaderResource)
			, nullptr);

		FurthestHZBOutput0 = direct_ctx->CreateD3D12Texture2D(512, 512,
			DXGI_FORMAT_R16_FLOAT
			, ETextureCreateFlags(TexCreate_UAV | TexCreate_ShaderResource)
			, nullptr);
	}

}

void CrateApp::BuildRootSignature()
{
	//PrePass
	{
		XPipelineRegisterBoundCount pipeline_register_count;
		memset(&pipeline_register_count, 0, sizeof(XPipelineRegisterBoundCount));
		
		pipeline_register_count.register_count[EShaderType::SV_Vertex].ConstantBufferCount
			= mShaders["PrePassVS"].GetCBVCount();
		PrePassRootSig.Create(&Device, pipeline_register_count);
	}

	{
		XPipelineRegisterBoundCount pipeline_register_count;
		memset(&pipeline_register_count, 0, sizeof(XPipelineRegisterBoundCount));

		pipeline_register_count.register_count[EShaderType::SV_Compute].UnorderedAccessCount
			= mShaders["HZBPassCS"].GetUAVCount();
		pipeline_register_count.register_count[EShaderType::SV_Compute].ShaderResourceCount
			= mShaders["HZBPassCS"].GetSRVCount();
		pipeline_register_count.register_count[EShaderType::SV_Compute].ConstantBufferCount
			= mShaders["HZBPassCS"].GetCBVCount();
		HZBPassRootSig.Create(&Device, pipeline_register_count);
	}

	{
		XPipelineRegisterBoundCount pipeline_register_count;
		memset(&pipeline_register_count, 0, sizeof(XPipelineRegisterBoundCount));

		pipeline_register_count.register_count[EShaderType::SV_Vertex].ConstantBufferCount
			= mShaders["ShadowPassVS"].GetCBVCount();

		ShadowPassRootSig.Create(&Device, pipeline_register_count);
	}

	//BasePass
	{
		XPipelineRegisterBoundCount pipeline_register_count;
		memset(&pipeline_register_count, 0, sizeof(XPipelineRegisterBoundCount));

		pipeline_register_count.register_count[EShaderType::SV_Vertex].ConstantBufferCount
			= mShaders["standardVS"].GetCBVCount();
		
		
		pipeline_register_count.register_count[EShaderType::SV_Pixel].ConstantBufferCount
			= mShaders["opaquePS"].GetCBVCount();
		pipeline_register_count.register_count[EShaderType::SV_Pixel].ShaderResourceCount
			= mShaders["opaquePS"].GetSRVCount();

		d3d12_root_signature.Create(&Device, pipeline_register_count);
	}

	{
		XPipelineRegisterBoundCount pipeline_register_count;
		memset(&pipeline_register_count, 0, sizeof(XPipelineRegisterBoundCount));

		pipeline_register_count.register_count[EShaderType::SV_Vertex].ConstantBufferCount
			= mShaders["ShadowMaskVS"].GetCBVCount();

		pipeline_register_count.register_count[EShaderType::SV_Pixel].ConstantBufferCount
			= mShaders["ShadowMaskPS"].GetCBVCount();
		pipeline_register_count.register_count[EShaderType::SV_Pixel].ShaderResourceCount
			= mShaders["ShadowMaskPS"].GetSRVCount();

		ShadowMaskPassRootSig.Create(&Device, pipeline_register_count);
		
	}
	//LightPass
	{
		XPipelineRegisterBoundCount pipeline_register_count;
		memset(&pipeline_register_count, 0, sizeof(XPipelineRegisterBoundCount));

		pipeline_register_count.register_count[EShaderType::SV_Vertex].ConstantBufferCount
			= mShaders["LightPassVS"].GetCBVCount();


		pipeline_register_count.register_count[EShaderType::SV_Pixel].ConstantBufferCount
			= mShaders["LightPassPS"].GetCBVCount();
		pipeline_register_count.register_count[EShaderType::SV_Pixel].ShaderResourceCount
			= mShaders["LightPassPS"].GetSRVCount();

		LightPassRootSig.Create(&Device, pipeline_register_count);
	}

	//FullScreenPass
	{
		XPipelineRegisterBoundCount pipeline_register_count;
		memset(&pipeline_register_count, 0, sizeof(XPipelineRegisterBoundCount));
		pipeline_register_count.register_count[EShaderType::SV_Pixel].ShaderResourceCount
			= mShaders["fullScreenPS"].GetSRVCount();

		FullScreenRootSig.Create(&Device, pipeline_register_count);
	}
}

void CrateApp::BuildShadersAndInputLayout()
{
	{
		mShaders["PrePassVS"].CreateShader(EShaderType::SV_Vertex);
		mShaders["PrePassVS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/DepthOnlyVertexShader.hlsl", nullptr, "VS", "vs_5_1");
		mShaders["PrePassVS"].ShaderReflect();

		mShaders["PrePassPS"].CreateShader(EShaderType::SV_Pixel);
		mShaders["PrePassPS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/DepthOnlyVertexShader.hlsl", nullptr, "PS", "ps_5_1");
		mShaders["PrePassPS"].ShaderReflect();
	}

	{
		mShaders["HZBPassCS"].CreateShader(EShaderType::SV_Compute);
		mShaders["HZBPassCS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/HZB.hlsl", nullptr, "HZBBuildCS", "cs_5_1");
		mShaders["HZBPassCS"].ShaderReflect();
	}

	{
		mShaders["ShadowPassVS"].CreateShader(EShaderType::SV_Vertex);
		mShaders["ShadowPassVS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/ShadowDepthShader.hlsl", nullptr, "VS", "vs_5_1");
		mShaders["ShadowPassVS"].ShaderReflect();

		mShaders["ShadowPassPS"].CreateShader(EShaderType::SV_Pixel);
		mShaders["ShadowPassPS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/ShadowDepthShader.hlsl", nullptr, "PS", "ps_5_1");
		mShaders["ShadowPassPS"].ShaderReflect();
	}

	{
		mShaders["standardVS"].CreateShader(EShaderType::SV_Vertex);
		mShaders["standardVS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/BasePassPixelShader.hlsl", nullptr, "VS", "vs_5_1");
		mShaders["standardVS"].ShaderReflect();

		mShaders["opaquePS"].CreateShader(EShaderType::SV_Pixel);
		mShaders["opaquePS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/BasePassPixelShader.hlsl", nullptr, "PS", "ps_5_1");
		mShaders["opaquePS"].ShaderReflect();
	}

	{
		mShaders["ShadowMaskVS"].CreateShader(EShaderType::SV_Vertex);
		mShaders["ShadowMaskVS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/ShadowProjectionShader.hlsl", nullptr, "VS", "vs_5_1");
		mShaders["ShadowMaskVS"].ShaderReflect();

		mShaders["ShadowMaskPS"].CreateShader(EShaderType::SV_Pixel);
		mShaders["ShadowMaskPS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/ShadowProjectionShader.hlsl", nullptr, "PS", "ps_5_1");
		mShaders["ShadowMaskPS"].ShaderReflect();
	}

	{
		mShaders["LightPassVS"].CreateShader(EShaderType::SV_Vertex);
		mShaders["LightPassVS"].CompileShader(
			L"E:/XEngine/XEnigine/Source/Shaders/DeferredLightVertexShaders.hlsl", 
			nullptr, "DeferredLightVertexMain", "vs_5_1");
		mShaders["LightPassVS"].ShaderReflect();

		mShaders["LightPassPS"].CreateShader(EShaderType::SV_Pixel);
		mShaders["LightPassPS"].CompileShader(
			L"E:/XEngine/XEnigine/Source/Shaders/DeferredLightPixelShaders.hlsl", 
			nullptr, "DeferredLightPixelMain", "ps_5_1");
		mShaders["LightPassPS"].ShaderReflect();
	}
	
	{
		mShaders["fullScreenVS"].CreateShader(EShaderType::SV_Vertex);
		mShaders["fullScreenVS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/fullScreen.hlsl", nullptr, "VS", "vs_5_1");
		mShaders["fullScreenVS"].ShaderReflect();

		mShaders["fullScreenPS"].CreateShader(EShaderType::SV_Pixel);
		mShaders["fullScreenPS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/fullScreen.hlsl", nullptr, "PS", "ps_5_1");
		mShaders["fullScreenPS"].ShaderReflect();
	}

	//LPCSTR SemanticName;
	//UINT SemanticIndex;
	//DXGI_FORMAT Format;
	//UINT InputSlot;
	//UINT AlignedByteOffset;
	//D3D12_INPUT_CLASSIFICATION InputSlotClass;
	//UINT InstanceDataStepRate;
	mBasePassLayout =
	{
		{ "ATTRIBUTE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "ATTRIBUTE", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "ATTRIBUTE", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "ATTRIBUTE", 3, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

    mFullScreenInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
}

void CrateApp::BuildShapeGeometry()
{
	//step1
    GeometryGenerator geoGen;
	{
		GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5, 20, 20);//geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
		GeometryGenerator::MeshData grid = geoGen.CreateGrid(40.0f, 60.0f, 60, 40);

		//step2
		UINT sphereVertexOffset = 0;
		UINT gridVertexOffset = (UINT)sphere.Vertices.size();

		UINT sphereIndexOffset = 0;
		UINT gridIndexOffset = (UINT)sphere.Indices32.size();

		//step3
		SubmeshGeometry sphereSubmesh;
		sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
		sphereSubmesh.StartIndexLocation = 0;
		sphereSubmesh.BaseVertexLocation = 0;

		SubmeshGeometry gridSubmesh;
		gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
		gridSubmesh.StartIndexLocation = gridIndexOffset;
		gridSubmesh.BaseVertexLocation = gridVertexOffset;

		//step4 vertices
		auto totalVertexCount =
			sphere.Vertices.size() +
			grid.Vertices.size();

		std::vector<Vertex> vertices(totalVertexCount);

		uint32 k = 0;
		for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
		{
			vertices[k].Position = DirectX::XMFLOAT4(
				sphere.Vertices[i].Position.x, 
				sphere.Vertices[i].Position.y, 
				sphere.Vertices[i].Position.z, 
				1.0f);
			
			vertices[k].TangentX = sphere.Vertices[i].TangentU;
			vertices[k].TangentZ = DirectX::XMFLOAT4(
				sphere.Vertices[i].Normal.x,
				sphere.Vertices[i].Normal.y,
				sphere.Vertices[i].Normal.z,
				1.0f);
			vertices[k].TexCoord = sphere.Vertices[i].TexC;
		}

		for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
		{
			vertices[k].Position = DirectX::XMFLOAT4(
				grid.Vertices[i].Position.x,
				grid.Vertices[i].Position.y,
				grid.Vertices[i].Position.z,
				1.0f);

			vertices[k].TangentX = grid.Vertices[i].TangentU;
			vertices[k].TangentZ = DirectX::XMFLOAT4(
				grid.Vertices[i].Normal.x,
				grid.Vertices[i].Normal.y,
				grid.Vertices[i].Normal.z,
				1.0f);
			vertices[k].TexCoord = grid.Vertices[i].TexC;
		}

		//step4 indices
		std::vector<std::uint16_t> indices;
		indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
		indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		auto geo = std::make_unique<MeshGeometry>();
		geo->Name = "shapeGeo";

		//CPU
		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
		CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
		CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		//GPU
		geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

		geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

		geo->VertexByteStride = sizeof(Vertex);
		geo->VertexBufferByteSize = vbByteSize;
		geo->IndexFormat = DXGI_FORMAT_R16_UINT;
		geo->IndexBufferByteSize = ibByteSize;

		geo->DrawArgs["sphere"] = sphereSubmesh;
		geo->DrawArgs["grid"] = gridSubmesh;
		mGeometries[geo->Name] = std::move(geo);
	}
	
	{
		GeometryGenerator::MeshData fullScreenQuad = geoGen.CreateFullScreenQuad();
		SubmeshGeometry fullScreenQuadSubmesh;
		fullScreenQuadSubmesh.IndexCount = (UINT)fullScreenQuad.Indices32.size();
		fullScreenQuadSubmesh.StartIndexLocation = 0;
		fullScreenQuadSubmesh.BaseVertexLocation = 0;

		std::vector<FullScreenVertex> quadVertices(fullScreenQuad.Vertices.size());
		for (size_t i = 0; i < fullScreenQuad.Vertices.size(); ++i)
		{
			quadVertices[i].Pos = DirectX::XMFLOAT2(
				fullScreenQuad.Vertices[i].Position.x,
				fullScreenQuad.Vertices[i].Position.y);
			quadVertices[i].TexC = fullScreenQuad.Vertices[i].TexC;
		}
		std::vector<std::uint16_t> quadindices;
		quadindices.insert(quadindices.end(), std::begin(fullScreenQuad.GetIndices16()), std::end(fullScreenQuad.GetIndices16()));
		const UINT vbByteSize = (UINT)quadVertices.size() * sizeof(FullScreenVertex);
		const UINT ibByteSize = (UINT)quadindices.size() * sizeof(std::uint16_t);

		auto geo = std::make_unique<MeshGeometry>();
		geo->Name = "fullQuad";

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
		CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), quadVertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
		CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), quadindices.data(), ibByteSize);

		geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), quadVertices.data(), vbByteSize, geo->VertexBufferUploader);

		geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), quadindices.data(), ibByteSize, geo->IndexBufferUploader);

		geo->VertexByteStride = sizeof(FullScreenVertex);
		geo->VertexBufferByteSize = vbByteSize;
		geo->IndexFormat = DXGI_FORMAT_R16_UINT;
		geo->IndexBufferByteSize = ibByteSize;

		geo->DrawArgs["quad"] = fullScreenQuadSubmesh;
		mGeometries[geo->Name] = std::move(geo);
	}
	
}

void CrateApp::BuildPSOs()
{
	//PrePass
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC DepthOnlyPSODesc;

		ZeroMemory(&DepthOnlyPSODesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		DepthOnlyPSODesc.InputLayout = { mBasePassLayout.data(), (UINT)mBasePassLayout.size() };
		DepthOnlyPSODesc.pRootSignature = PrePassRootSig.GetDXRootSignature();
		DepthOnlyPSODesc.VS =
		{
			reinterpret_cast<BYTE*>(mShaders["PrePassVS"].GetByteCode()->GetBufferPointer()),
			mShaders["PrePassVS"].GetByteCode()->GetBufferSize()
		};
		DepthOnlyPSODesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["PrePassPS"].GetByteCode()->GetBufferPointer()),
			mShaders["PrePassPS"].GetByteCode()->GetBufferSize()
		};

		//TODO
		DepthOnlyPSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		DepthOnlyPSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		DepthOnlyPSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		DepthOnlyPSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;

		DepthOnlyPSODesc.SampleMask = UINT_MAX;
		DepthOnlyPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		DepthOnlyPSODesc.NumRenderTargets = 0;
		DepthOnlyPSODesc.SampleDesc.Count = 1;
		DepthOnlyPSODesc.SampleDesc.Quality = 0;
		DepthOnlyPSODesc.DSVFormat = mDepthStencilFormat;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&DepthOnlyPSODesc, IID_PPV_ARGS(&mDepthOnlyPSO)));
	}


	//HZB Pass
	{
		//ID3D12RootSignature* pRootSignature;
		//D3D12_SHADER_BYTECODE CS;
		//UINT NodeMask;
		//D3D12_CACHED_PIPELINE_STATE CachedPSO;
		//D3D12_PIPELINE_STATE_FLAGS Flags;
		D3D12_COMPUTE_PIPELINE_STATE_DESC HZBPassPSODesc;
		ZeroMemory(&HZBPassPSODesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
		HZBPassPSODesc.pRootSignature = HZBPassRootSig.GetDXRootSignature();
		HZBPassPSODesc.CS = {
			reinterpret_cast<BYTE*>(mShaders["HZBPassCS"].GetByteCode()->GetBufferPointer()),
			mShaders["HZBPassCS"].GetByteCode()->GetBufferSize()
		};
		HZBPassPSODesc.NodeMask = 0;
		//HZBPassPSODesc.CachedPSO=
		HZBPassPSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		ThrowIfFailed(md3dDevice->CreateComputePipelineState(&HZBPassPSODesc, IID_PPV_ARGS(&HZBPSO)));
	}

	//ShadowPass
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC ShadowPSODesc;

		ZeroMemory(&ShadowPSODesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		ShadowPSODesc.InputLayout = { mBasePassLayout.data(), (UINT)mBasePassLayout.size() };
		ShadowPSODesc.pRootSignature = PrePassRootSig.GetDXRootSignature();
		ShadowPSODesc.VS =
		{
			reinterpret_cast<BYTE*>(mShaders["ShadowPassVS"].GetByteCode()->GetBufferPointer()),
			mShaders["ShadowPassVS"].GetByteCode()->GetBufferSize()
		};
		ShadowPSODesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["ShadowPassPS"].GetByteCode()->GetBufferPointer()),
			mShaders["ShadowPassPS"].GetByteCode()->GetBufferSize()
		};

		//TODO
		ShadowPSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		ShadowPSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		ShadowPSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		ShadowPSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;

		ShadowPSODesc.SampleMask = UINT_MAX;
		ShadowPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		ShadowPSODesc.NumRenderTargets = 0;
		ShadowPSODesc.SampleDesc.Count = 1;
		ShadowPSODesc.SampleDesc.Quality = 0;
		ShadowPSODesc.DSVFormat = mDepthStencilFormat;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&ShadowPSODesc, IID_PPV_ARGS(&ShadowPSO)));
	}

	//BasePass
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

		ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		opaquePsoDesc.InputLayout = { mBasePassLayout.data(), (UINT)mBasePassLayout.size() };
		opaquePsoDesc.pRootSignature = d3d12_root_signature.GetDXRootSignature();
		opaquePsoDesc.VS =
		{
			reinterpret_cast<BYTE*>(mShaders["standardVS"].GetByteCode()->GetBufferPointer()),
			mShaders["standardVS"].GetByteCode()->GetBufferSize()
		};
		opaquePsoDesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["opaquePS"].GetByteCode()->GetBufferPointer()),
			mShaders["opaquePS"].GetByteCode()->GetBufferSize()
		};

		//TODO
		opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		opaquePsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;

		opaquePsoDesc.SampleMask = UINT_MAX;
		opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		opaquePsoDesc.NumRenderTargets = 4;
		opaquePsoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		opaquePsoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		opaquePsoDesc.RTVFormats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		opaquePsoDesc.RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;
		opaquePsoDesc.SampleDesc.Count = 1;
		opaquePsoDesc.SampleDesc.Quality = 0;
		opaquePsoDesc.DSVFormat = mDepthStencilFormat;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mOpaquePSO)));
	}
	
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC ShadowMaskPsoDesc;

		ZeroMemory(&ShadowMaskPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		ShadowMaskPsoDesc.InputLayout = { mFullScreenInputLayout.data(), (UINT)mFullScreenInputLayout.size() };
		ShadowMaskPsoDesc.pRootSignature = ShadowMaskPassRootSig.GetDXRootSignature();
		ShadowMaskPsoDesc.VS =
		{
			reinterpret_cast<BYTE*>(mShaders["ShadowMaskVS"].GetByteCode()->GetBufferPointer()),
			mShaders["ShadowMaskVS"].GetByteCode()->GetBufferSize()
		};
		ShadowMaskPsoDesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["ShadowMaskPS"].GetByteCode()->GetBufferPointer()),
			mShaders["ShadowMaskPS"].GetByteCode()->GetBufferSize()
		};
		ShadowMaskPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		ShadowMaskPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		ShadowMaskPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		ShadowMaskPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		ShadowMaskPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;


		ShadowMaskPsoDesc.SampleMask = UINT_MAX;
		ShadowMaskPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		ShadowMaskPsoDesc.NumRenderTargets = 1;
		ShadowMaskPsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		ShadowMaskPsoDesc.SampleDesc.Count = 1;
		ShadowMaskPsoDesc.SampleDesc.Quality = 0;
		ShadowMaskPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&ShadowMaskPsoDesc, IID_PPV_ARGS(&ShadowMaskPSO)));
		
	}
	//LightPass
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC LightPassPsoDesc;

		ZeroMemory(&LightPassPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		LightPassPsoDesc.InputLayout = { mFullScreenInputLayout.data(), (UINT)mFullScreenInputLayout.size() };
		LightPassPsoDesc.pRootSignature = LightPassRootSig.GetDXRootSignature();
		LightPassPsoDesc.VS =
		{
			reinterpret_cast<BYTE*>(mShaders["LightPassVS"].GetByteCode()->GetBufferPointer()),
			mShaders["LightPassVS"].GetByteCode()->GetBufferSize()
		};
		LightPassPsoDesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["LightPassPS"].GetByteCode()->GetBufferPointer()),
			mShaders["LightPassPS"].GetByteCode()->GetBufferSize()
		};
		LightPassPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		LightPassPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		LightPassPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		LightPassPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		LightPassPsoDesc.SampleMask = UINT_MAX;
		LightPassPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		LightPassPsoDesc.NumRenderTargets = 1;
		LightPassPsoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		LightPassPsoDesc.SampleDesc.Count = 1;
		LightPassPsoDesc.SampleDesc.Quality = 0;
		LightPassPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&LightPassPsoDesc, IID_PPV_ARGS(&LightPassPso)));
	}

	//FullScreenPass
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC FullScreenPsoDesc;

		ZeroMemory(&FullScreenPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		FullScreenPsoDesc.InputLayout = { mFullScreenInputLayout.data(), (UINT)mFullScreenInputLayout.size() };
		FullScreenPsoDesc.pRootSignature = FullScreenRootSig.GetDXRootSignature();
		FullScreenPsoDesc.VS =
		{
			reinterpret_cast<BYTE*>(mShaders["fullScreenVS"].GetByteCode()->GetBufferPointer()),
			mShaders["fullScreenVS"].GetByteCode()->GetBufferSize()
		};
		FullScreenPsoDesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["fullScreenPS"].GetByteCode()->GetBufferPointer()),
			mShaders["fullScreenPS"].GetByteCode()->GetBufferSize()
		};
		FullScreenPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		FullScreenPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		FullScreenPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		FullScreenPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		FullScreenPsoDesc.SampleMask = UINT_MAX;
		FullScreenPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		FullScreenPsoDesc.NumRenderTargets = 1;
		FullScreenPsoDesc.RTVFormats[0] = mBackBufferFormat;
		FullScreenPsoDesc.SampleDesc.Count = 1;
		FullScreenPsoDesc.SampleDesc.Quality = 0;
		FullScreenPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&FullScreenPsoDesc, IID_PPV_ARGS(&FullScreenPSO)));
	}
}


void CrateApp::BuildMaterials()
{
	{
		auto metalmat = std::make_unique<Material>();
		metalmat->Name = "metal";
		metalmat->MatCBIndex = 0;
		metalmat->Metallic = 0.8f;
		metalmat->Specular = 0.0;
		metalmat->Roughness = 0.4f;
		metalmat->TextureScale = 1.0f;
		metalmat->TextureBaseColor = TextureMetalBaseColor;
		metalmat->TextureNormal = TextureMetalNormal;
		metalmat->TextureRoughness = TextureRoughness;
		mMaterials["metal"] = std::move(metalmat);
	}

	{
		auto woodCrate = std::make_unique<Material>();
		woodCrate->Name = "wood";
		woodCrate->MatCBIndex = 1;
		woodCrate->Metallic = 0.0;
		woodCrate->Specular = 0.5;
		woodCrate->Roughness = 0.8f;
		woodCrate->TextureScale = 5.0f;
		woodCrate->TextureBaseColor = TextureWoodBaseColor;
		woodCrate->TextureNormal = TextureWoodNormal;
		woodCrate->TextureRoughness = TextureRoughness;
		mMaterials["wood"] = std::move(woodCrate);
	}

}

void CrateApp::BuildRenderItems()
{

	{
		auto sphereBackRitem = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&sphereBackRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(0.0f, -1.5f, 0.0f));
		XMStoreFloat4x4(&sphereBackRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		sphereBackRitem->ObjCBIndex = 0;
		sphereBackRitem->Mat = mMaterials["metal"].get();
		sphereBackRitem->Geo = mGeometries["shapeGeo"].get();
		sphereBackRitem->IndexCount = sphereBackRitem->Geo->DrawArgs["sphere"].IndexCount;
		sphereBackRitem->StartIndexLocation = sphereBackRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
		sphereBackRitem->BaseVertexLocation = sphereBackRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;
		mAllRitems.push_back(std::move(sphereBackRitem));

		auto sphereRitem = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&sphereRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(0.0f, 0.5f, 0.0f));
		XMStoreFloat4x4(&sphereRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		sphereRitem->ObjCBIndex = 1;
		sphereRitem->Mat = mMaterials["metal"].get();
		sphereRitem->Geo = mGeometries["shapeGeo"].get();
		sphereRitem->IndexCount = sphereRitem->Geo->DrawArgs["sphere"].IndexCount;
		sphereRitem->StartIndexLocation = sphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
		sphereRitem->BaseVertexLocation = sphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;
		mAllRitems.push_back(std::move(sphereRitem));


		auto gridRitem = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(8.0f, 8.0f, 1.0f));
		gridRitem->ObjCBIndex = 2;
		gridRitem->Mat = mMaterials["wood"].get();
		gridRitem->Geo = mGeometries["shapeGeo"].get();
		gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
		gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
		gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
		mAllRitems.push_back(std::move(gridRitem));

		// All the render items are opaque.
		for (auto& e : mAllRitems)
			mOpaqueRitems.push_back(e.get());
	}

	{
		LightPassItem = std::make_unique<RenderItem>();
		LightPassItem->ObjCBIndex = 3;
		//fullScreenItem->Mat = mMaterials["woodCrate"].get();
		LightPassItem->Geo = mGeometries["fullQuad"].get();
		LightPassItem->IndexCount = LightPassItem->Geo->DrawArgs["quad"].IndexCount;
		LightPassItem->StartIndexLocation = LightPassItem->Geo->DrawArgs["quad"].StartIndexLocation;
		LightPassItem->BaseVertexLocation = LightPassItem->Geo->DrawArgs["quad"].BaseVertexLocation;
	}

	{
		fullScreenItem = std::make_unique<RenderItem>();
		fullScreenItem->ObjCBIndex = 0;//No  Use
		//fullScreenItem->Mat = mMaterials["woodCrate"].get();
		fullScreenItem->Geo = mGeometries["fullQuad"].get();
		fullScreenItem->IndexCount = fullScreenItem->Geo->DrawArgs["quad"].IndexCount;
		fullScreenItem->StartIndexLocation = fullScreenItem->Geo->DrawArgs["quad"].StartIndexLocation;
		fullScreenItem->BaseVertexLocation = fullScreenItem->Geo->DrawArgs["quad"].BaseVertexLocation;
	}
	
	
}


int main()
{

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(703);
	int* a = new int(5);
	try
	{
		CrateApp theApp;
		if (!theApp.Initialize())
			return 0;



		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
	
	_CrtDumpMemoryLeaks();
}



