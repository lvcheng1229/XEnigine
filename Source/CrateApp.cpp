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
	
private:
	XD3D12RootSignature d3d12_root_signature;
	std::shared_ptr<XRHITexture2D>Texture2DTest;

	std::shared_ptr<XRHITexture2D>TextureGBufferA;
	std::shared_ptr<XRHITexture2D>TextureGBufferB;
	std::shared_ptr<XRHITexture2D>TextureGBufferC;
	std::shared_ptr<XRHITexture2D>TextureGBufferD;

	XRHIRenderTargetView* RTViews[8];
private:
	//Full Screen Pass
	XD3D12RootSignature FullScreenRootSig;
	ComPtr<ID3D12PipelineState>FullScreenPSO = nullptr;
	std::unique_ptr<RenderItem> fullScreenItem;
private:
	float clear_color[4] = { 0, 0, 0, 0 };
	std::unique_ptr<FrameResource>mFrameResource;
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	//std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, XShader> mShaders;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

    ComPtr<ID3D12PipelineState> mOpaquePSO = nullptr;
 
	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	

	// Render items divided by PSO.
	std::vector<RenderItem*> mOpaqueRitems;

    PassConstants mMainPassCB;

	XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

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
	
	//for (int i = 0; i < mMaterials.size();i++)
	//{
	//	mFrameResource.get()->MaterialConstantBuffer.push_back(
	//		abstrtact_device.CreateUniformBuffer(d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants))));
	//}

	for (uint32 i = 0; i < mAllRitems.size();++i)
	{
		mFrameResource.get()->ObjectConstantBuffer.push_back(
			abstrtact_device.CreateUniformBuffer(d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants))));
	}
	
    
	BuildPSOs();

	OutputDebugString(L"1111\n");

	direct_ctx->CloseCmdList();
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	direct_cmd_queue->CommandQueueWaitFlush();
	OutputDebugString(L"2222\n");
    return true;
}
 
