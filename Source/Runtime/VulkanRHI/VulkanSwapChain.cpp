#include "Runtime\Core\PixelFormat.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "GLFW\glfw3.h"
#include <algorithm>

void XVulkanDevice::SetPresentQueue(VkSurfaceKHR Surface)
{
    if (!PresentQueue)
    {
        VkBool32 bPresentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(Gpu, GfxQueue->GetFamilyIndex(), Surface, &bPresentSupport);
        XASSERT(bPresentSupport != 0);
        PresentQueue = GfxQueue;
    }
}



XVulkanSwapChain::XVulkanSwapChain(EPixelFormat& InOutPixelFormat, XVulkanDevice* VulkanDevice, void* InWindowHandle, VkInstance InInstance, std::vector<VkImage>& OutImages , VkExtent2D& SwapChainExtent)
    : Device(VulkanDevice)
    , WindowHandle(InWindowHandle)
    , Instance(InInstance)
{
    glfwCreateWindowSurface(Instance, (GLFWwindow*)WindowHandle, nullptr, &Surface);//temp    
    Device->SetPresentQueue(Surface);

    //get capabilities
    VkSurfaceCapabilitiesKHR Capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*Device->GetVkPhysicalDevice(), Surface, &Capabilities);

    //get formats
    uint32 FormatCount;
    std::vector<VkSurfaceFormatKHR> Formats;

    vkGetPhysicalDeviceSurfaceFormatsKHR(*Device->GetVkPhysicalDevice(), Surface, &FormatCount, nullptr);

    if (FormatCount != 0) 
    {
        Formats.resize(FormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(*Device->GetVkPhysicalDevice(), Surface, &FormatCount, Formats.data());
    }

    //get present mode
    uint32 PresentModeCount;
    std::vector<VkPresentModeKHR> PresentModes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(*Device->GetVkPhysicalDevice(), Surface, &PresentModeCount, nullptr);

    if (PresentModeCount != 0) {
        PresentModes.resize(PresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(*Device->GetVkPhysicalDevice(), Surface, &PresentModeCount, PresentModes.data());
    }

    uint32 DesiredNumBuffers = Capabilities.maxImageCount > 0 ? std::clamp(2u, Capabilities.minImageCount, Capabilities.maxImageCount) : 2u;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = Surface;

    createInfo.minImageCount = DesiredNumBuffers;
    createInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;// VkFormat(GPixelFormats[(int32)InOutPixelFormat].PlatformFormat);
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = Capabilities.currentExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = Capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VULKAN_VARIFY(vkCreateSwapchainKHR(Device->GetVkDevice(), &createInfo, nullptr, &swapChain));

    vkGetSwapchainImagesKHR(Device->GetVkDevice(), swapChain, &DesiredNumBuffers, nullptr);
    OutImages.resize(DesiredNumBuffers);
    vkGetSwapchainImagesKHR(Device->GetVkDevice(), swapChain, &DesiredNumBuffers, OutImages.data());

    SwapChainExtent = Capabilities.currentExtent;
}

XVulkanSwapChain::~XVulkanSwapChain()
{
    vkDestroySwapchainKHR(Device->GetVkDevice(), swapChain, nullptr);
    vkDestroySurfaceKHR(Instance, Surface, nullptr);
}
