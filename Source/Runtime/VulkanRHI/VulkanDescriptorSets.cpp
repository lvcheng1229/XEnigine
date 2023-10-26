#include "VulkanDescriptorSets.h"
#include "VulkanResource.h"
#include "VulkanCommon.h"
#include "VulkanLoader.h"
#include "VulkanDevice.h"

static inline uint8 GetIndexForDescritorType(VkDescriptorType DescriptorType)
{
    switch (DescriptorType)
    {
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:return VulkanBindless::BindlesssStorageBufferSet;
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:return VulkanBindless::BindlessAccelerationStructureSet;
    }
    XASSERT(false);

    return VulkanBindless::None;
}

void XVulkanDescriptorSetsLayoutInfo::FinalizeBindings_Gfx(XVulkanShader* VertexShader, XVulkanShader* PixelShader)
{
    if (VertexShader->ResourceCount.NumCBV)
    {
        for (int32 Index = 0; Index < VertexShader->ResourceCount.NumCBV; Index++)
        {
            RemappingInfo.SetInfo.Types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            RemappingInfo.SetInfo.NumBufferInfos++;
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = RemappingInfo.SetInfo.Types.size() - 1;
            uboLayoutBinding.descriptorCount = 1;//?
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            SetLayout.LayoutBindings.push_back(uboLayoutBinding);

            LayoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER]++;
        }
    }

    if (PixelShader->ResourceCount.NumCBV)
    {
        for (int32 Index = 0; Index < PixelShader->ResourceCount.NumCBV; Index++)
        {
            RemappingInfo.SetInfo.Types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            RemappingInfo.SetInfo.NumBufferInfos++;
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = RemappingInfo.SetInfo.Types.size() - 1;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            SetLayout.LayoutBindings.push_back(uboLayoutBinding);

            LayoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER]++;
        }
    }

    //COMBINED IMAGE AND SAMPLER!!

    if (PixelShader->ResourceCount.NumSRV)
    {
        for (int32 Index = 0; Index < PixelShader->ResourceCount.NumSRV; Index++)
        {
            RemappingInfo.SetInfo.Types.push_back(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
            RemappingInfo.SetInfo.NumImageInfos++;
            VkDescriptorSetLayoutBinding textureLayoutBinding{};
            textureLayoutBinding.binding = RemappingInfo.SetInfo.Types.size() - 1;
            textureLayoutBinding.descriptorCount = 1;
            textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            textureLayoutBinding.pImmutableSamplers = nullptr;
            textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            SetLayout.LayoutBindings.push_back(textureLayoutBinding);

            LayoutTypes[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE]++;
        }
    }

    if (PixelShader->ResourceCount.NumSampler)
    {
        for (int32 Index = 0; Index < PixelShader->ResourceCount.NumSampler; Index++)
        {
            RemappingInfo.SetInfo.Types.push_back(VK_DESCRIPTOR_TYPE_SAMPLER);
            RemappingInfo.SetInfo.NumImageInfos++;
            VkDescriptorSetLayoutBinding samplerBinding{};
            samplerBinding.binding = RemappingInfo.SetInfo.Types.size() - 1;
            samplerBinding.descriptorCount = 1;
            samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            samplerBinding.pImmutableSamplers = nullptr;
            samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            SetLayout.LayoutBindings.push_back(samplerBinding);

            LayoutTypes[VK_DESCRIPTOR_TYPE_SAMPLER]++;
        }
    }

    SetLayout.GenerateHash();
    Hash = SetLayout.Hash;
}

XVulkanDescriptorPoolSetContainer& XVulkanDescriptorPoolsManager::AcquirePoolSetContainer()
{
    for (int32 Index = 0; Index < PoolSets.size(); ++Index)
    {
        auto* PoolSet = PoolSets[Index];
        if (PoolSet->IsUnused())
        {
            PoolSet->SetUsed(true);
            return *PoolSet;
        }
    }

    XVulkanDescriptorPoolSetContainer* PoolSet = new XVulkanDescriptorPoolSetContainer(Device);
    PoolSets.push_back(PoolSet);
    return *PoolSet;
}

void XVulkanGfxPipelineDescriptorInfo::Initialize(const XDescriptorSetRemappingInfo& InRemappingInfo)
{
    DescriptorSetRemappingInfo = InRemappingInfo;
}