void CrateApp::OnResize()
{
    D3DApp::OnResize();
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
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

	//Pass1
	{
		direct_ctx->ResetCmdAlloc();
		direct_ctx->OpenCmdList();
		direct_ctx->RHISetViewport(0.0f, 0.0f, 0.0f, mClientWidth, mClientHeight, 1.0f);

		mCommandList->SetPipelineState(mOpaquePSO.Get());
		pass_state_manager->SetRootSignature(&d3d12_root_signature);

		RTViews[0] = static_cast<XD3D12Texture2D*>(TextureGBufferA.get())->GetRenderTargetView();
		RTViews[1] = static_cast<XD3D12Texture2D*>(TextureGBufferB.get())->GetRenderTargetView();
		RTViews[2] = static_cast<XD3D12Texture2D*>(TextureGBufferC.get())->GetRenderTargetView();
		RTViews[3] = static_cast<XD3D12Texture2D*>(TextureGBufferD.get())->GetRenderTargetView();

		direct_ctx->RHISetRenderTargets(4, 
			RTViews,
			DsView);
		direct_ctx->RHIClearMRT(true, true, clear_color, 1.0f, 0);
		
		for (size_t i = 0; i < mOpaqueRitems.size(); ++i)
		{
			auto& ri = mOpaqueRitems[i];

			direct_ctx->RHISetShaderConstantBuffer(
				mShaders["standardVS"].GetRHIGraphicsShader().get(),
				0,
				mFrameResource.get()->ObjectConstantBuffer[ri->ObjCBIndex].get());
			
			direct_ctx->RHISetShaderConstantBuffer(
				mShaders["standardVS"].GetRHIGraphicsShader().get(),
				1,
				mFrameResource.get()->PassConstantBuffer.get());

			//direct_ctx->RHISetShaderConstantBuffer(
			//	mShaders["standardVS"].GetRHIGraphicsShader().get(),
			//	2,
			//	mFrameResource.get()->MaterialConstantBuffer[ri->Mat->MatCBIndex].get());


			direct_ctx->RHISetShaderTexture(mShaders["opaquePS"].GetRHIGraphicsShader().get(), 0, Texture2DTest.get());

			//direct_ctx->RHISetShaderConstantBuffer(
			//	mShaders["opaquePS"].GetRHIGraphicsShader().get(),
			//	0,
			//	mFrameResource.get()->PassConstantBuffer.get());
			//
			//direct_ctx->RHISetShaderConstantBuffer(
			//	mShaders["opaquePS"].GetRHIGraphicsShader().get(),
			//	1,
			//	mFrameResource.get()->MaterialConstantBuffer[ri->Mat->MatCBIndex].get());


			mCommandList.Get()->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
			mCommandList.Get()->IASetIndexBuffer(&ri->Geo->IndexBufferView());
			pass_state_manager->ApplyCurrentStateToPipeline();

			direct_ctx->GetCmdList()->CmdListFlushBarrier();
			mCommandList.Get()->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		}


		
		direct_ctx->CloseCmdList();

		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		direct_cmd_queue->CommandQueueWaitFlush();
		pass_state_manager->ResetState();
	}
	
	
	//Pass2
	{
		direct_ctx->ResetCmdAlloc();
		direct_ctx->OpenCmdList();
		mCommandList->SetPipelineState(FullScreenPSO.Get());
		pass_state_manager->SetRootSignature(&FullScreenRootSig);

		direct_ctx->RHISetViewport(0.0f, 0.0f, 0.0f, mClientWidth, mClientHeight, 1.0f);
		
		RTViews[0] = viewport.GetCurrentBackRTView();
		direct_ctx->RHISetRenderTargets(1, RTViews, DsView);//TODO
		direct_ctx->RHIClearMRT(true, false, clear_color, 1.0f, 0);

		direct_ctx->RHISetShaderTexture(
			mShaders["fullScreenPS"].GetRHIGraphicsShader().get()
			,0
			, TextureGBufferA.get());

		mCommandList.Get()->IASetVertexBuffers(0, 1, &fullScreenItem->Geo->VertexBufferView());
		mCommandList.Get()->IASetIndexBuffer(&fullScreenItem->Geo->IndexBufferView());
		pass_state_manager->ApplyCurrentStateToPipeline();
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
 
void CrateApp::UpdateCamera(const GameTimer& gt)
{
	// Convert Spherical to Cartesian coordinates.
	mEyePos.x = mRadius*sinf(mPhi)*cosf(mTheta);
	mEyePos.z = mRadius*sinf(mPhi)*sinf(mTheta);
	mEyePos.y = mRadius*cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(mEyePos.x, mEyePos.y, mEyePos.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);
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
	//for(auto& e : mMaterials)
	//{
	//	Material* mat = e.second.get();
	//	XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);
	//
	//	MaterialConstants matConstants;
	//	matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
	//	matConstants.FresnelR0 = mat->FresnelR0;
	//	matConstants.Roughness = mat->Roughness;
	//	XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));
	//	
	//	mFrameResource.get()->MaterialConstantBuffer[mat->MatCBIndex].get()->
	//		UpdateData(&matConstants, sizeof(MaterialConstants), 0);
	//}
}

void CrateApp::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	//XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	//XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	//XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	//XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	//XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	//XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	//XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	//XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	//mMainPassCB.EyePosW = mEyePos;
	//mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	//mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	//mMainPassCB.NearZ = 1.0f;
	//mMainPassCB.FarZ = 1000.0f;
	//mMainPassCB.TotalTime = gt.TotalTime();
	//mMainPassCB.DeltaTime = gt.DeltaTime();
	//mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	//mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	//mMainPassCB.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
	//mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	//mMainPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
	//mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	//mMainPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

	mFrameResource->PassConstantBuffer.get()->UpdateData(&mMainPassCB, sizeof(PassConstants), 0);
}


void CrateApp::LoadTextures()
{
	int w, h, n;
	unsigned char* data = stbi_load("E:/XEngine/XEnigine/Source/Shaders/T_Metal_Gold_D.TGA", &w, &h, &n, 0);
	if (n == 3)
	{
		X_Assert(false);
	}

	Texture2DTest = direct_ctx->CreateD3D12Texture2D(w, h,
		ETextureCreateFlags(TexCreate_SRGB)
		, data);
	stbi_image_free(data);
	
	TextureGBufferA = direct_ctx->CreateD3D12Texture2D(mClientWidth, mClientHeight,
		ETextureCreateFlags(TexCreate_RenderTargetable)
		, nullptr);

	TextureGBufferB = direct_ctx->CreateD3D12Texture2D(mClientWidth, mClientHeight,
		ETextureCreateFlags(TexCreate_RenderTargetable)
		, nullptr);

	TextureGBufferC = direct_ctx->CreateD3D12Texture2D(mClientWidth, mClientHeight,
		ETextureCreateFlags(TexCreate_RenderTargetable)
		, nullptr);

	TextureGBufferD = direct_ctx->CreateD3D12Texture2D(mClientWidth, mClientHeight,
		ETextureCreateFlags(TexCreate_RenderTargetable)
		, nullptr);
}

void CrateApp::BuildRootSignature()
{
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
		pipeline_register_count.register_count[EShaderType::SV_Pixel].ShaderResourceCount
			= mShaders["fullScreenPS"].GetSRVCount();

		FullScreenRootSig.Create(&Device, pipeline_register_count);
	}
}

