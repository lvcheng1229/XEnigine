#pragma once
#include <map>
#include <vulkan\vulkan_core.h>
#include <Runtime\HAL\Mch.h>
#include <Runtime\HAL\PlatformTypes.h>
#include "Runtime\RHI\RHIResource.h"

class XVulkanDevice;
struct XVulkanTextureView
{
	XVulkanTextureView()
		: View(VK_NULL_HANDLE)
		, Image(VK_NULL_HANDLE)
		, ViewId(0)
	{
	}

	void Create(XVulkanDevice* Device, VkImage InImage, VkImageViewType ViewType, VkFormat Format);
	VkImageView View;
	VkImage Image;
	uint32 ViewId;
private:
	void CreateImpl(XVulkanDevice* Device, VkImage InImage, VkImageViewType ViewType, VkFormat Format);
};

class XVulkanSurface
{
public:
	XVulkanSurface(XVulkanDevice* Device, EPixelFormat Format, uint32 Width , uint32 Height ,VkImageViewType	InViewType);
	
	// Constructor for externally owned Image
	XVulkanSurface(XVulkanDevice* InDevice, EPixelFormat InFormat , uint32 Width, uint32 Height, VkImageViewType	ViewType ,VkImage InImage);

	inline VkImageViewType GetViewType() const { return ViewType; }

	EPixelFormat PixelFormat;
	XVulkanDevice* Device;

	VkFormat ViewFormat;
	VkImage Image;

	uint32 Width;
	uint32 Height;

private:
	VkImageViewType	ViewType;
};

class XVulkanTextureBase
{
public:
	XVulkanTextureBase(XVulkanDevice* Device, EPixelFormat Format, uint32 Width, uint32 Height, VkImageViewType	InViewType);
	XVulkanTextureBase(XVulkanDevice* Device, EPixelFormat Format, uint32 Width, uint32 Height, VkImageViewType	InViewType, VkImage InImage);



	XVulkanSurface Surface;
};

class XVulkanTexture2D : public XRHITexture2D, public XVulkanTextureBase
{
public:
	XVulkanTexture2D(XVulkanDevice* Device, EPixelFormat Forma, uint32 Width, uint32 Height, VkImageViewType	InViewType);
	XVulkanTexture2D(XVulkanDevice* Device, EPixelFormat Forma, uint32 Width, uint32 Height, VkImageViewType	InViewType, VkImage InImage);

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
	class XSpirvContainer
	{
	public:
		friend class XVulkanShader;
		std::vector<uint8>	SpirvCode;
	} SpirvContainer;
};

class XVulkanVertexShader : public XRHIVertexShader , public XVulkanShader
{
public:
	enum
	{
		ShaderTypeStatic = EShaderType::SV_Vertex
	};
};

class XVulkanPixelShader : public XRHIVertexShader, public XVulkanShader
{
public:
	enum
	{
		ShaderTypeStatic = EShaderType::SV_Pixel
	};
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
