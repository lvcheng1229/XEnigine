#pragma once
#include "D3D12CommandQueue.h"
#include "D3D12Context.h"
#include "D3D12DescriptorArray.h"
#include "D3D12Allocation.h"
#include "D3D12Texture.h"

#include <memory>


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

	inline XD3D12PhysicDevice*		GetPhysicalDevice()						{ return PhysicalDevice; }
	inline XD3DDirectContex*		GetDirectContex(uint32 index_thread)	{ return &DirectCtxs[index_thread]; }
	inline XD3DBuddyAllocator*		GetConstantBufferUploadHeapAlloc()		{ return &ConstantBufferUploadHeapAlloc; }
	
	inline XD3D12DescArrayManager*	GetRenderTargetDescArrayManager()		{ return &RenderTargetDescArrayManager; }
	inline XD3D12DescArrayManager*	GetDepthStencilDescArrayManager()		{ return &DepthStencilDescArrayManager; }
	inline XD3D12DescArrayManager*	GetShaderResourceDescArrayManager()		{ return &ShaderResourceDescArrayManager; }

	inline std::unordered_map<std::size_t, std::shared_ptr<XD3D12RootSignature>>& GetRootSigMap() { return RootSignatureMap; }

	XD3D12CommandQueue* GetCmdQueueByType(D3D12_COMMAND_LIST_TYPE cmd_type);

	std::shared_ptr<XD3D12ConstantBuffer>CreateUniformBuffer(uint32 size);

	XD3D12Texture2D* CreateD3D12Texture2D(XD3D12DirectCommandList* x_cmd_list, uint32 width, uint32 height, uint32 SizeZ,
		bool bTextureArray, bool bCubeTexture, EPixelFormat Format, ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data);

	XD3D12Texture3D* CreateD3D12Texture3D(XD3D12DirectCommandList* x_cmd_list, uint32 width, uint32 height, uint32 SizeZ,
		EPixelFormat Format, ETextureCreateFlags flag, uint32 NumMipsIn, uint8* tex_data);

	template<typename BufferType>
	BufferType* CreateRHIVIBuffer(XD3D12DirectCommandList* D3D12CmdList, const D3D12_RESOURCE_DESC& InDesc, uint32 Alignment,
		uint32 Stride, uint32 Size, EBufferUsage InUsage, XRHIResourceCreateData& CreateData);

private:
	uint64 TempResourceIndex;
	std::vector<XD3D12Resource>ResourceManagerTempVec;

	std::unordered_map<std::size_t, std::shared_ptr<XD3D12RootSignature>>RootSignatureMap;

	XD3D12PhysicDevice* PhysicalDevice;

	XD3D12CommandQueue* DirectxCmdQueue;;
	XD3D12CommandQueue* ComputeCmdQueue;

	XD3DBuddyAllocator VIBufferBufferAllocDefault;//manual+default
	//XD3DBuddyAllocator VIBufferBufferAllocUpload;//manual+upload

	XD3DBuddyAllocator DefaultNonRtDsTextureHeapAlloc;	//placed+default
	XD3DBuddyAllocator UploadHeapAlloc;					//manual+upload
	XD3DBuddyAllocator ConstantBufferUploadHeapAlloc;	//manual+upload

	std::vector<XD3DDirectContex>DirectCtxs;

	XD3D12DescArrayManager RenderTargetDescArrayManager;
	XD3D12DescArrayManager DepthStencilDescArrayManager;
	XD3D12DescArrayManager ShaderResourceDescArrayManager;
	XD3D12DescArrayManager ConstantBufferDescArrayManager;
};
