#include "VulkanDevice.h"
#include "VulkanPipeline.h"

const std::vector<const ACHAR*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

XVulkanDevice::XVulkanDevice(XVulkanPlatformRHI* InRHI, VkPhysicalDevice InGpu)
	: Device(VK_NULL_HANDLE)
	, RHI(InRHI)
	, Gpu(InGpu)
    , GfxQueue(nullptr)
    , PresentQueue(nullptr)
    , MemoryManager(this, &DeviceMemoryManager)
    , FenceManager(this)
{
    PipelineStateCache = new XVulkanPipelineStateCacheManager(this);
    DescriptorPoolsManager = new XVulkanDescriptorPoolsManager(this);
}

XVulkanDevice::~XVulkanDevice()
{
    delete PipelineStateCache;
    delete GfxContext;
    delete GfxQueue;
    delete DescriptorPoolsManager;
    vkDestroyDevice(Device, nullptr);
}

void XVulkanDevice::InitGPU()
{
    GfxContext = new XVulkanCommandListContext(RHI, this, GfxQueue);
   
    // Setup default resource
    {
        XSamplerStateInitializerRHI Default(SF_Point);
        DefaultSampler = ResourceCast(RHICreateSamplerState(Default).GetReference());

        FRHIResourceCreateInfo CreateInfo(TEXT("FVulkanDevice_DefaultImage"));
        DefaultImage = new XVulkanSurface(*this, 0, VK_IMAGE_VIEW_TYPE_2D, PF_B8G8R8A8, 1, 1, 1, 1, 1, 1, TexCreate_RenderTargetable | TexCreate_ShaderResource, ERHIAccess::SRVMask, CreateInfo);
        DefaultTextureView.Create(this, DefaultImage->Image, VK_IMAGE_VIEW_TYPE_2D, DefaultImage->GetFullAspectMask(), PF_B8G8R8A8, VK_FORMAT_B8G8R8A8_UNORM, 0, 1, 0, 1);
    }

}

void XVulkanDevice::CreateDevice()
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(Gpu, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(Gpu, &queueFamilyCount, queueFamilies.data());

    int32 GfxQueueFamilyIndex = -1;
    int32 ComputeQueueFamilyIndex = -1;
    int32 TransferQueueFamilyIndex = -1;
    for (int32 Index = 0; Index < queueFamilies.size(); Index++)
    {
        if (queueFamilies[Index].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            GfxQueueFamilyIndex = Index;
            break;
        }
    }

    GfxQueueIndex = GfxQueueFamilyIndex;
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = GfxQueueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledLayerCount = 0;

    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VULKAN_VARIFY(vkCreateDevice(Gpu, &createInfo, nullptr, &Device));
    GfxQueue = new XVulkanQueue(this, GfxQueueFamilyIndex);
    
    DeviceMemoryManager.Init(this);
    StagingManager.Init(this);
    MemoryManager.Init();
}


