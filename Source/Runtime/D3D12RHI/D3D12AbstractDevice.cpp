#include "D3D12AbstractDevice.h"
#include "Runtime/D3D12RHI/d3dx12.h"
#include "Core/AlignmentTemplates.h"
#include "D3D12Common.h"
#include <string>
XD3D12AbstractDevice::~XD3D12AbstractDevice()
{
	delete ComputeCmdQueue;
	delete DirectxCmdQueue;
}

void XD3D12AbstractDevice::Create(XD3D12PhysicDevice* PhysicalDeviceIn)
{
	//TODO
	//if dont reserve at start,then the address that ptr point to maybe change when resize is excuted
	TempResourceIndex = 0;
	ResourceManagerTempVec.resize(1000);

	PhysicalDevice = PhysicalDeviceIn;
	DirectxCmdQueue = new XD3D12CommandQueue(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
	ComputeCmdQueue = new XD3D12CommandQueue(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COMPUTE);
	DirectxCmdQueue->Create(PhysicalDeviceIn);
	ComputeCmdQueue->Create(PhysicalDeviceIn);

	uint32 temp_thread_num = 2;
	DirectCtxs.resize(temp_thread_num);
	for (int i = 0; i < temp_thread_num; ++i)
	{
		DirectCtxs[i].Create(this);
	}

	RenderTargetDescArrayManager.Create(PhysicalDeviceIn, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 128);
	DepthStencilDescArrayManager.Create(PhysicalDeviceIn, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 128);
	ShaderResourceDescArrayManager.Create(PhysicalDeviceIn, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128);


	XAllocConfig default_cfg;
	default_cfg.d3d12_heap_flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
	default_cfg.d3d12_heap_type = D3D12_HEAP_TYPE_DEFAULT;
	DefaultNonRtDsTextureHeapAlloc.Create(
		PhysicalDevice,
		default_cfg,
		128 * (1 << 20),
		(64 * 1024),
		AllocStrategy::PlacedResource);

	XAllocConfig upload_cfg;
	upload_cfg.d3d12_heap_type = D3D12_HEAP_TYPE_UPLOAD;
	upload_cfg.d3d12_resource_states = D3D12_RESOURCE_STATE_GENERIC_READ;
	UploadHeapAlloc.Create(
		PhysicalDevice,
		upload_cfg,
		128 * (1 << 20),
		(64 * 1024),
		AllocStrategy::ManualSubAllocation
	);

	ConstantBufferUploadHeapAlloc.Create(
		PhysicalDevice,
		upload_cfg,
		128 * (1 << 20),
		256,
		AllocStrategy::ManualSubAllocation
	);
}

std::shared_ptr<XD3D12ConstantBuffer> XD3D12AbstractDevice::CreateUniformBuffer(uint32 size)
{
	std::shared_ptr<XD3D12ConstantBuffer> ContantBuffer= std::make_shared<XD3D12ConstantBuffer>();
	XD3D12ResourceLocation& Location = ContantBuffer.get()->ResourceLocation;
	ConstantBufferUploadHeapAlloc.Allocate(size, 256, Location);
	return ContantBuffer;
}


XD3D12Texture2D* XD3D12AbstractDevice::CreateD3D12Texture2D(XD3D12DirectCommandList* x_cmd_list, uint32 width, uint32 height, DXGI_FORMAT format, ETextureCreateFlags flag,uint32 NumMipsIn,uint8* tex_data)
{
	bool bCreateShaderResource = true;
	bool bCreateRTV = false;
	bool bCreateDSV = false;
	bool bCreateUAV = false;


	auto cmd_list = x_cmd_list->GetDXCmdList();
	//ResourceManagerTempVec.push_back(XD3D12Resource());
	XD3D12Resource* TextureResource = &ResourceManagerTempVec[TempResourceIndex];
	//XLog(TextureResource);
	TempResourceIndex++;
	XLog("Memory Leak : CreateD3D12Texture2D");

	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Alignment = MIN_PLACED_BUFFER_SIZE;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.MipLevels = NumMipsIn;
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	//textureDesc.Layout = ;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	if (flag & ETextureCreateFlags::TexCreate_RenderTargetable)
	{
		textureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		bCreateRTV = true;
	}

	if (flag & ETextureCreateFlags::TexCreate_DepthStencilTargetable)
	{
		textureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		bCreateDSV = true;
	}

	if (bCreateDSV&& !(flag & TexCreate_ShaderResource))
	{
		textureDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
		bCreateShaderResource = false;
	}

	if (flag & TexCreate_UAV)
	{
		textureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		bCreateUAV = true;
	}

	const DXGI_FORMAT PlatformShaderResourceFormat = FindShaderResourceDXGIFormat(format);
	const DXGI_FORMAT PlatformDepthStencilFormat = FindDepthStencilDXGIFormat(format);

	const XD3D12ResourceTypeHelper Type(textureDesc, D3D12_HEAP_TYPE_DEFAULT);
	const D3D12_RESOURCE_STATES InitialState = Type.GetOptimalInitialState(false);

	const D3D12_RESOURCE_ALLOCATION_INFO Info = PhysicalDevice->GetDXDevice()->GetResourceAllocationInfo(0, 1, &textureDesc);
	XD3D12ResourceLocation default_location;

	if (bCreateRTV || bCreateDSV)
	{
		D3D12_CLEAR_VALUE clearValue;
		
		if (bCreateRTV)
		{
			clearValue.Format = PlatformShaderResourceFormat;
			clearValue.Color[0] = 0.0f; clearValue.Color[1] = 0.0f; clearValue.Color[2] = 0.0f; clearValue.Color[3] = 0.0f;
		}
		else if (bCreateDSV)
		{
			clearValue.Format = PlatformDepthStencilFormat;
			clearValue.DepthStencil.Stencil = 0;
			clearValue.DepthStencil.Depth = 0.0f;
		}
		const D3D12_HEAP_PROPERTIES HeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		PhysicalDevice->GetDXDevice()->CreateCommittedResource(
			&HeapProps,
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			InitialState,
			&clearValue,
			IID_PPV_ARGS(TextureResource->GetPtrToResourceAdress()));
		
		TextureResource->SetResourceState(InitialState);
	}
	else
	{
		bool res = DefaultNonRtDsTextureHeapAlloc.Allocate(
			Info.SizeInBytes, 
			D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, 
			default_location);
		X_Assert(res == true);

		const D3D12_RESOURCE_STATES ResourceState = (tex_data != nullptr ? D3D12_RESOURCE_STATE_COPY_DEST : InitialState);

		uint64 ResoureceAlignOffset = AlignArbitrary(
			DefaultNonRtDsTextureHeapAlloc.GetAllocationOffsetInBytes(default_location.GetBuddyAllocData()),
			D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

		
		ThrowIfFailed(PhysicalDevice->GetDXDevice()->CreatePlacedResource(
			default_location.GetBuddyAllocator()->GetDXHeap(), 
			ResoureceAlignOffset,
			&textureDesc,
			ResourceState,
			nullptr, IID_PPV_ARGS(TextureResource->GetPtrToResourceAdress())));
		//XLog(TextureResource->GetPtrToResourceAdress());
		TextureResource->SetResourceState(ResourceState);
	}

	{
		
		std::wstring str = L"xx" + std::to_wstring(TempResourceIndex) + L"xx\n";
		TextureResource->GetResource()->SetName(str.c_str());
	}
	
	

	XD3D12Texture2D* TextureRet = new XD3D12Texture2D();
	
	if (bCreateShaderResource)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = PlatformShaderResourceFormat;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = TextureResource->GetResource()->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		uint32 index_of_desc_in_heap;
		uint32 index_of_heap;
		ShaderResourceDescArrayManager.AllocateDesc(index_of_desc_in_heap, index_of_heap);

		XD3D12ShaderResourceView ShaderResourceView;
		ShaderResourceView.Create(
			PhysicalDevice, TextureResource,
			srvDesc, ShaderResourceDescArrayManager.compute_cpu_ptr(index_of_desc_in_heap, index_of_heap));
		TextureRet->SetShaderResourceView(ShaderResourceView);

	}

	if (bCreateUAV)
	{
		for (uint32 i = 0; i < NumMipsIn; i++)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc = {};
			UavDesc.Format = PlatformShaderResourceFormat;
			UavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			UavDesc.Texture2D.MipSlice = i;
			UavDesc.Texture2D.PlaneSlice = 0;

			uint32 index_of_desc_in_heap;
			uint32 index_of_heap;
			ShaderResourceDescArrayManager.AllocateDesc(index_of_desc_in_heap, index_of_heap);

			XD3D12UnorderedAcessView UnorderedAcessView;
			UnorderedAcessView.Create(
				PhysicalDevice, TextureResource,
				UavDesc, ShaderResourceDescArrayManager.compute_cpu_ptr(index_of_desc_in_heap, index_of_heap));
			TextureRet->SetUnorderedAcessView(UnorderedAcessView);
		}
	}

	if (bCreateRTV)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtDesc = {};
		rtDesc.Format = TextureResource->GetResource()->GetDesc().Format;
		rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtDesc.Texture2D.MipSlice = 0;
		rtDesc.Texture2D.PlaneSlice = 0;

		uint32 index_of_desc_in_heap;
		uint32 index_of_heap;
		RenderTargetDescArrayManager.AllocateDesc(index_of_desc_in_heap, index_of_heap);

		XD3D12RenderTargetView ShaderResourceView;
		ShaderResourceView.Create(
			PhysicalDevice, TextureResource,
			rtDesc, RenderTargetDescArrayManager.compute_cpu_ptr(index_of_desc_in_heap, index_of_heap));
		TextureRet->SetRenderTargetView(ShaderResourceView);

	}

	if (bCreateDSV)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC ds_desc;
		ds_desc.Format = PlatformDepthStencilFormat;
		ds_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		ds_desc.Flags = D3D12_DSV_FLAG_NONE;
		ds_desc.Texture2D.MipSlice = 0;
		
		uint32 index_of_desc_in_heap_ds;
		uint32 index_of_heap_ds;
		DepthStencilDescArrayManager.AllocateDesc(index_of_desc_in_heap_ds, index_of_heap_ds);

		XD3D12DepthStencilView DepthStencilView;
		DepthStencilView.Create(
			PhysicalDevice, TextureResource,
			ds_desc, DepthStencilDescArrayManager.compute_cpu_ptr(index_of_desc_in_heap_ds, index_of_heap_ds));
		TextureRet->SetDepthStencilView(DepthStencilView);
	}

	if (tex_data != nullptr)
	{
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(TextureResource->GetResource(), 0, 1);

		XD3D12ResourceLocation upload_location;
		bool res = UploadHeapAlloc.Allocate(uploadBufferSize, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, upload_location);
		X_Assert(res == true);

		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = tex_data;
		textureData.RowPitch = width * 4;
		textureData.SlicePitch = textureData.RowPitch * height;

		CopyDataFromUploadToDefaulHeap
		(cmd_list, TextureResource->GetResource(),
			UploadHeapAlloc.GetDXResource(), 
			UploadHeapAlloc.GetAllocationOffsetInBytes(upload_location.GetBuddyAllocData()),
			0, 1, &textureData);
		cmd_list->ResourceBarrier(1, &(CD3DX12_RESOURCE_BARRIER::Transition(TextureResource->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)));
		TextureResource->SetResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	return TextureRet;
}

XD3D12CommandQueue* XD3D12AbstractDevice::GetCmdQueueByType(D3D12_COMMAND_LIST_TYPE cmd_type)
{
	switch (cmd_type)
	{
	case D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT:
		return DirectxCmdQueue;
		break;
	case D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COMPUTE:
		return ComputeCmdQueue;
		break;
	default:
		X_Assert(false);
		break;
	}
}