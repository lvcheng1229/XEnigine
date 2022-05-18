//***************************************************************************************
// CrateApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include <memory>
#include <windows.h>
#include "UnitTest/d3dApp.h"


#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb_image.h"

#include "Runtime/RenderCore/GlobalShader.h"
#include "Runtime/RenderCore/ShaderParameter.h"
#include "Runtime/RenderCore/CommonRenderRresource.h"
#include "Runtime/RenderCore/Shader.h"
#include "Runtime/RenderCore/VertexFactory.h"

#include "Runtime/Core/XMath.h"
#include "Runtime/Render/SceneRendering.h"
#include "Runtime/Render/MeshPassProcessor.h"
#include "Runtime/Render/MaterialShader.h"
#include "Runtime/Render/BasePassRendering.h"

#include "Runtime/Engine/SceneView.h"
#include "Runtime/Engine/ShaderCompiler/ShaderCompiler.h"
#include "Runtime/Engine/Material/MaterialShared.h"

#include "Runtime/RHI/PipelineStateCache.h"
#include "Runtime/RHI/RHIStaticStates.h"

#include "Runtime/D3D12RHI/D3D12Shader.h"
#include "Runtime/D3D12RHI/D3D12OnlineDescArray.h"
#include "Runtime/D3D12RHI/D3D12OnlineDescArrayManager.h"
#include "Runtime/D3D12RHI/D3D12PlatformRHI.h"
#include "Runtime/D3D12RHI/D3D12Texture.h"

#include "Runtime/Core/Math/Math.h"


//ImGUI Begin
#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>

#include "Editor/EditorUI.h"
//#include "Runtime/Engine/UIBackend.h"
//ImGUI End


#include "Runtime/Core/MainInit.h"
#include "Runtime/Engine/Classes/Texture.h"
#include "Runtime/HAL/FileManagerGeneric.h"
#include "Runtime/Core/Misc/Path.h"

#include "Runtime/Core/ComponentNode/Camera.h"

#include "Runtime/Engine/Classes/Material.h"
#include "Runtime/Engine/ResourcecConverter.h"

#include "Runtime/Render/DeferredShadingRenderer.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;


struct BoundSphere
{
	XVector3 Center;
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
	void TempDelete()override;
	virtual void InitCamInfo()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Renderer(const GameTimer& gt)override;

