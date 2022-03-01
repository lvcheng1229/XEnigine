// XEngine.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define _CRTDBG_MAP_ALLOC
#include<stdlib.h>
#include<crtdbg.h>


#include <iostream>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <wrl.h>
#include <windows.h>

#include <array>
#include <vector>

#include "d3dx12.h"

#include "d3dcompiler.h"
#include <DirectXColors.h>

#include "Runtime/Useless/DDSTextureLoader.h"

void fun2();
std::vector<UINT8> GenerateTextureData();
std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
void Draw();
void Updata()
{
	Draw();

}

struct WindowData
{
	//std::string Title;
	unsigned int Width = 800, Height = 600;
};
WindowData m_Data;
HWND mhMainWnd = nullptr;

void Run()
{
	MSG msg = { };
	static int num = 50;
	//while (num--)
	while (1)
	{
		if (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Updata();
	}
	DestroyWindow(mhMainWnd);
}


void InitWindow()
{
	WNDCLASSEX wc;
	wc = { sizeof(WNDCLASSEX),
				CS_CLASSDC,
				WindowProc,
				0L, 0L,
				GetModuleHandle(NULL),
				NULL, NULL, NULL, NULL,
				L"MainWindow",
				NULL };

	RegisterClassEx(&wc);

	mhMainWnd = CreateWindow(
		wc.lpszClassName,
		L"WindowTitle",
		WS_OVERLAPPEDWINDOW,

		//Position          Size
		100, 100, m_Data.Width, m_Data.Height,

		NULL,
		NULL,
		wc.hInstance,
		NULL);


	ShowWindow(mhMainWnd, SW_SHOWDEFAULT);
	UpdateWindow(mhMainWnd);
}

#include "Runtime/D3D12RHI/DX12GIAdapter.h"
#include "Runtime/D3D12RHI/D3D12PhysicDevice.h"
#include "Runtime/D3D12RHI/D3D12AbstractDevice.h"
#include "Runtime/D3D12RHI/D3D12CommandQueue.h"

XDXGIAdapter Adapter;
XD3D12PhysicDevice Device;
XD3D12AbstractDevice abstrtact_device;
DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
//DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

XD3D12CommandQueue* direct_cmd_queue;
XD3DDirectContex* direct_ctx;
Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;




struct ObjectConstants
{
	DirectX::XMFLOAT4X4 WorldViewProj = DirectX::XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
};

//swap chain
static const int SwapChainBufferCount = 2;
int mCurrBackBuffer = 0;
Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

XDxRefCount<ID3D12RootSignature> mRootSignature = nullptr;
XDxRefCount<ID3D12PipelineState> mPSO = nullptr;
XDxRefCount <ID3D12Resource>m_vertexBuffer = nullptr;
XDxRefCount<ID3D12Resource> UploadconstantBuffer;
XDxRefCount<ID3D12DescriptorHeap> mCbvHeap;
XDxRefCount<ID3D12DescriptorHeap> mSrvDescriptorHeap;
D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
unsigned char* mMappedData = nullptr;

XD3D12DescriptorArray* rt_desc_array;
XD3D12DescriptorArray* ds_desc_array;

//viewport
//D3D12_VIEWPORT mScreenViewport;
//D3D12_RECT mScissorRect;

#include <type_traits>
template<typename T>
requires(!std::is_lvalue_reference_v<T>)
T* get_rvalue_ptr(T&& v) {
	return &v;
}

#include <memory>
void InitRHI()
{
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	Adapter.Create();
	Device.Create(&Adapter);
	abstrtact_device.Create(&Device);
	direct_cmd_queue = abstrtact_device.GetCmdQueueByType(D3D12_COMMAND_LIST_TYPE_DIRECT);
	mCommandQueue = direct_cmd_queue->GetDXCommandQueue();

	//Step3
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_Data.Width ;
	sd.BufferDesc.Height =m_Data.Height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = mBackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count =1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2;
	sd.OutputWindow = mhMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ThrowIfFailed(Adapter.GetDXFactory()->CreateSwapChain(
		mCommandQueue.Get(),
		&sd,
		mSwapChain.GetAddressOf()));


	
	rt_desc_array = abstrtact_device.GetRenderTargetDescArray();
	ds_desc_array = abstrtact_device.GetDepthStencilDescArray();
	//Step5
	direct_cmd_queue->CommandQueueWaitFlush();
	for (int i = 0; i < SwapChainBufferCount; ++i)
		mSwapChainBuffer[i].Reset();
	mDepthStencilBuffer.Reset();
	mCurrBackBuffer = 0;

	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
		Device.GetDXDevice()->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rt_desc_array->GetCPUDescPtrByIndex(i));
	}

	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_Data.Width;
	depthStencilDesc.Height = m_Data.Height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = mDepthStencilFormat;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality =0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = mDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	Device.GetDXDevice()->CreateCommittedResource(
		get_rvalue_ptr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf()));
	
	Device.GetDXDevice()->CreateDepthStencilView(mDepthStencilBuffer.Get(), nullptr, ds_desc_array->GetCPUDescPtrBegin());
	
	direct_ctx = abstrtact_device.GetDirectContex();
	direct_ctx->Create(&Device);
	direct_ctx->OpenCmdList();
	mCommandList = direct_ctx->GetCmdList()->GetDXCmdList();
	mCommandList->ResourceBarrier(1, get_rvalue_ptr(CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE)));
	
