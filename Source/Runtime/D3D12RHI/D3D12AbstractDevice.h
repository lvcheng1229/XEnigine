#pragma once
#include "D3D12CommandQueue.h"
#include "D3D12Context.h"
#include "D3D12DescriptorArray.h"
#include "D3D12Allocation.h"
class XD3D12AbstractDevice
{
public:
	XD3D12AbstractDevice() :
		physic_device(nullptr), 
		direct_cmd_queue(nullptr),
		compute_cmd_queue(nullptr), 
		direct_ctx_default(true) {};

	~XD3D12AbstractDevice();
	void Create(XD3D12PhysicDevice* device_in);

	void CreateD3D12Texture2D(
		XDxRefCount<ID3D12GraphicsCommandList>& cmd_list,
		uint32 width, uint32 height, DXGI_FORMAT format, uint8* tex_data);

	XD3D12CommandQueue* GetCmdQueueByType(D3D12_COMMAND_LIST_TYPE cmd_type);

	//void CreateBuffer();

	inline XD3D12PhysicDevice* GetPhysicalDevice() { return physic_device; }
	inline XD3DDirectContex* GetDirectContex() { return &direct_ctx_default; }
	
	inline XD3D12DescArrayManager* GetRenderTargetDescArrayManager() { return &RenderTargetDescArrayManager; }
	inline XD3D12DescArrayManager* GetDepthStencilDescArrayManager() { return &DepthStencilDescArrayManager; }
	inline XD3D12DescArrayManager* GetShaderResourceDescArrayManager() { return &ShaderResourceDescArrayManager; }

	//inline XD3D12DescriptorArray* GetRenderTargetDescArray() { return &RenderTargetDescArray; };
	//inline XD3D12DescriptorArray* GetDepthStencilDescArray() { return &DepthStencilDescArray; };
	//inline XD3D12DescriptorArray* GetShaderRresourceDescArray() { return &ShaderResourceDescArray; };
private:
	XDxRefCount<ID3D12Resource> m_texture;//for test

	XD3D12PhysicDevice* physic_device;
	XD3D12CommandQueue* direct_cmd_queue;
	XD3D12CommandQueue* compute_cmd_queue;
	XD3DDirectContex direct_ctx_default;

	XD3DBuddyAllocator texture_default_heap_alloc;
	XD3DBuddyAllocator texture_upload_heap_alloc;

	XD3DBuddyAllocator constant_upload_heap_alloc;

	//XD3D12DescriptorArray RenderTargetDescArray;
	//XD3D12DescriptorArray DepthStencilDescArray;
	//XD3D12DescriptorArray ShaderResourceDescArray;
	//XD3D12DescriptorArray ConstantBufferDescArray;
	
	//XD3D12DescriptorArray ShaderResourceDescArray;
	//XD3D12DescriptorArray SamplerDescArray;

	XD3D12DescArrayManager RenderTargetDescArrayManager;
	XD3D12DescArrayManager DepthStencilDescArrayManager;
	XD3D12DescArrayManager ShaderResourceDescArrayManager;
	XD3D12DescArrayManager ConstantBufferDescArrayManager;
};