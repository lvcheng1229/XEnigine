#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb_image.h"

#include "Runtime/Core/MainInit.h"
#include "Runtime/Engine/ResourcecConverter.h"
#include "Runtime/Render/DeferredShadingRenderer.h"
#include "Runtime/Core/ComponentNode/GameTimer.h"

#include "Runtime/ApplicationCore/Windows/WindowsApplication.h"
#include "Runtime/ApplicationCore/GlfwApp/GlfwApplication.h"
XApplication* XApplication::Application = nullptr;

#if !USE_DX12
#include <fstream>
#include "Runtime\VulkanRHI\VulkanResource.h"
#include <Runtime\VulkanRHI\VulaknHack.h>
#include "Runtime\RHI\RHIStaticStates.h"
#include "Runtime\RHI\PipelineStateCache.h"
const int MAX_FRAMES_IN_FLIGHT = 1;
#include "Runtime\RenderCore\GlobalShader.h"
#include <chrono>

class XHLSL2SPIRPS :public XGloablShader
{
public:
    static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
    {
        return new XHLSL2SPIRPS(Initializer);
    }

    static ShaderInfos StaticShaderInfos;
    static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}

public:
    XHLSL2SPIRPS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer) {}
};

class XHLSL2SPIRVS :public XGloablShader
{
public:
    static XXShader* CustomConstrucFunc(const XShaderInitlizer& Initializer)
    {
        return new XHLSL2SPIRVS(Initializer);
    }
    static ShaderInfos StaticShaderInfos;
    static void ModifyShaderCompileSettings(XShaderCompileSetting& OutSettings) {}

public:
    XHLSL2SPIRVS(const XShaderInitlizer& Initializer) :XGloablShader(Initializer) {}
};
XHLSL2SPIRVS::ShaderInfos XHLSL2SPIRVS::StaticShaderInfos(
    "XHLSL2SPIRVS", GET_SHADER_PATH("VulkanShaderTest/hlsl2spirtest.hlsl"),
    "VS", EShaderType::SV_Vertex, XHLSL2SPIRVS::CustomConstrucFunc,
    XHLSL2SPIRVS::ModifyShaderCompileSettings);
XHLSL2SPIRPS::ShaderInfos XHLSL2SPIRPS::StaticShaderInfos(
    "XHLSL2SPIRPS", GET_SHADER_PATH("VulkanShaderTest/hlsl2spirtest.hlsl"),
    "PS", EShaderType::SV_Pixel, XHLSL2SPIRPS::CustomConstrucFunc,
    XHLSL2SPIRPS::ModifyShaderCompileSettings);

class XTestVertexLayout : public XRenderResource
{
public:
    std::shared_ptr<XRHIVertexLayout> RHIVertexLayout;
    virtual void InitRHI()override
    {
        XRHIVertexLayoutArray LayoutArray;
        LayoutArray.push_back(XVertexElement(0, EVertexElementType::VET_Float2, 0, 0));
        LayoutArray.push_back(XVertexElement(1, EVertexElementType::VET_Float3, 0, 0 + sizeof(DirectX::XMFLOAT2)));
        RHIVertexLayout = RHICreateVertexLayout(LayoutArray);
    }

    virtual void ReleaseRHI()override
    {
        RHIVertexLayout.reset();
    }
};
TGlobalResource<XTestVertexLayout> GTestVtxLayout;

struct XTestVertex
{
    XVector2 Position;
    XVector3 Color;
    struct XTestVertex
    (XVector2 PositionIn, XVector3 ColorIn)
        :Position(PositionIn), Color(ColorIn) {}
};

