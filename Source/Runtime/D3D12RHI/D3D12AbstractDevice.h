#pragma once
#include "D3D12CommandQueue.h"
#include "D3D12Context.h"
#include "D3D12DescriptorArray.h"
#include "D3D12Allocation.h"
#include "D3D12Texture.h"

#include <memory>
class ResourceManager_Temp
{
	
};

class XD3D12AbstractDevice
{
	
public:
	XD3D12AbstractDevice():
		TempResourceIndex(0),
		PhysicalDevice(nullptr),
		DirectxCmdQueue(nullptr),
		ComputeCmdQueue(nullptr)
		{};

	~XD3D12AbstractDevice();
	void Create(XD3D12PhysicDevice* PhysicalDeviceIn);

	std::shared_ptr<XD3D12ConstantBuffer>CreateUniformBuffer(uint32 size);

	XD3D12Texture2D* CreateD3D12Texture2D(XD3D12DirectCommandList* x_cmd_list, uint32 width, uint32 height, uint32 SizeZ,
		bool bTextureArray, bool bCubeTexture, EPixelFormat Format, ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data);

	XD3D12Texture3D* CreateD3D12Texture3D(XD3D12DirectCommandList* x_cmd_list, uint32 width, uint32 height, uint32 SizeZ,
		EPixelFormat Format, ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data);

	XD3D12CommandQueue* GetCmdQueueByType(D3D12_COMMAND_LIST_TYPE cmd_type);


	inline XD3D12PhysicDevice* GetPhysicalDevice() { return PhysicalDevice; }
	inline XD3DDirectContex* GetDirectContex(uint32 index_thread) { return &DirectCtxs[index_thread]; }
	inline XD3D12DescArrayManager* GetRenderTargetDescArrayManager() { return &RenderTargetDescArrayManager; }
	inline XD3D12DescArrayManager* GetDepthStencilDescArrayManager() { return &DepthStencilDescArrayManager; }
	inline XD3D12DescArrayManager* GetShaderResourceDescArrayManager() { return &ShaderResourceDescArrayManager; }
private:
	uint64 TempResourceIndex;
	std::vector<XD3D12Resource>ResourceManagerTempVec;

	XD3D12PhysicDevice* PhysicalDevice;

	XD3D12CommandQueue* DirectxCmdQueue;;
	XD3D12CommandQueue* ComputeCmdQueue;

	XD3DBuddyAllocator DefaultNonRtDsTextureHeapAlloc;
	XD3DBuddyAllocator UploadHeapAlloc;
	XD3DBuddyAllocator ConstantBufferUploadHeapAlloc;

	std::vector<XD3DDirectContex>DirectCtxs;

	XD3D12DescArrayManager RenderTargetDescArrayManager;
	XD3D12DescArrayManager DepthStencilDescArrayManager;
	XD3D12DescArrayManager ShaderResourceDescArrayManager;
	XD3D12DescArrayManager ConstantBufferDescArrayManager;
};