	void UpdateCamera(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	void LoadTextures();
	void BuildPSOs();

	void TestExecute();

	BoundSphere BoundSphere0;

	GCamera CamIns;
	RendererViewInfo RViewInfo;

	XVector3 LightDir = { -1 / sqrtf(3.0f),1/ sqrtf(3.0f),1/ sqrtf(3.0f) };
	XVector3 LightColor = { 1,1,1 };
	float LightIntensity = 7.0f;

	struct ObjectConstants
	{
		XMatrix World;
		DirectX::XMFLOAT3 BoundingBoxCenter;
		float padding0 = 1.0f;
		DirectX::XMFLOAT3 BoundingBoxExtent;
		float padding1 = 1.0f;
	};

	XDeferredShadingRenderer DeferredShadingRenderer;
private:
	std::shared_ptr<XRHIStructBuffer>GlobalObjectStructBuffer;
	std::shared_ptr<XRHIShaderResourceView>GlobalObjectStructBufferSRV;

	struct cbCullingParametersStruct
	{
		XMatrix  ShdowViewProject;
		XPlane Planes[(int)ECameraPlane::CP_MAX];
		float commandCount;
	};
	cbCullingParametersStruct CullingParametersIns;
	std::shared_ptr<XRHIConstantBuffer>cbCullingParameters;
private:
	std::vector<std::shared_ptr<GGeomertry>>RenderGeos;
private:
	uint64 FrameNum = 0;
	float Far = 1000.0f;
	float Near = 1.0f;
	float FoVAngleY = 0.25f * X_PI;

private:
	XMatrix mLightProj = XMatrix::Identity;
	XMatrix mLightView = XMatrix::Identity;
	POINT mLastMousePos;
};

CrateApp::CrateApp()
{
}

CrateApp::~CrateApp()
{
}


//-------------------
bool CrateApp::Initialize()
{

	if (!D3DApp::Initialize())
		return false;

	RHICmdList.Open();

	LoadTextures();

	RViewInfo.ViewConstantBuffer = RHICreateConstantBuffer(sizeof(ViewConstantBufferData));
	RViewInfo.ViewWidth = Application->ClientWidth;
	RViewInfo.ViewHeight = Application->ClientHeight;

	cbCullingParameters = RHICreateConstantBuffer(sizeof(cbCullingParametersStruct));

	BuildPSOs();
	TestExecute();

	BoundSphere0.Center = XVector3(0, 0, 0);
	BoundSphere0.Radius = 96.0f;
	DeferredShadingRenderer.SceneBoundingSphere.Center = BoundSphere0.Center;
	DeferredShadingRenderer.SceneBoundingSphere.Radius = BoundSphere0.Radius;
	DeferredShadingRenderer.ShadowLightDir = LightDir;
	DeferredShadingRenderer.MainLightColor = LightColor;
	DeferredShadingRenderer.LightIntensity = LightIntensity;
	DeferredShadingRenderer.RenderGeos = RenderGeos;
	DeferredShadingRenderer.RViewInfo.ViewWidth = Application->ClientWidth;
	DeferredShadingRenderer.RViewInfo.ViewHeight = Application->ClientHeight;
	
	Application->UISetup();
	DeferredShadingRenderer.Setup();
	RHICmdList.Execute();
	

	return true;
}


void CrateApp::InitCamInfo()
{

	CamIns.SetPerspective(FoVAngleY, AspectRatio(), Near, Far);
	XMatrix mProj = CamIns.GetProjectMatrix();
	RViewInfo.ViewMats.Create(mProj, CamIns.GetEyePosition(), CamIns.GetTargetPosition());

	
}

void CrateApp::Update(const GameTimer& gt)
{
	UpdateCamera(gt);
	UpdateMainPassCB(gt);
}


void CrateApp::TestExecute()
{
		
	//ObjectConstants
	CullingParametersIns.commandCount = RenderGeos.size();
	
	uint32 ObjConstVecSize = sizeof(ObjectConstants) * RenderGeos.size();
	ObjectConstants* ConstantArray = (ObjectConstants*)std::malloc(ObjConstVecSize);
	
	for (int i = 0; i < RenderGeos.size(); i++)
	{
		auto& RG = RenderGeos[i];
	
		XBoundingBox BoudingBoxTans = RG->GetBoudingBoxWithTrans();
		ConstantArray[i].BoundingBoxCenter = BoudingBoxTans.Center;
		ConstantArray[i].BoundingBoxExtent = BoudingBoxTans.Extent;
		ConstantArray[i].World = RG->GetWorldTransform().GetCombineMatrix();
	}
	
	{
	
		FResourceVectorUint8 ObjectStructBufferData;
		ObjectStructBufferData.Data = ConstantArray;
		ObjectStructBufferData.SetResourceDataSize(ObjConstVecSize);
		XRHIResourceCreateData ObjectStructBufferResourceData(&ObjectStructBufferData);
		
		GlobalObjectStructBuffer = RHIcreateStructBuffer(
			sizeof(ObjectConstants),
			ObjConstVecSize,
			EBufferUsage((int)EBufferUsage::BUF_ShaderResource),
			ObjectStructBufferResourceData);
	
		GlobalObjectStructBufferSRV = RHICreateShaderResourceView(GlobalObjectStructBuffer.get());
	}
}

void CrateApp::Renderer(const GameTimer& gt)
{
	//ImGui_ImplWin32_NewFrame();
	Application->UINewFrame();
	DeferredShadingRenderer.Rendering(RHICmdList);

	RViewInfo = DeferredShadingRenderer.RViewInfo;
	RViewInfo.ViewConstantBuffer.get()->UpdateData(&RViewInfo.ViewCBCPUData, sizeof(ViewConstantBufferData), 0);
}


static GCamera* camss = nullptr;
static void TempUpdate2()
{
	if (XAppInput::InputPtr->GetMousePressed(EInputKey::MOUSE_RIGHT))
	{
		camss->ProcessMouseMove(XAppInput::InputPtr->GetMouseDelta().X, XAppInput::InputPtr->GetMouseDelta().Y);
	}
}
XAppInput::FunPtr XAppInput::TempUpFun = TempUpdate2;

void CrateApp::UpdateCamera(const GameTimer& gt)
{
	camss = &CamIns;

	RViewInfo.ViewMats.UpdateViewMatrix(CamIns.GetEyePosition(), CamIns.GetTargetPosition());

	{
		//BoundSphere BoundSphere0;


		//Compute Project Matrix
		float l = BoundSphere0.Center.x - BoundSphere0.Radius;
		float r = BoundSphere0.Center.x + BoundSphere0.Radius;
		float t = BoundSphere0.Center.y + BoundSphere0.Radius;
		float b = BoundSphere0.Center.y - BoundSphere0.Radius;
		float f = BoundSphere0.Radius * 2;
		float n = 0.1f;
		
		XMMATRIX lightProj = DirectX::XMMatrixOrthographicOffCenterLH(l, r, b, t, f, n);
		XMStoreFloat4x4(&mLightProj, lightProj);
		
		//Compute Light Pos
		XMVECTOR LightDirCom = DirectX::XMVector3Normalize(XMLoadFloat3(&LightDir));
		XMVECTOR lightPos = LightDirCom * BoundSphere0.Radius * 1.0;
		XVector3 lightPosStore; XMStoreFloat3(&lightPosStore, lightPos);
		
		XVector3 upTempp(0, 1, 0);
		XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, BoundSphere0.Center, upTempp);
		
		XMStoreFloat4x4(&mLightView, lightView);
	}
	
}