#pragma region name
	//Step 1: build root signature
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		0); // register t0

	//CD3DX12_DESCRIPTOR_RANGE cbvTable;
	//cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);


	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);

	//static Sampler 
	auto staticSamplers = GetStaticSamplers();


	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(2, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	XDxRefCount<ID3DBlob> signature;
	XDxRefCount<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(Device.GetDXDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));


	if (error != nullptr)
	{
		::OutputDebugStringA((char*)error->GetBufferPointer());
	}

	//Step2: Load and compile the shaders
	XDxRefCount<ID3DBlob> vertexShader;
	XDxRefCount<ID3DBlob> pixelShader;

	//#if defined(SR_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	//#else
	//	UINT compileFlags = 0;
	//#endif
	XDxRefCount<ID3DBlob> errorsV;
	XDxRefCount<ID3DBlob> errorsP;

	const std::wstring ShaderDir = L"E:/XEngine/XEnigine/Source/Shaders/";
	const std::wstring filename = L"shaders.hlsl";
	ThrowIfFailed(D3DCompileFromFile((ShaderDir + filename).c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorsV));
	ThrowIfFailed(D3DCompileFromFile((ShaderDir + filename).c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &errorsP));

	if (errorsV != nullptr)
		OutputDebugStringA((char*)errorsV->GetBufferPointer());

	if (errorsP != nullptr)
		OutputDebugStringA((char*)errorsP->GetBufferPointer());

	//Step3:Create the vertex input layout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	//Step4:Fill out a pipeline state description, using the helper structures available, then create the graphics pipeline state:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.DSVFormat = mDepthStencilFormat;

	ThrowIfFailed(Device.GetDXDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));


	//Step6: Create the vertex buffer
	struct SR_Float2
	{
		float x, y;
		SR_Float2(float x, float y) :x(x), y(y) {}
	};
	struct SR_Float3
	{
		float x, y, z;
		SR_Float3(float x, float y, float z) :x(x), y(y), z(z) {}
	};
	struct SR_Float4
	{
		float x, y, z, w;
		SR_Float4(float x, float y, float z, float w) :x(x), y(y), z(z), w(w) {}
	};
	struct SR_Vertex
	{
		SR_Float3 position;
		SR_Float3 color;
		SR_Float2 uv;
		SR_Vertex(SR_Float3 pos, SR_Float3 col, SR_Float2 uv) :position(pos), color(col), uv(uv) {}
	};
	SR_Vertex triangleVertices[] =
	{
		{ { 0.0f, 0.25f , 0.0f }, { 1.0f, 0.0f, 0.0f},{0.5,1.0} },
		{ { 0.25f, -0.25f , 0.0f }, { 0.0f, 1.0f, 0.0f},{0.0,0.0} },
		{ { -0.25f, -0.25f , 0.0f }, { 0.0f, 0.0f, 1.0f} ,{1.0,0.0}}

	};

	const UINT vertexBufferSize = sizeof(triangleVertices);

	//---------Upload Buffer Begin------------------------------------------------------------------------

	ThrowIfFailed(Device.GetDXDevice()->CreateCommittedResource(
		get_rvalue_ptr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		get_rvalue_ptr(CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize)),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)));


	XDxRefCount<ID3D12Resource>VertexBufferUploader = nullptr;
	ThrowIfFailed(Device.GetDXDevice()->CreateCommittedResource(
		get_rvalue_ptr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
		D3D12_HEAP_FLAG_NONE,
		get_rvalue_ptr(CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&VertexBufferUploader)));

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = triangleVertices;
	subResourceData.RowPitch = vertexBufferSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;



	//ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	mCommandList->ResourceBarrier(1, get_rvalue_ptr(CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST)));
	UpdateSubresources<1>(mCommandList.Get(), m_vertexBuffer.Get(), VertexBufferUploader.Get(), 0, 0, 1, &subResourceData);
	mCommandList->ResourceBarrier(1, get_rvalue_ptr(CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ)));


	//---------Upload Buffer End------------------------------------------------------------------------

	//Step10: constant buffer

	UINT objCBByteSize = (sizeof(ObjectConstants) + 255) & ~255;
	ThrowIfFailed(Device.GetDXDevice()->CreateCommittedResource(
		get_rvalue_ptr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
		D3D12_HEAP_FLAG_NONE,
		get_rvalue_ptr(CD3DX12_RESOURCE_DESC::Buffer(objCBByteSize * 1)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&UploadconstantBuffer)));
	ThrowIfFailed(UploadconstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData)));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = UploadconstantBuffer->GetGPUVirtualAddress();

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(Device.GetDXDevice()->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&mCbvHeap)));

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = objCBByteSize;

	Device.GetDXDevice()->CreateConstantBufferView(
		&cbvDesc,
		mCbvHeap->GetCPUDescriptorHandleForHeapStart());


	//Step11: Set Constance buffer data
	DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
		0.25f * 3.14159265,
		static_cast<float>(m_Data.Width) / m_Data.Height,
		1.0f,
		1000.0f);


	DirectX::XMVECTOR pos = DirectX::XMVectorSet(0, 0, -3, 1.0f);
	DirectX::XMVECTOR target = DirectX::XMVectorZero();
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);

	DirectX::XMMATRIX worldViewProj = DirectX::XMLoadFloat4x4(new DirectX::XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f)) * view * P;

	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
	memcpy(&mMappedData[0], &objConstants, sizeof(ObjectConstants));

	//--------------------------------------------------------------------

	//Step12 : Texture
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(Device.GetDXDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	XDxRefCount<ID3D12Resource> textureUploadHeap;
	XDxRefCount<ID3D12Resource> m_texture;
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(Device.GetDXDevice(),
		mCommandList.Get(), L"E:/XEngine/XEnigine/Source/Shaders/bricks.dds",
		m_texture, textureUploadHeap));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto woodCrateTex = m_texture;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = woodCrateTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = woodCrateTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	Device.GetDXDevice()->CreateShaderResourceView(woodCrateTex.Get(), &srvDesc, hDescriptor);
	{
		//const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);
		//
		//ThrowIfFailed(Device.GetDXDevice()->CreateCommittedResource(
		//	get_rvalue_ptr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
		//	D3D12_HEAP_FLAG_NONE,
		//	get_rvalue_ptr(CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize)),
		//	D3D12_RESOURCE_STATE_GENERIC_READ,
		//	nullptr,
		//	IID_PPV_ARGS(&textureUploadHeap)));
		//
		//
		//
		//D3D12_RESOURCE_DESC textureDesc = {};
		//textureDesc.MipLevels = 1;
		//textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		//textureDesc.Width = 256;
		//textureDesc.Height = 256;
		//textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		//textureDesc.DepthOrArraySize = 1;
		//textureDesc.SampleDesc.Count = 1;
		//textureDesc.SampleDesc.Quality = 0;
		//textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		//
		//ThrowIfFailed(Device.GetDXDevice()->CreateCommittedResource(
		//	get_rvalue_ptr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		//	D3D12_HEAP_FLAG_NONE,
		//	&textureDesc,
		//	D3D12_RESOURCE_STATE_COPY_DEST,
		//	nullptr,
		//	IID_PPV_ARGS(&m_texture)));

		// Copy data to the intermediate upload heap and then schedule a copy 
		// from the upload heap to the Texture2D.
		//std::vector<UINT8> texture = GenerateTextureData();
	
		//D3D12_SUBRESOURCE_DATA textureData = {};
		//textureData.pData = &texture[0];
		//textureData.RowPitch = 256 * 4;
		//textureData.SlicePitch = textureData.RowPitch * 256;
		//
		//UpdateSubresources(mCommandList.Get(), m_texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
		//
		//mCommandList->ResourceBarrier(1, get_rvalue_ptr(CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)));
	
		// Describe and create a SRV for the texture.
		//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		//srvDesc.Format = textureDesc.Format;
		//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		//srvDesc.Texture2D.MipLevels = 1;
		//Device.GetDXDevice()->CreateShaderResourceView(m_texture.Get(), &srvDesc, mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}

	//Step8: initialize the vertex buffer view
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(SR_Vertex);
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
#pragma endregion comment
	//fun2();
	
	direct_ctx->CloseCmdList();

	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	
	direct_cmd_queue->CommandQueueWaitFlush();
	direct_ctx->ResetCmdAlloc();

	VertexBufferUploader = nullptr;

}

