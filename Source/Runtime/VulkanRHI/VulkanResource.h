#pragma once
#include <map>
#include <vulkan\vulkan_core.h>
#include <Runtime\HAL\Mch.h>
#include <Runtime\HAL\PlatformTypes.h>
#include "Runtime\RHI\RHIResource.h"
#include "Runtime\RenderCore\ShaderCore.h"
#include "VulkanPipeline.h"
#include "VulkanMemory.h"

class XVulkanDevice;
struct XVulkanTextureView
{
	XVulkanTextureView()
		: View(VK_NULL_HANDLE)
		, Image(VK_NULL_HANDLE)
		, ViewId(0)
	{
	}

	void Create(XVulkanDevice* Device, VkImage InImage, VkImageViewType ViewType, VkImageAspectFlags Aspect,VkFormat Format);
	VkImageView View;
	VkImage Image;
	uint32 ViewId;
private:
	void CreateImpl(XVulkanDevice* Device, VkImage InImage, VkImageViewType ViewType, VkImageAspectFlags Aspect, VkFormat Format);
};

class XVulkanVertexLayout :public XRHIVertexLayout
{
public:
	XRHIVertexLayoutArray VertexElements;
	explicit XVulkanVertexLayout(const XRHIVertexLayoutArray& InVertexElements) :
		VertexElements(InVertexElements) {}
};

class XVulkanSurface
{
public:
	XVulkanSurface(XVulkanDevice* Device, EPixelFormat Format, uint32 Width , uint32 Height ,VkImageViewType	InViewType);
	
	// Constructor for externally owned Image
	XVulkanSurface(XVulkanDevice* InDevice, EPixelFormat InFormat, uint32 Width, uint32 Height, VkImageViewType	ViewType ,VkImage InImage);
	XVulkanSurface(XVulkanEvictable* Owner, XVulkanDevice* InDevice, EPixelFormat InFormat, uint32 Width, uint32 Height, VkImageViewType	InViewType, ETextureCreateFlags Flag, uint32 NumMipsIn, uint8* TexData, uint32 DataSize);

	inline VkImageViewType GetViewType() const { return ViewType; }

	static void InternalLockWrite(class XVulkanCommandListContext* Context, XVulkanSurface* Surface, const VkBufferImageCopy& Region, XStagingBuffer* StagingBuffer);

	EPixelFormat PixelFormat;
	XVulkanDevice* Device;

	VkFormat ViewFormat;
	VkImage Image;

	uint32 Width;
	uint32 Height;

	VkImageAspectFlags FullAspectMask;
	VkMemoryRequirements MemoryRequirements;
private:
	void SetInitialImageState(XVulkanCommandListContext& Context, VkImageLayout InitialLayout, bool bClear);

	XVulkanAllocation Allocation;
	VkImageViewType	ViewType;
};

class XVulkanTextureBase :public XVulkanEvictable
{
public:
	XVulkanTextureBase(XVulkanDevice* Device, EPixelFormat Format, uint32 Width, uint32 Height, VkImageViewType	InViewType);
	XVulkanTextureBase(XVulkanDevice* Device, EPixelFormat Format, uint32 Width, uint32 Height, VkImageViewType	InViewType, VkImage InImage);

	XVulkanTextureBase(XVulkanDevice* Device, EPixelFormat Format, uint32 Width, uint32 Height, VkImageViewType	InViewType, ETextureCreateFlags Flag, uint32 NumMipsIn, uint8* TexData, uint32 DataSize);

	XVulkanSurface Surface;
	XVulkanTextureView DefaultView;
};

class XVulkanTexture2D : public XRHITexture2D, public XVulkanTextureBase
{
public:
	XVulkanTexture2D(XVulkanDevice* Device, EPixelFormat Forma, uint32 Width, uint32 Height, VkImageViewType	InViewType);
	XVulkanTexture2D(XVulkanDevice* Device, EPixelFormat Forma, uint32 Width, uint32 Height, VkImageViewType	InViewType, VkImage InImage);
	
	XVulkanTexture2D(XVulkanDevice* Device, EPixelFormat Format, uint32 Width, uint32 Height, ETextureCreateFlags Flag, uint32 NumMipsIn, uint8* TexData, uint32 DataSize);

	virtual void* GetTextureBaseRHI()
	{
		return static_cast<XVulkanTextureBase*>(this);
	}
};

inline XVulkanTextureBase* GetVulkanTextureFromRHITexture(XRHITexture* Texture)
{
	if (!Texture) { return NULL; }
	XVulkanTextureBase* Result = ((XVulkanTextureBase*)Texture->GetTextureBaseRHI()); XASSERT(Result);
	return Result;
}

class XVulkanShader
{
public:
	XVulkanShader(XVulkanDevice* InDevice, EShaderType InShaderType);
	
	class XSpirvContainer
	{
	public:
		friend class XVulkanShader;
		std::vector<uint8>	SpirvCode;
	} SpirvContainer;

	XShaderResourceCount ResourceCount;
	std::string EntryName;


	std::string GetEntryPoint()
	{
		return EntryName;
	}

	VkShaderModule GetOrCreateHandle(const XGfxPipelineDesc& Desc, uint32 LayoutHash)
	{
		auto iter = ShaderModules.find(LayoutHash);;
		if (iter != ShaderModules.end())
		{
			return iter->second;
		}

		return CreateHandle(Desc, LayoutHash);
	}

	XVulkanDevice* Device;
	std::map<uint32, VkShaderModule> ShaderModules;
protected:
	VkShaderModule CreateHandle(const XGfxPipelineDesc& Desc, uint32 LayoutHash);
};

class XVulkanVertexShader : public XRHIVertexShader , public XVulkanShader
{
public:
	XVulkanVertexShader(XVulkanDevice* InDevice)
		:XVulkanShader(InDevice, EShaderType::SV_Vertex) {}
	enum
	{
		ShaderTypeStatic = EShaderType::SV_Vertex
	};
};

class XVulkanPixelShader : public XRHIPixelShader, public XVulkanShader
{
public:
	XVulkanPixelShader(XVulkanDevice* InDevice)
		:XVulkanShader(InDevice, EShaderType::SV_Pixel) {}

	enum
	{
		ShaderTypeStatic = EShaderType::SV_Pixel
	};
};

//TODO XVulkanResourceMultiBuffer
class XVulkanResourceMultiBuffer : public XRHIBuffer ,public XVulkanEvictable
{
public:
	XVulkanResourceMultiBuffer(XVulkanDevice* Device,uint32 Stride, uint32 Size, EBufferUsage Usage, XRHIResourceCreateData ResourceData);
	
	void* Lock(EResourceLockMode LockMode, uint32 LockSize, uint32 Offset);
	
	static VkBufferUsageFlags UEToVKBufferUsageFlags(EBufferUsage InUEUsage);
	
	
	XVulkanAllocation Buffer;
	VkBufferUsageFlags BufferUsageFlags;
	static void InternalUnlock(class XVulkanCommandListContext* Context, class XPendingBufferLock* PendingLock, XVulkanResourceMultiBuffer* MultiBuffer);
private:
	XVulkanDevice* Device;

};

class XVulkanShaderFactory
{
public:
	~XVulkanShaderFactory();

	template <typename ShaderType>
	ShaderType* CreateShader(XArrayView<uint8> Code, XVulkanDevice* Device);

	template <typename ShaderType>
	ShaderType* LookupShader(uint64 ShaderKey) const;

private:
	std::map<uint64,XVulkanShader*>MapToVkShader[(uint32)EShaderType::SV_ShaderCount];
	
};