void CrateApp::UpdateMainPassCB(const GameTimer& gt)
{

	FrameNum++;
	RViewInfo.ViewCBCPUData.StateFrameIndexMod8 = FrameNum % 8;
	RViewInfo.ViewCBCPUData.ViewSizeAndInvSize = 
		XMFLOAT4(Application->ClientWidth, Application->ClientHeight, 
			1.0 / Application->ClientWidth, 1.0 / Application->ClientHeight);
	float LenSqrt = sqrt(LightDir.x * LightDir.x + LightDir.y * LightDir.y + LightDir.z * LightDir.z);
	RViewInfo.ViewCBCPUData.AtmosphereLightDirection = XMFLOAT4(LightDir.x / LenSqrt, LightDir.y / LenSqrt, LightDir.z / LenSqrt, 1.0f);
	RViewInfo.ViewCBCPUData.ViewToClip = RViewInfo.ViewMats.GetProjectionMatrix();
	RViewInfo.ViewCBCPUData.TranslatedViewProjectionMatrix = RViewInfo.ViewMats.GetTranslatedViewProjectionMatrix();
	RViewInfo.ViewCBCPUData.ScreenToWorld = RViewInfo.ViewMats.GetScreenToWorld();
	RViewInfo.ViewCBCPUData.ScreenToTranslatedWorld = RViewInfo.ViewMats.GetScreenToTranslatedWorld();
	RViewInfo.ViewCBCPUData.InvDeviceZToWorldZTransform = CreateInvDeviceZToWorldZTransform(RViewInfo.ViewMats.GetProjectionMatrix());
	RViewInfo.ViewCBCPUData.WorldCameraOrigin = RViewInfo.ViewMats.GetViewOrigin();
	RViewInfo.ViewCBCPUData.ViewProjectionMatrix = RViewInfo.ViewMats.GetViewProjectionMatrix();
	RViewInfo.ViewCBCPUData.ViewProjectionMatrixInverse = RViewInfo.ViewMats.GetViewProjectionMatrixInverse();
	RViewInfo.ViewCBCPUData.BufferSizeAndInvSize = 
		XMFLOAT4(Application->ClientWidth, Application->ClientHeight, 1.0f / Application->ClientWidth, 1.0f / Application->ClientHeight);
	
	DeferredShadingRenderer.LightViewMat = mLightView;
	DeferredShadingRenderer.LightProjMat = mLightProj;

	XMatrix mLightViewProj = mLightView * mLightProj;


	RViewInfo.ViewMats.GetPlanes(CullingParametersIns.Planes);
	CullingParametersIns.ShdowViewProject = mLightViewProj;
	cbCullingParameters->UpdateData(&CullingParametersIns, sizeof(cbCullingParametersStruct), 0);


	{
		DeferredShadingRenderer.GlobalObjectStructBufferSRV = GlobalObjectStructBufferSRV;
		DeferredShadingRenderer.cbCullingParameters = cbCullingParameters;
		DeferredShadingRenderer.RViewInfo = RViewInfo;
	}
}


