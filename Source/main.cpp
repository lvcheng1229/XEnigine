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
const int MAX_FRAMES_IN_FLIGHT = 2;
#include "Runtime\RenderCore\GlobalShader.h"

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
#endif


class XSandBox
{
public:
#if !USE_DX12
    VkHack mVkHack;
    VkPipelineLayout pipelineLayout;
    //VkRenderPass renderPass;
    //std::vector<VkFramebuffer> swapChainFramebuffers;

    VkPipeline graphicsPipeline;

    //VkCommandPool commandPool;
    //std::vector<VkCommandBuffer> commandBuffers;

    //std::vector<VkSemaphore> imageAvailableSemaphores;
    //std::vector<VkSemaphore> renderFinishedSemaphores;
    //std::vector<VkFence> inFlightFences;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

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
    static std::vector<char> readFile(const std::string& filename) 
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) 
        {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(mVkHack.GetVkDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    void createGraphicsPipeline() {
        std::cout << "CreateShaderShawn" << std::endl;
        static bool bPSOCreated = false;
        if (!bPSOCreated)
        {
            std::cout << "CreateShaderShawn" << std::endl;
            bPSOCreated = true;
        }
        else
        {
            std::cout << "CreateShaderShawn2" << std::endl;
            return;
        }

        auto vertShaderCode = readFile("G:/XEngineF/XEnigine/Source/Shaders/VulkanShaderTest/vert.spv");
        auto fragShaderCode = readFile("G:/XEngineF/XEnigine/Source/Shaders/VulkanShaderTest/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(mVkHack.GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = mVkHack.GetVkRenderPas();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(mVkHack.GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(mVkHack.GetVkDevice(), fragShaderModule, nullptr);
        vkDestroyShaderModule(mVkHack.GetVkDevice(), vertShaderModule, nullptr);
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        XRHITexture* BackTex = RHIGetCurrentBackTexture();
        XRHIRenderPassInfo RPInfos(1, &BackTex, ERenderTargetLoadAction::EClear, nullptr, EDepthStencilLoadAction::ENoAction);
        RHICmdList.RHIBeginRenderPass(RPInfos, "VulkanTestRP", sizeof("VulkanTestRP"));
        
        XGraphicsPSOInitializer GraphicsPSOInit;
        GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
        GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
        GraphicsPSOInit.RasterState = TStaticRasterizationState<>::GetRHI();

        TShaderReference<XHLSL2SPIRVS> VertexShader = GetGlobalShaderMapping()->GetShader<XHLSL2SPIRVS>();
        TShaderReference<XHLSL2SPIRPS> PixelShader = GetGlobalShaderMapping()->GetShader<XHLSL2SPIRPS>();
        
        GraphicsPSOInit.BoundShaderState.RHIVertexShader = VertexShader.GetVertexShader();
        GraphicsPSOInit.BoundShaderState.RHIPixelShader = PixelShader.GetPixelShader();
        //GraphicsPSOInit.BoundShaderState.RHIVertexLayout = GFullScreenLayout.RHIVertexLayout.get();
        
        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        SetGraphicsPipelineStateFromPSOInit(RHICmdList, GraphicsPSOInit);
        //
        //createGraphicsPipeline();
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

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

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

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
    //_CrtSetBreakAlloc(473);
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

