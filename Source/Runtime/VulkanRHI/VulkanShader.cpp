#include "VulkanPlatformRHI.h"
#include "VulkanResource.h"
#include "VulkanDevice.h"
#include "Runtime\Core\Template\XEngineTemplate.h"

template XVulkanVertexShader* XVulkanShaderFactory::CreateShader<XVulkanVertexShader>(XArrayView<uint8> Code, XVulkanDevice* Device);
template XVulkanPixelShader* XVulkanShaderFactory::CreateShader<XVulkanPixelShader>(XArrayView<uint8> Code, XVulkanDevice* Device);

template XVulkanVertexShader* XVulkanShaderFactory::LookupShader<XVulkanVertexShader>(uint64 ShaderKey) const;
template XVulkanPixelShader* XVulkanShaderFactory::LookupShader<XVulkanPixelShader>(uint64 ShaderKey) const;

XVulkanShaderFactory::~XVulkanShaderFactory()
{
	for (int32 Index = 0; Index < (uint32)EShaderType::SV_ShaderCount; Index++)
	{
		for (auto Iter : MapToVkShader[Index])
		{
			delete Iter.second;
		}
	}
}

template<typename ShaderType>
inline ShaderType* XVulkanShaderFactory::CreateShader(XArrayView<uint8> Code, XVulkanDevice* Device)
{
	std::size_t ShaderHash = std::hash<std::string>{}(std::string((char*)Code.data(), Code.size()));
	ShaderType* ShaderPtr = LookupShader<ShaderType>(ShaderHash);
	if (ShaderPtr)
	{
		return ShaderPtr;
	}
	
	ShaderPtr = new ShaderType(Device);
	const uint8* CodaData = (const uint8*)Code.data();
	ShaderPtr->SpirvContainer.SpirvCode.insert(ShaderPtr->SpirvContainer.SpirvCode.end(), &CodaData[0], &CodaData[Code.size()]);
	
	return ShaderPtr;
}

template<typename ShaderType>
ShaderType* XVulkanShaderFactory::LookupShader(uint64 ShaderKey) const
{
	if (ShaderKey)
	{
		auto iter = MapToVkShader[ShaderType::ShaderTypeStatic].find(ShaderKey);
		if (iter != MapToVkShader[ShaderType::ShaderTypeStatic].end())
		{
			return static_cast<ShaderType*>(iter->second);
		}
	}
	return nullptr;
}

std::shared_ptr<XRHIVertexShader> XVulkanPlatformRHI::RHICreateVertexShader(XArrayView<uint8> Code)
{
	return std::shared_ptr<XVulkanVertexShader>(Device->GetVkShaderFactory()->CreateShader<XVulkanVertexShader>(Code, Device));
}

std::shared_ptr<XRHIPixelShader> XVulkanPlatformRHI::RHICreatePixelShader(XArrayView<uint8> Code)
{
	return std::shared_ptr<XVulkanPixelShader>(Device->GetVkShaderFactory()->CreateShader<XVulkanPixelShader>(Code, Device));
}