void CrateApp::LoadTextures()
{
	std::shared_ptr<GGeomertry> DefaultSphere = TempCreateSphereGeoWithMat();
	DefaultSphere->GetGVertexBuffer()->CreateRHIBufferChecked();
	DefaultSphere->GetGIndexBuffer()->CreateRHIBufferChecked();
	DefaultSphere->SetWorldTranslate(XVector3(0, -0.5, 0));

	std::shared_ptr<GGeomertry> SphereInsUp = DefaultSphere->CreateGeoInstancewithMat();
	SphereInsUp->SetWorldTranslate(XVector3(0, 1.5, 0));

	std::shared_ptr<GGeomertry> SphereInsRight = DefaultSphere->CreateGeoInstancewithMat();
	SphereInsRight->SetWorldTranslate(XVector3(2, 1.5, 0));

	std::shared_ptr<GGeomertry> DefaultQuad = TempCreateQuadGeoWithMat();
	DefaultQuad->GetGVertexBuffer()->CreateRHIBufferChecked();
	DefaultQuad->GetGIndexBuffer()->CreateRHIBufferChecked();
	DefaultQuad->SetWorldTranslate(XVector3(0.0, 1.01, 0.0));

	RenderGeos.push_back(DefaultSphere);
	RenderGeos.push_back(SphereInsUp);
	RenderGeos.push_back(DefaultQuad);
	RenderGeos.push_back(SphereInsRight);

	MainInit::TempInit2();
}

void CrateApp::BuildPSOs()
{
	CompileGlobalShaderMap();
}

void CrateApp::TempDelete()
{
	if (GGlobalShaderMapping)
		delete GGlobalShaderMapping;
}




void TestReflectAndArchive()
{
	std::cout << GTexture::StaticReflectionInfo.GetProperty(0)->PropertyName << std::endl;
	std::wstring FileName = XPath::ProjectResourceSavedDir() + L"/T_Rock_Sandstone_D111.xasset";
	std::shared_ptr<XArchiveBase>ArchiveWriterTex = XFileManagerGeneric::CreateFileWriter(FileName.c_str());
	GTexture2D Texture;
	Texture.LoadTextureFromImage("E:/XEngine/XEnigine/Source/Shaders/T_Rock_Sandstone_D.TGA");
	Texture.ArchiveImpl(*ArchiveWriterTex);
	ArchiveWriterTex->Close();

	std::shared_ptr<XArchiveBase>ArchiveReaderTex = XFileManagerGeneric::CreateFileReader(FileName.c_str());
	GTexture2D Texture2;
	Texture2.ArchiveImpl(*ArchiveReaderTex);
	X_Assert(Texture2.SizeX == Texture.SizeX);
}

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(403);
	int* a = new int(5);

	{
		MainInit::Init();
		TestReflectAndArchive();

		CrateApp theApp;
		if (!theApp.Initialize())return 0;
		return theApp.Run();
	}

	_CrtDumpMemoryLeaks();
}