class XTestVertexBuffer :public RVertexBuffer
{
public:
    void InitRHI()override
    {
        TResourceVector<XTestVertex>Vertices;
        Vertices.PushBack(XTestVertex(XVector2(-0.5f, -0.5f), XVector3(1.0f, 0.0f, 0.0f)));
        Vertices.PushBack(XTestVertex(XVector2(0.5f, -0.5f), XVector3(0.0f, 1.0f, 0.0f)));
        Vertices.PushBack(XTestVertex(XVector2(0.5f, 0.5f), XVector3(0.0f, 0.0f, 1.0f)));
        Vertices.PushBack(XTestVertex(XVector2(-0.5f, 0.5f), XVector3(1.0f, 1.0f, 1.0f)));

        XRHIResourceCreateData CreateData(&Vertices);
        RHIVertexBuffer = RHIcreateVertexBuffer(sizeof(XTestVertex), 3 * sizeof(XTestVertex), EBufferUsage::BUF_Static, CreateData);
    }
};

class XTestIndexBuffer :public RVertexBuffer
{
public:
    void InitRHI()override
    {
        TResourceVector<uint16> Indecies;
        Indecies.PushBack(0);
        Indecies.PushBack(1);
        Indecies.PushBack(2);
        Indecies.PushBack(2);
        Indecies.PushBack(3);
        Indecies.PushBack(0);
        XRHIResourceCreateData CreateData(&Indecies);
        RHIVertexBuffer = RHIcreateVertexBuffer(sizeof(uint16), 6 * sizeof(uint16), EBufferUsage::BUF_Static, CreateData);
    }
};
TGlobalResource<XTestVertexBuffer> GTestVertexRHI;
TGlobalResource<XTestIndexBuffer> GTestIndexRHI;

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
#endif

class XSandBox
{
public:
#if !USE_DX12
    VkHack mVkHack;
    //VkPipelineLayout pipelineLayout;

    //VkBuffer vertexBuffer;
    //VkDeviceMemory vertexBufferMemory;

    //VkRenderPass renderPass;
    //std::vector<VkFramebuffer> swapChainFramebuffers;

    //VkPipeline graphicsPipeline;

    //VkCommandPool commandPool;
    //std::vector<VkCommandBuffer> commandBuffers;

    //std::vector<VkSemaphore> imageAvailableSemaphores;
    //std::vector<VkSemaphore> renderFinishedSemaphores;
    //std::vector<VkFence> inFlightFences;
    //VkDescriptorSetLayout descriptorSetLayout;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    //VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    //uint32_t currentFrame = 0;
#endif

    XRHICommandList RHICmdList;

#if USE_DX12
    XBoundSphere SceneBoundingSphere;
    XDeferredShadingRenderer DeferredShadingRenderer;
    std::vector<std::shared_ptr<GGeomertry>> RenderGeos;
#endif

    //Timer
    GameTimer mTimer;
    
    //Camera
    GCamera CamIns;
    float Far = 1000.0f;
    float Near = 1.0f;
    float FoVAngleY = 0.25f * X_PI;

