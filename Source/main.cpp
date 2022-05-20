//***************************************************************************************
// CrateApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include <memory>
#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb_image.h"
#include "Runtime/Core/MainInit.h"
#include "Runtime/Engine/ResourcecConverter.h"
#include "Runtime/Render/DeferredShadingRenderer.h"
#include "Runtime/ApplicationCore/Windows/WindowsApplication.h"

static GCamera* camss = nullptr;
static void TempUpdate2()
{
	if (XAppInput::InputPtr->GetMousePressed(EInputKey::MOUSE_RIGHT))
	{
		camss->ProcessMouseMove(XAppInput::InputPtr->GetMouseDelta().X, XAppInput::InputPtr->GetMouseDelta().Y);
	}
}
XAppInput::FunPtr XAppInput::TempUpFun = TempUpdate2;
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
	XVector3 LightDir = { 1 / sqrtf(3.0f),1 / sqrtf(3.0f),1 / sqrtf(3.0f) };
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
	}
	{
		delete Application;
	}

	_CrtDumpMemoryLeaks();
}

//void TestReflectAndArchive()
//{
//	std::cout << GTexture::StaticReflectionInfo.GetProperty(0)->PropertyName << std::endl;
//	std::wstring FileName = XPath::ProjectResourceSavedDir() + L"/T_Rock_Sandstone_D111.xasset";
//	std::shared_ptr<XArchiveBase>ArchiveWriterTex = XFileManagerGeneric::CreateFileWriter(FileName.c_str());
//	GTexture2D Texture;
//	Texture.LoadTextureFromImage("E:/XEngine/XEnigine/Source/Shaders/T_Rock_Sandstone_D.TGA");
//	Texture.ArchiveImpl(*ArchiveWriterTex);
//	ArchiveWriterTex->Close();
//
//	std::shared_ptr<XArchiveBase>ArchiveReaderTex = XFileManagerGeneric::CreateFileReader(FileName.c_str());
//	GTexture2D Texture2;
//	Texture2.ArchiveImpl(*ArchiveReaderTex);
//	X_Assert(Texture2.SizeX == Texture.SizeX);
//}


