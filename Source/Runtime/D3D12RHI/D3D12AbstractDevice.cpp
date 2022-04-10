#include <string>
#include "D3D12AbstractDevice.h"
#include "Runtime/D3D12RHI/d3dx12.h"
#include "Runtime/Core/Template/XEngineTemplate.h"


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

	RenderTargetDescArrayManager.Create(PhysicalDeviceIn, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 128);
	DepthStencilDescArrayManager.Create(PhysicalDeviceIn, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 128);
	ShaderResourceDescArrayManager.Create(PhysicalDeviceIn, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128);

	XAllocConfig default_cfg;
	default_cfg.d3d12_heap_flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
	default_cfg.d3d12_heap_type = D3D12_HEAP_TYPE_DEFAULT;
	DefaultNonRtDsTextureHeapAlloc.Create(PhysicalDevice, default_cfg, 512 * (1 << 20), (64 * 1024), AllocStrategy::PlacedResource);


	XAllocConfig BufferTypeAlloc_HeapDefault;
	BufferTypeAlloc_HeapDefault.d3d12_heap_flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
	BufferTypeAlloc_HeapDefault.d3d12_heap_type = D3D12_HEAP_TYPE_DEFAULT;
	BufferTypeAlloc_HeapDefault.d3d12_resource_states = D3D12_RESOURCE_STATE_COMMON;
	VIBufferBufferAllocDefault.Create(PhysicalDevice, BufferTypeAlloc_HeapDefault, 512 * (1 << 20), (64 * 1024), AllocStrategy::ManualSubAllocation);


	XAllocConfig upload_cfg;
	upload_cfg.d3d12_heap_type = D3D12_HEAP_TYPE_UPLOAD;
	upload_cfg.d3d12_resource_states = D3D12_RESOURCE_STATE_GENERIC_READ;
	UploadHeapAlloc.Create(PhysicalDevice, upload_cfg, 512 * (1 << 20), (64 * 1024), AllocStrategy::ManualSubAllocation);

	ConstantBufferUploadHeapAlloc.Create(PhysicalDevice, upload_cfg, 128 * (1 << 20), 256, AllocStrategy::ManualSubAllocation);

	uint32 temp_thread_num = 2;
	DirectCtxs.resize(temp_thread_num);
	for (uint32 i = 0; i < temp_thread_num; ++i)
	{
		DirectCtxs[i].Create(this);
	}
}

std::shared_ptr<XD3D12ConstantBuffer> XD3D12AbstractDevice::CreateUniformBuffer(uint32 size)
{
	std::shared_ptr<XD3D12ConstantBuffer> ContantBuffer= std::make_shared<XD3D12ConstantBuffer>();
	XD3D12ResourcePtr_CPUGPU& Location = ContantBuffer.get()->ResourceLocation;
	ConstantBufferUploadHeapAlloc.Allocate(size, 256, Location);
	return ContantBuffer;
}