    //Light
    XVector3 LightDir = {1 / sqrtf(3.0f), 1 / sqrtf(3.0f), -1 / sqrtf(3.0f)};
    XVector3 LightColor = {1, 1, 1};
    float LightIntensity = 7.0f;
#if USE_DX12
    void SceneBuild()
    {
        std::shared_ptr<GGeomertry> DefaultSphere = TempCreateSphereGeoWithMat();
        DefaultSphere->SetWorldTranslate(XVector3(0, -0.5, 0));
    
        std::shared_ptr<GGeomertry> DefaultCube = TempCreateCubeGeoWithMat();
        DefaultCube->SetWorldTranslate(XVector3(-1, 1.5, 0));
        DefaultCube->GetMaterialInstance()->SetMaterialValueFloat("ConstantMetatllic", 0.8);
        DefaultCube->GetMaterialInstance()->SetMaterialValueFloat("ConstantRoughness", 0.6);
    
        std::shared_ptr<GGeomertry> DefaultCubeRight = DefaultCube->CreateGeoInstancewithMat();
        DefaultCubeRight->SetWorldTranslate(XVector3(1, 1.5, 0));
    
        std::shared_ptr<GGeomertry> DefaultQuad = TempCreateQuadGeoWithMat();
        DefaultQuad->SetWorldTranslate(XVector3(0.0, 1.0, 0.0));
    
        std::shared_ptr<GGeomertry> LeftQuad = DefaultQuad->CreateGeoInstancewithMat();
        LeftQuad->SetWorldRotate(XVector3(0, 0, 1), -(3.14159 * 0.5));
        LeftQuad->SetWorldTranslate(XVector3(-2.0, 0.0, 0.0));
    
        std::shared_ptr<GGeomertry> FrontQuad = DefaultQuad->CreateGeoInstancewithMat();
        FrontQuad->SetWorldRotate(XVector3(1, 0, 0), -(3.14159 * 0.5));
        FrontQuad->SetWorldTranslate(XVector3(0.0, 0.0, 2.0));
    
        RenderGeos.push_back(DefaultCube);
        RenderGeos.push_back(DefaultCubeRight);
        RenderGeos.push_back(DefaultSphere);
        RenderGeos.push_back(DefaultQuad);
        RenderGeos.push_back(LeftQuad);
        RenderGeos.push_back(FrontQuad);
    
        //OBjLoaderTest(RenderGeos);
        for (auto& t : RenderGeos)
        {
            t->GetGVertexBuffer()->CreateRHIBufferChecked();
            t->GetGIndexBuffer()->CreateRHIBufferChecked();
        }
    }
#else

    /*void createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(mVkHack.GetVkDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    }*/

    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

            vkMapMemory(mVkHack.GetVkDevice(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        }
    }

