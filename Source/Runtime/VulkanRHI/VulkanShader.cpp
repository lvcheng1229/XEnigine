#include "VulkanPlatformRHI.h"
#include "VulkanResource.h"
#include "VulkanDevice.h"
#include "Runtime\Core\Template\XEngineTemplate.h"

template XVulkanVertexShader* XVulkanShaderFactory::CreateShader<XVulkanVertexShader>(XArrayView<uint8> Code, XVulkanDevice* Device);
template XVulkanPixelShader* XVulkanShaderFactory::CreateShader<XVulkanPixelShader>(XArrayView<uint8> Code, XVulkanDevice* Device);

template XVulkanVertexShader* XVulkanShaderFactory::LookupShader<XVulkanVertexShader>(uint64 ShaderKey) const;
template XVulkanPixelShader* XVulkanShaderFactory::LookupShader<XVulkanPixelShader>(uint64 ShaderKey) const;


void XVulkanLayout::Compile(XVulkanDescriptorSetLayoutMap& InDSetLayoutMap)
{
	DescriptorSetLayout.Compile(InDSetLayoutMap);

	const VkDescriptorSetLayout LayoutHandle = DescriptorSetLayout.LayoutHandle;
	VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {};
	PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	PipelineLayoutCreateInfo.setLayoutCount = 1;
	PipelineLayoutCreateInfo.pSetLayouts = &LayoutHandle;

	VULKAN_VARIFY(vkCreatePipelineLayout(Device->GetVkDevice(), &PipelineLayoutCreateInfo, nullptr, &PipelineLayout));
}


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

	XShaderCodeReader ShaderCodeReader(Code);
	const XShaderResourceCount* ResourceCount = (const XShaderResourceCount*)(ShaderCodeReader.FindOptionalData(
		XShaderResourceCount::Key, sizeof(XShaderResourceCount)));
	if (ResourceCount == nullptr) { return nullptr; }

	int32 EntryNameSize = *(int32*)((const char*)ResourceCount + sizeof(XShaderResourceCount));
	const char* EntryName = ((const char*)ResourceCount + sizeof(XShaderResourceCount) + sizeof(int32));
	ShaderPtr->EntryName = std::string(EntryName, EntryNameSize);

	const uint8* CodaData = (const uint8*)Code.data();
	ShaderPtr->SpirvContainer.SpirvCode.insert(ShaderPtr->SpirvContainer.SpirvCode.end(), &CodaData[0], &CodaData[Code.size() - ShaderCodeReader.GetOptionalDataSize()]);
	ShaderPtr->ResourceCount = *ResourceCount;

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

uint32 XVulkanDescriptorSetWriter::SetupDescriptorWrites(
	const std::vector<VkDescriptorType>& Types, VkWriteDescriptorSet* InWriteDescriptors,
	VkDescriptorImageInfo* InImageInfo, VkDescriptorBufferInfo* InBufferInfo,
	const XVulkanSamplerState* DefaultSampler, const XVulkanTextureView* DefaultImageView)
{
	NumWrites = Types.size();
	WriteDescriptors = InWriteDescriptors;
	for (int32 Index = 0; Index < Types.size(); ++Index)
	{
		InWriteDescriptors->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		InWriteDescriptors->dstBinding = Index;
		InWriteDescriptors->descriptorCount = 1;
		InWriteDescriptors->descriptorType = Types[Index];

		switch (Types[Index])
		{
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			InWriteDescriptors->pBufferInfo = InBufferInfo++;
			break;
		case VK_DESCRIPTOR_TYPE_SAMPLER:
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
			InImageInfo->sampler = DefaultSampler->Sampler;
			InImageInfo->imageView = DefaultImageView->View;
			InImageInfo->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			InWriteDescriptors->pImageInfo = InImageInfo++;
			break;
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
			break;
		default:
			XASSERT(false);
			break;
		}
		++InWriteDescriptors;
	}


	return uint32();
}
