#include "D3D12AbstractDevice.h"
#include "Runtime/D3D12RHI/d3dx12.h"
XD3D12AbstractDevice::~XD3D12AbstractDevice()
{
	delete direct_cmd_queue;
	delete compute_cmd_queue;
}

void XD3D12AbstractDevice::Create(XD3D12PhysicDevice* device_in)
{
	physic_device = device_in;
	direct_cmd_queue = new XD3D12CommandQueue(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
	compute_cmd_queue = new XD3D12CommandQueue(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COMPUTE);
	direct_cmd_queue->Create(device_in);
	compute_cmd_queue->Create(device_in);

	//D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	//rtvHeapDesc.NumDescriptors = 2;//swap chain buffer count
	//rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	//rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	//rtvHeapDesc.NodeMask = 0;
	//RenderTargetDescArray.Create(device_in, rtvHeapDesc);

	RenderTargetDescArrayManager.Create(device_in, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 128);

	//D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	//dsvHeapDesc.NumDescriptors = 1;
	//dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	//dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	//dsvHeapDesc.NodeMask = 0;
	//DepthStencilDescArray.Create(device_in, dsvHeapDesc);

	DepthStencilDescArrayManager.Create(device_in, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 128);

	//D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	//srvHeapDesc.NumDescriptors = 16;
	//srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	//ShaderResourceDescArray.Create(device_in, srvHeapDesc);

	ShaderResourceDescArrayManager.Create(device_in, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128);

	//D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	//cbvHeapDesc.NumDescriptors = 16;
	//cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	//ConstantBufferDescArray.Create(device_in, cbvHeapDesc);


	XAllocConfig default_cfg;
	default_cfg.d3d12_heap_flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
	default_cfg.d3d12_heap_type = D3D12_HEAP_TYPE_DEFAULT;
	texture_default_heap_alloc.Create(
		physic_device,
		default_cfg,
		100 * (1 << 20),
		(64 * 1024),
		AllocStrategy::PlacedResource);

	XAllocConfig upload_cfg;
	upload_cfg.d3d12_heap_type = D3D12_HEAP_TYPE_UPLOAD;
	upload_cfg.d3d12_resource_states = D3D12_RESOURCE_STATE_GENERIC_READ;
	texture_upload_heap_alloc.Create(
		physic_device,
		upload_cfg,
		100 * (1 << 20),
		(64 * 1024),
		AllocStrategy::ManualSubAllocation
	);
}

void XD3D12AbstractDevice::CreateD3D12Texture2D(XDxRefCount<ID3D12GraphicsCommandList>& cmd_list, uint32 width, uint32 height, DXGI_FORMAT format, uint8* tex_data)
{
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//XDxRefCount<ID3D12Resource> m_texture;


	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Alignment = MIN_PLACED_BUFFER_SIZE;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	//textureDesc.Layout = ;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	const D3D12_RESOURCE_ALLOCATION_INFO Info = physic_device->GetDXDevice()->GetResourceAllocationInfo(0, 1, &textureDesc);
	XD3D12ResourceLocation default_location;
	bool res = texture_default_heap_alloc.Allocate(Info.SizeInBytes, default_location);
	X_Assert(res == true);

	ThrowIfFailed(physic_device->GetDXDevice()->CreatePlacedResource(
			default_location.GetBuddyAllocator()->GetDXHeap(), default_location.GetBuddyAllocData().offset, &textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_texture)));
	
	m_texture->SetName(L"texture test");

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

	XD3D12ResourceLocation upload_location;
	texture_upload_heap_alloc.Allocate(uploadBufferSize, upload_location);


	
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = tex_data;
	textureData.RowPitch = 256 * 4;
	textureData.SlicePitch = textureData.RowPitch * 256;

	CopyDataFromUploadToDefaulHeap
	(cmd_list.Get(), m_texture.Get(),
		texture_upload_heap_alloc.GetDXResource(), upload_location.GetBuddyAllocData().offset, 0, 1, &textureData);
	cmd_list->ResourceBarrier(1, &(CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_texture->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = m_texture->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	uint32 index_of_desc_in_heap;
	uint32 index_of_heap;
	ShaderResourceDescArrayManager.AllocateDesc(index_of_desc_in_heap, index_of_heap);
	//TODO
	//physic_device->GetDXDevice()->CreateShaderResourceView(m_texture.Get(), &srvDesc, ShaderResourceDescArray.GetCPUDescPtrByIndex(0));
	
	physic_device->GetDXDevice()->CreateShaderResourceView(
		m_texture.Get(), &srvDesc,
		ShaderResourceDescArrayManager.compute_cpu_ptr(index_of_desc_in_heap, index_of_heap));
}

XD3D12CommandQueue* XD3D12AbstractDevice::GetCmdQueueByType(D3D12_COMMAND_LIST_TYPE cmd_type)
{
	switch (cmd_type)
	{
	case D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT:
		return direct_cmd_queue;
		break;
	case D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COMPUTE:
		return compute_cmd_queue;
		break;
	default:
		X_Assert(false);
		break;
	}
}