void fun2()
{
}

ID3D12Resource* CurrentBackBuffer()
{
	return mSwapChainBuffer[mCurrBackBuffer].Get();
}


void Draw()
{
	
	OutputDebugString(L" asd ");
	direct_ctx->ResetCmdAlloc();
	OutputDebugString(L" cr ");
	direct_ctx->OpenCmdList();
	mCommandList->SetPipelineState(mPSO.Get());



	D3D12_CPU_DESCRIPTOR_HANDLE rt_ptr = rt_desc_array->GetCPUDescPtrByIndex(mCurrBackBuffer);
	D3D12_CPU_DESCRIPTOR_HANDLE ds_ptr = ds_desc_array->GetCPUDescPtrByIndex(0);
	auto barraier = get_rvalue_ptr(CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	mCommandList->ResourceBarrier(1, barraier);

	direct_ctx->RHISetViewport(0.0f, 0.0f, 0.0f, m_Data.Width, m_Data.Height, 1.0f);
	mCommandList->ClearRenderTargetView(rt_ptr, DirectX::Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(ds_desc_array->GetCPUDescPtrByIndex(0), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	mCommandList->OMSetRenderTargets(1, &rt_ptr, true, &ds_ptr);

	
#pragma region ert
	mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	
	
	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);//Set SBC_SRV_UAV heap

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	mCommandList->SetGraphicsRootDescriptorTable(0, tex);
	mCommandList->SetGraphicsRootConstantBufferView(1, UploadconstantBuffer->GetGPUVirtualAddress());

	mCommandList->DrawInstanced(3, 1, 0, 0);
#pragma endregion comment1


	mCommandList->ResourceBarrier(1, get_rvalue_ptr(CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));
	ThrowIfFailed(mCommandList->Close());
	

	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	
	direct_cmd_queue->CommandQueueWaitFlush();

	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
	
	
	
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}
std::vector<UINT8> GenerateTextureData()
{
	const UINT rowPitch = 256 * 4;
	const UINT cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
	const UINT cellHeight = 256 >> 3;    // The height of a cell in the checkerboard texture.
	const UINT textureSize = rowPitch * 256;

	std::vector<UINT8> data(textureSize);
	UINT8* pData = &data[0];

	for (UINT n = 0; n < textureSize; n += 4)
	{
		UINT x = n % rowPitch;
		UINT y = n / rowPitch;
		UINT i = x / cellPitch;
		UINT j = y / cellHeight;

		if (i % 2 == j % 2)
		{
			pData[n] = 0xff;        // R
			pData[n + 1] = 0x00;    // G
			pData[n + 2] = 0x00;    // B
			pData[n + 3] = 0xff;    // A
		}
		else
		{
			pData[n] = 0xff;        // R
			pData[n + 1] = 0xff;    // G
			pData[n + 2] = 0x00;    // B
			pData[n + 3] = 0xff;    // A
		}
	}

	return data;
}
void FinalFunction()
{
	InitWindow();
	InitRHI();
	Run();

}
int main()
{
	//int* a = new int(5);
	FinalFunction();
	//2*40 byte caused by XD3DDevice
	_CrtDumpMemoryLeaks();
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