void CrateApp::BuildShadersAndInputLayout()
{
	mShaders["standardVS"].CreateShader(EShaderType::SV_Vertex);
	mShaders["standardVS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/BasePassPixelShader.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["standardVS"].ShaderReflect();

	mShaders["opaquePS"].CreateShader(EShaderType::SV_Pixel);
	mShaders["opaquePS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/BasePassPixelShader.hlsl", nullptr, "PS", "ps_5_1");
	mShaders["opaquePS"].ShaderReflect();
	
	mShaders["fullScreenVS"].CreateShader(EShaderType::SV_Vertex);
	mShaders["fullScreenVS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/fullScreen.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["fullScreenVS"].ShaderReflect();

	mShaders["fullScreenPS"].CreateShader(EShaderType::SV_Pixel);
	mShaders["fullScreenPS"].CompileShader(L"E:/XEngine/XEnigine/Source/Shaders/fullScreen.hlsl", nullptr, "PS", "ps_5_1");
	mShaders["fullScreenPS"].ShaderReflect();

    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
}

void CrateApp::BuildShapeGeometry()
{
	//step1
    GeometryGenerator geoGen;
	{
		GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5, 20, 20);//geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
		GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);

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
			vertices[i].Pos = sphere.Vertices[i].Position;
			vertices[i].Normal = sphere.Vertices[i].Normal;
			vertices[i].TexC = sphere.Vertices[i].TexC;
		}

		for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
		{
			vertices[k].Pos = grid.Vertices[i].Position;
			vertices[k].Normal = grid.Vertices[i].Normal;
			vertices[k].TexC = grid.Vertices[i].TexC;
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

		std::vector<Vertex> quadVertices(fullScreenQuad.Vertices.size());
		for (size_t i = 0; i < fullScreenQuad.Vertices.size(); ++i)
		{
			quadVertices[i].Pos = fullScreenQuad.Vertices[i].Position;
			quadVertices[i].Normal = fullScreenQuad.Vertices[i].Normal;
			quadVertices[i].TexC = fullScreenQuad.Vertices[i].TexC;
		}
		std::vector<std::uint16_t> quadindices;
		quadindices.insert(quadindices.end(), std::begin(fullScreenQuad.GetIndices16()), std::end(fullScreenQuad.GetIndices16()));
		const UINT vbByteSize = (UINT)quadVertices.size() * sizeof(Vertex);
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

		geo->VertexByteStride = sizeof(Vertex);
		geo->VertexBufferByteSize = vbByteSize;
		geo->IndexFormat = DXGI_FORMAT_R16_UINT;
		geo->IndexBufferByteSize = ibByteSize;

		geo->DrawArgs["quad"] = fullScreenQuadSubmesh;
		mGeometries[geo->Name] = std::move(geo);
	}
	
}

void CrateApp::BuildPSOs()
{
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

		ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
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
		opaquePsoDesc.SampleMask = UINT_MAX;
		opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		opaquePsoDesc.NumRenderTargets = 4;
		opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
		opaquePsoDesc.RTVFormats[1] = mBackBufferFormat;
		opaquePsoDesc.RTVFormats[2] = mBackBufferFormat;
		opaquePsoDesc.RTVFormats[3] = mBackBufferFormat;
		opaquePsoDesc.SampleDesc.Count = 1;
		opaquePsoDesc.SampleDesc.Quality = 0;
		opaquePsoDesc.DSVFormat = mDepthStencilFormat;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mOpaquePSO)));
	}
    
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC FullScreenPsoDesc;

		ZeroMemory(&FullScreenPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		FullScreenPsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
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
		FullScreenPsoDesc.DSVFormat = mDepthStencilFormat;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&FullScreenPsoDesc, IID_PPV_ARGS(&FullScreenPSO)));
	}
}


void CrateApp::BuildMaterials()
{
	//auto woodCrate = std::make_unique<Material>();
	//woodCrate->Name = "woodCrate";
	//woodCrate->MatCBIndex = 0;
	//woodCrate->DiffuseSrvHeapIndex = 0;
	//woodCrate->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//woodCrate->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	//woodCrate->Roughness = 0.2f;
	//
	//mMaterials["woodCrate"] = std::move(woodCrate);
}

void CrateApp::BuildRenderItems()
{
	{
		auto sphereRitem = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&sphereRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(0.0f, 0.5f, 0.0f));
		XMStoreFloat4x4(&sphereRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		sphereRitem->ObjCBIndex = 0;
		//sphereRitem->Mat = mMaterials["woodCrate"].get();
		sphereRitem->Geo = mGeometries["shapeGeo"].get();
		sphereRitem->IndexCount = sphereRitem->Geo->DrawArgs["sphere"].IndexCount;
		sphereRitem->StartIndexLocation = sphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
		sphereRitem->BaseVertexLocation = sphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;
		mAllRitems.push_back(std::move(sphereRitem));


		auto gridRitem = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(8.0f, 8.0f, 1.0f));
		gridRitem->ObjCBIndex = 1;
		//gridRitem->Mat = mMaterials["woodCrate"].get();
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
		fullScreenItem = std::make_unique<RenderItem>();
		fullScreenItem->ObjCBIndex = 0;
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



