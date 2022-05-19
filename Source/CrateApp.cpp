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
	virtual void Update(const GameTimer& gt)override;
	virtual void Renderer(const GameTimer& gt)override;

	void UpdateCamera(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	void LoadTextures();

	XBoundSphere BoundSphere0;

	GCamera CamIns;
	//RendererViewInfo RViewInfo;

	XVector3 LightDir = { -1 / sqrtf(3.0f),1/ sqrtf(3.0f),1/ sqrtf(3.0f) };
	XVector3 LightColor = { 1,1,1 };
	float LightIntensity = 7.0f;


	XDeferredShadingRenderer DeferredShadingRenderer;
private:
private:
	std::vector<std::shared_ptr<GGeomertry>>RenderGeos;
private:
	uint64 FrameNum = 0;
	float Far = 1000.0f;
	float Near = 1.0f;
	float FoVAngleY = 0.25f * X_PI;
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

	BoundSphere0.Center = XVector3(0, 0, 0);
	BoundSphere0.Radius = 96.0f;

	
	Application->UISetup();

	CamIns.SetPerspective(FoVAngleY, AspectRatio(), Near, Far);
	DeferredShadingRenderer.ViewInfoSetup(Application->ClientWidth, Application->ClientHeight, CamIns);
	DeferredShadingRenderer.Setup(
		RenderGeos,
		XBoundSphere{ BoundSphere0.Center ,BoundSphere0.Radius },
		LightDir,
		LightColor,
		LightIntensity);

	RHICmdList.Execute();
	return true;
}


void CrateApp::Update(const GameTimer& gt)
{
	UpdateCamera(gt);
	UpdateMainPassCB(gt);
}

void CrateApp::Renderer(const GameTimer& gt)
{
	Application->UINewFrame();
	DeferredShadingRenderer.Rendering(RHICmdList);
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
}

void CrateApp::UpdateMainPassCB(const GameTimer& gt)
{
	DeferredShadingRenderer.ViewInfoUpdate(CamIns);
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

	MainInit::InitAfterRHI();
}


void CrateApp::TempDelete()
{
	MainInit::Destroy();
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

static XApplication* Application = nullptr;
class TestMain
{
public:
	
	//XApplication* Application;
	XRHICommandList RHICmdList;
	HWND mhMainWnd = nullptr;

	XBoundSphere SceneBoundingSphere;
	XDeferredShadingRenderer DeferredShadingRenderer;

	std::vector<std::shared_ptr<GGeomertry>>RenderGeos;

	//Camera
	GCamera CamIns;
	float Far = 1000.0f;
	float Near = 1.0f;
	float FoVAngleY = 0.25f * X_PI;

	//Light
	XVector3 LightDir = { -1 / sqrtf(3.0f),1 / sqrtf(3.0f),1 / sqrtf(3.0f) };
	XVector3 LightColor = { 1,1,1 };
	float LightIntensity = 7.0f;

	void SceneBuild()
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
	}

	void MainInit()
	{
		MainInit::Init();

		Application = new XWindowsApplication();
		Application->PreInitial();
		Application->CreateAppWindow();
		Application->InitRHI();
		mhMainWnd = (HWND)Application->GetPlatformHandle();
		RHICmdList = GRHICmdList;

		SceneBoundingSphere.Center = XVector3(0, 0, 0);
		SceneBoundingSphere.Radius = 96.0f;

		RHICmdList.Open();
		SceneBuild();
		MainInit::InitAfterRHI();
		Application->UISetup();

		float AspectRatio = static_cast<float>(Application->ClientWidth) / static_cast<float>(Application->ClientHeight);
		CamIns.SetPerspective(FoVAngleY, AspectRatio, Near, Far);
		DeferredShadingRenderer.ViewInfoSetup(Application->ClientWidth, Application->ClientHeight, CamIns);
		DeferredShadingRenderer.Setup(RenderGeos,
			XBoundSphere{ SceneBoundingSphere.Center ,SceneBoundingSphere.Radius },
			LightDir, LightColor, LightIntensity);
		RHICmdList.Execute();
	}

	void MainRun()
	{
		MSG msg = { 0 };

		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				DeferredShadingRenderer.ViewInfoUpdate(CamIns);
				camss = &CamIns;
				Application->UINewFrame();
				DeferredShadingRenderer.Rendering(RHICmdList);
			}
		}
	}

	void MainDstroy()
	{
		MainInit::Destroy();
		RHIRelease();
	}

	~TestMain()
	{
		//DeferredShadingRenderer.~XDeferredShadingRenderer();
		
	}

	void MainFun()
	{
		MainInit();
		MainRun();
		MainDstroy();
	}
};


int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(4086);
	int* a = new int(5);

	{
		TestMain TestMainIns;
		TestMainIns.MainFun();
		TestReflectAndArchive();
	}
	{
		delete Application;
	}

	_CrtDumpMemoryLeaks();
}