bool XVulkanGfxPipelineDescriptorInfo::GetDescriptorSetAndBindingIndex(const EShaderType Stage, int32 ParameterIndex, uint8& OutDescriptorSet, uint32& OutBindingIndex) const
{
    return false;
}

XDescriptorSetRemappingInfo::~XDescriptorSetRemappingInfo()
{
    int32 ForDebug;
}

void XVulkanBindlessDescriptorManager::UpdateBuffer(XRHIDescriptorHandle DescriptorHandle, VkBuffer Buffer, VkDeviceSize BufferOffset, VkDeviceSize BufferSize)
{
    VkBufferDeviceAddressInfo BufferInfo = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
    BufferInfo.buffer = Buffer;
    const VkDeviceAddress BufferAddress = VulkanExtension::vkGetBufferDeviceAddressKHR(Device->GetVkDevice(), &BufferInfo);
    UpdateBuffer(DescriptorHandle, BufferAddress + BufferOffset, BufferSize);
}

void XVulkanBindlessDescriptorManager::UpdateBuffer(XRHIDescriptorHandle DescriptorHandle, VkDeviceAddress BufferAddress, VkDeviceSize BufferSize)
{
    VkDescriptorAddressInfoEXT AddressInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT };
    AddressInfo.address = BufferAddress;
    AddressInfo.range = BufferSize;

    VkDescriptorDataEXT DescriptorData;
    DescriptorData.pStorageBuffer = &AddressInfo;
    UpdateDescriptor(DescriptorHandle, DescriptorData);
}

void XVulkanBindlessDescriptorManager::UpdateDescriptor(XRHIDescriptorHandle DescriptorHandle, VkDescriptorDataEXT DescriptorData)
{
    const uint8 SetIndex = DescriptorHandle.Type;
    BindlessSetState& State = BindlessSetStates[SetIndex];
    const uint32 ByteOffset = DescriptorHandle.Index * State.DescriptorSize;

    VkDescriptorGetInfoEXT Info = { VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
    Info.type = State.DescriptorType;
    Info.data = DescriptorData;

    VulkanExtension::vkGetDescriptorEXT(Device->GetVkDevice(), &Info, State.DescriptorSize, &State.DebugDescriptors[ByteOffset]);
    memcpy(State.MappedPointer, &State.DebugDescriptors[ByteOffset], State.DescriptorSize);
}

void XVulkanBindlessDescriptorManager::Unregister(XRHIDescriptorHandle DescriptorHandle)
{
    if (DescriptorHandle.IsValid())
    {
        const uint8 SetIndex = DescriptorHandle.Type;
        BindlessSetState& State = BindlessSetStates[SetIndex];
        const uint32 PreviousHead = State.FreeListHead;
        State.FreeListHead = DescriptorHandle.Index;
        const uint32 ByteOffset = DescriptorHandle.Index * State.DescriptorSize;
        uint32* NewSlot = (uint32*)(&State.DebugDescriptors[ByteOffset]);
        memset(NewSlot, 0, State.DescriptorSize);
        *NewSlot = PreviousHead;
    }
}

uint32 XVulkanBindlessDescriptorManager::GetFreeResourceStateIndex(BindlessSetState& State)
{
    if (State.FreeListHead != -1 && State.CurDescriptorCount >= State.MaxDescriptorCount)
    {
        const uint32 FreeIndex = State.FreeListHead;
        const uint32 ByteOffset = State.FreeListHead * State.DescriptorSize;
        uint32* NextSlot = (uint32*)(&State.DebugDescriptors[ByteOffset]);
        State.FreeListHead = *NextSlot;
        return FreeIndex;
    }

    const uint32 ResourceIndex = State.CurDescriptorCount++;
    return ResourceIndex;
}

XRHIDescriptorHandle XVulkanBindlessDescriptorManager::ReserveDescriptor(VkDescriptorType DescriptorType)
{
    const uint8 SetIndex = GetIndexForDescritorType(DescriptorType);
    BindlessSetState& State = BindlessSetStates[SetIndex];
    const uint32 ResourceIndex = GetFreeResourceStateIndex(State);
    return XRHIDescriptorHandle(SetIndex, ResourceIndex);
}
