
#include "VulkanPlatformRHI.h"


XVulkanPlatformRHI::XVulkanPlatformRHI()
{
}

XVulkanPlatformRHI::~XVulkanPlatformRHI()
{
    vkDestroyInstance(Instance, nullptr);
}

void XVulkanPlatformRHI::Init()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const ACHAR*> Layers;
    std::vector<const ACHAR*> Extensions;
    GetInstanceLayersAndExtensions(Layers, Extensions);
    createInfo.enabledExtensionCount = Extensions.size();
    createInfo.ppEnabledExtensionNames = Extensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(Layers.size());
    createInfo.ppEnabledLayerNames = Layers.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    debugCreateInfo = {};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = VkDebugCallback;
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

    VULKAN_VARIFY(vkCreateInstance(&createInfo, nullptr, &Instance))
    
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) 
    {
        VULKAN_VARIFY(func(Instance, &debugCreateInfo, nullptr, &DebugMessenger))
    }
    else 
    {
        XASSERT(false);
    }
}