    void createDescriptorPool() {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        if (vkCreateDescriptorPool(mVkHack.GetVkDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, mVkHack.GetVkDescriptorSetLayout());
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(mVkHack.GetVkDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(mVkHack.GetVkDevice(), 1, &descriptorWrite, 0, nullptr);
        }
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(mVkHack.GetVkDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(mVkHack.GetVkDevice(), buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(mVkHack.GetVkDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(mVkHack.GetVkDevice(), buffer, bufferMemory, 0);

    }
    void updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), float(XApplication::Application->ClientWidth) / float(XApplication::Application->ClientHeight), 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));

    }
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(mVkHack.GetVkPhyDevice(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        updateUniformBuffer(0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        XRHITexture* BackTex = RHIGetCurrentBackTexture();
        XRHIRenderPassInfo RPInfos(1, &BackTex, ERenderTargetLoadAction::EClear, nullptr, EDepthStencilLoadAction::ENoAction);
        RHICmdList.RHIBeginRenderPass(RPInfos, "VulkanTestRP", sizeof("VulkanTestRP"));
        RHICmdList.CacheActiveRenderTargets(RPInfos);

        XGraphicsPSOInitializer GraphicsPSOInit;
        GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
        GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
        GraphicsPSOInit.RasterState = TStaticRasterizationState<>::GetRHI();

        TShaderReference<XHLSL2SPIRVS> VertexShader = GetGlobalShaderMapping()->GetShader<XHLSL2SPIRVS>();
        TShaderReference<XHLSL2SPIRPS> PixelShader = GetGlobalShaderMapping()->GetShader<XHLSL2SPIRPS>();
        
        GraphicsPSOInit.BoundShaderState.RHIVertexShader = VertexShader.GetVertexShader();
        GraphicsPSOInit.BoundShaderState.RHIPixelShader = PixelShader.GetPixelShader();
        GraphicsPSOInit.BoundShaderState.RHIVertexLayout = GTestVtxLayout.RHIVertexLayout.get();
        
        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        SetGraphicsPipelineStateFromPSOInit(RHICmdList, GraphicsPSOInit);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)mVkHack.GetBkBufferExtent().width;
        viewport.height = (float)mVkHack.GetBkBufferExtent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = mVkHack.GetBkBufferExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        XVulkanAllocation& TT = static_cast<XVulkanResourceMultiBuffer*>(GTestVertexRHI.RHIVertexBuffer.get())->Buffer;
        VkBuffer vertexBuffers[] = { TT.VulkanHandle };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, static_cast<XVulkanResourceMultiBuffer*>(GTestIndexRHI.RHIVertexBuffer.get())->Buffer.VulkanHandle, 0, VK_INDEX_TYPE_UINT16);
       
        static bool bInitOnce = false;
        if (!bInitOnce)
        {
            createDescriptorPool();
            createDescriptorSets();
            bInitOnce = true;
        }


        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mVkHack.GetVkPipelineLayout(), 0, 1, &descriptorSets[0], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
        //vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void createSyncObjects() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(mVkHack.GetVkDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(mVkHack.GetVkDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(mVkHack.GetVkDevice(), &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }

    }
    void drawFrame2() {
        vkWaitForFences(mVkHack.GetVkDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(mVkHack.GetVkDevice(), 1, &inFlightFence);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(mVkHack.GetVkDevice(), mVkHack.GetVkSwapChain(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        vkResetCommandBuffer(*mVkHack.GetCmdBuffer(), /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(*mVkHack.GetCmdBuffer(), imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = (mVkHack.GetCmdBuffer());

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(mVkHack.GetVkQueue(), 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { mVkHack.GetVkSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(mVkHack.GetVkQueue(), &presentInfo);
        mVkHack.TempPresent();
    }
#endif
    void Init()
    {
        MainInit::Init();

#if USE_DX12
        XApplication::Application = new XWindowsApplication();
#else
        XApplication::Application = new XGlfwApplication();
#endif
        XApplication::Application->CreateAppWindow();
#if USE_DX12
        XApplication::Application->SetRenderer(&DeferredShadingRenderer);
#endif
        XApplication::Application->AppInput.SetCamera(&CamIns);
        XApplication::Application->AppInput.SetTimer(&mTimer);

        RHIInit(XApplication::Application->ClientWidth, XApplication::Application->ClientHeight, USE_DX12);

        RHICmdList = GRHICmdList;
#if USE_DX12
        RHICmdList.Open();
        SceneBuild();
#endif
        MainInit::InitAfterRHI();
#if USE_DX12
        XApplication::Application->UISetup();
        SceneBoundingSphere.Center = XVector3(0, 0, 0);
        SceneBoundingSphere.Radius = 48.0f;
        float AspectRatio = static_cast<float>(XApplication::Application->ClientWidth) / static_cast<float>(XApplication::Application->
            ClientHeight);
        CamIns.SetPerspective(FoVAngleY, AspectRatio, Near, Far);
        DeferredShadingRenderer.ViewInfoSetup(XApplication::Application->ClientWidth, XApplication::Application->ClientHeight, CamIns);
        DeferredShadingRenderer.Setup(RenderGeos,
            XBoundSphere{ SceneBoundingSphere.Center, SceneBoundingSphere.Radius },
            LightDir, LightColor, LightIntensity, RHICmdList);
        RHICmdList.Execute();
#else
        //createDescriptorSetLayout();
        createUniformBuffers();

        createSyncObjects();
#endif
    }

    void Destroy()
    {
        MainInit::Destroy();
        RHIRelease();
    }
};

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(10686);
    {
        XSandBox SandBox;
        SandBox.Init();
#if !USE_DX12
        while (!glfwWindowShouldClose((GLFWwindow*)XApplication::Application->GetPlatformHandle())) {
            glfwPollEvents();
            SandBox.drawFrame2();
        }
#else
        XApplication::Application->ApplicationLoop();
#endif
        SandBox.Destroy();
    }
    {
        delete XWindowsApplication::Application;
        return 0;
    }
    _CrtDumpMemoryLeaks();
}

