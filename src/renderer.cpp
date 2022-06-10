#include "renderer.hpp"

vk::Instance Renderer::instance_ = nullptr;
vk::SurfaceKHR Renderer::surface_ = nullptr;
vk::PhysicalDevice Renderer::phyDevice_ = nullptr;
Renderer::QueueFamilyIndices Renderer::queueIndices_;
vk::Device Renderer::device_ = nullptr;
vk::Queue Renderer::graphicQueue_ = nullptr;
vk::Queue Renderer::presentQueue_ = nullptr;
vk::SwapchainKHR Renderer::swapchain_ = nullptr;
Renderer::SwapchainRequiredInfo Renderer::requiredInfo_;
std::vector<vk::Image> Renderer::images_;
std::vector<vk::ImageView> Renderer::imageViews_;
vk::Pipeline Renderer::pipeline_ = nullptr;
std::vector<vk::ShaderModule> Renderer::shaderModules_;
vk::PipelineLayout Renderer::layout_ = nullptr;
vk::RenderPass Renderer::renderPass_ = nullptr;
std::vector<vk::Framebuffer> Renderer::framebuffers_;
vk::CommandPool Renderer::cmdPool_ = nullptr;
vk::CommandBuffer Renderer::cmdBuf_ = nullptr;
vk::Semaphore Renderer::imageAvaliableSem_ = nullptr;
vk::Semaphore Renderer::renderFinishSem_ = nullptr;
vk::Fence Renderer::fence_ = nullptr;

#define CHECK_NULL(expr)  \
if (!(expr)) { \
    throw std::runtime_error(#expr " is nullptr"); \
}

void Renderer::Init(SDL_Window* window) {
    // get extensions
    std::vector<const char*> extensions{"VK_KHR_get_physical_device_properties2"};
    unsigned int count;
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    size_t oldSize = extensions.size();
    extensions.resize(extensions.size() + count);
    SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data() + oldSize);

    std::cout << "support extensions:" << std::endl;
    auto extensionNames = vk::enumerateInstanceExtensionProperties();
    for (auto& ext : extensionNames) {
        std::cout << '\t' << ext.extensionName << std::endl;
    }

    instance_ = createInstance(extensions);
    CHECK_NULL(instance_);

    surface_ = createSurface(window);
    CHECK_NULL(surface_);

    phyDevice_ = pickupPhysicalDevice();
    CHECK_NULL(phyDevice_);
    std::cout << "pickup " << phyDevice_.getProperties().deviceName << std::endl;

    queueIndices_ = queryPhysicalDevice();

    device_ = createDevice();
    CHECK_NULL(device_);

    graphicQueue_ = device_.getQueue(queueIndices_.graphicsIndices.value(), 0);
    presentQueue_ = device_.getQueue(queueIndices_.presentIndices.value(), 0);
    CHECK_NULL(graphicQueue_);;
    CHECK_NULL(presentQueue_);;

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    requiredInfo_ = querySwapchainRequiredInfo(w, h);

    swapchain_ = createSwapchain();
    CHECK_NULL(swapchain_);
    images_ = device_.getSwapchainImagesKHR(swapchain_);

    imageViews_ = createImageViews();

    layout_ = createLayout();
    CHECK_NULL(layout_);

    renderPass_ = createRenderPass();
    CHECK_NULL(renderPass_);

    framebuffers_ = createFramebuffers();
    for (auto& framebuffer : framebuffers_) {
        CHECK_NULL(framebuffer);
    }

    cmdPool_ = createCmdPool();
    CHECK_NULL(cmdPool_);

    cmdBuf_ = createCmdBuffer();
    CHECK_NULL(cmdBuf_);

    imageAvaliableSem_ = createSemaphore();
    renderFinishSem_ = createSemaphore();
    CHECK_NULL(imageAvaliableSem_);
    CHECK_NULL(renderFinishSem_);

    fence_ = createFence();
    CHECK_NULL(fence_);
}

vk::Instance Renderer::createInstance(const std::vector<const char*>& extensions) {
    // validation layers
    std::array<const char*, 1> layers{"VK_LAYER_KHRONOS_validation"};

    vk::InstanceCreateInfo info;
    info.setPEnabledExtensionNames(extensions);
    info.setPEnabledLayerNames(layers);

    return vk::createInstance(info);
}

vk::SurfaceKHR Renderer::createSurface(SDL_Window* window) {
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window, instance_, &surface)) {
        throw std::runtime_error("surface create failed");
    }
    return surface;
}

vk::PhysicalDevice Renderer::pickupPhysicalDevice() {
    return instance_.enumeratePhysicalDevices()[0];
}

Renderer::QueueFamilyIndices Renderer::queryPhysicalDevice() {
    Renderer::QueueFamilyIndices indices;
    auto families = phyDevice_.getQueueFamilyProperties(); 
    uint32_t idx = 0;
    for (auto family : families) {
        if (family.queueFlags | vk::QueueFlagBits::eGraphics) {
            indices.graphicsIndices = idx;
        }
        if (phyDevice_.getSurfaceSupportKHR(idx, surface_)) {
            indices.presentIndices = idx;
        }
        if (indices.graphicsIndices && indices.presentIndices) {
            break;
        }
        idx ++;
    }
    return indices;
}

vk::Device Renderer::createDevice() {
    std::vector<vk::DeviceQueueCreateInfo> queueInfos;

    if (queueIndices_.graphicsIndices.value() == queueIndices_.presentIndices.value()) {
        vk::DeviceQueueCreateInfo info;
        float priority = 1.0;
        info.setQueuePriorities(priority);
        info.setQueueFamilyIndex(queueIndices_.graphicsIndices.value());
        info.setQueueCount(1);
        queueInfos.push_back(info);
    } else {
        vk::DeviceQueueCreateInfo info1;
        float priority = 1.0;
        info1.setQueuePriorities(priority);
        info1.setQueueFamilyIndex(queueIndices_.graphicsIndices.value());
        info1.setQueueCount(1);

        vk::DeviceQueueCreateInfo info2;
        info2.setQueuePriorities(priority);
        info2.setQueueFamilyIndex(queueIndices_.presentIndices.value());
        info2.setQueueCount(1);

        queueInfos.push_back(info1);
        queueInfos.push_back(info2);
    }

    std::array<const char*, 2> extensions{"VK_KHR_portability_subset",
                                          VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::DeviceCreateInfo info;
    info.setPEnabledExtensionNames(extensions);
    info.setQueueCreateInfos(queueInfos);

    return phyDevice_.createDevice(info);
}

vk::SwapchainKHR Renderer::createSwapchain() {
    vk::SwapchainCreateInfoKHR info;
    info.setImageColorSpace(requiredInfo_.format.colorSpace);
    info.setImageFormat(requiredInfo_.format.format);
    info.setMinImageCount(requiredInfo_.imageCount);
    info.setImageExtent(requiredInfo_.extent);
    info.setPresentMode(requiredInfo_.presentMode);
    info.setPreTransform(requiredInfo_.capabilities.currentTransform);

    if (queueIndices_.graphicsIndices.value() == queueIndices_.presentIndices.value()) {
        info.setQueueFamilyIndices(queueIndices_.graphicsIndices.value());
        info.setImageSharingMode(vk::SharingMode::eExclusive);
    } else {
        std::array<uint32_t, 2> indices{queueIndices_.graphicsIndices.value(),
                                        queueIndices_.presentIndices.value()};
        info.setQueueFamilyIndices(indices);
        info.setImageSharingMode(vk::SharingMode::eConcurrent);
    }

    info.setClipped(true);
    info.setSurface(surface_);
    info.setImageArrayLayers(1);
    info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

    return device_.createSwapchainKHR(info);
}

Renderer::SwapchainRequiredInfo Renderer::querySwapchainRequiredInfo(int w, int h) {
    SwapchainRequiredInfo info;
    info.capabilities = phyDevice_.getSurfaceCapabilitiesKHR(surface_);
    auto formats = phyDevice_.getSurfaceFormatsKHR(surface_);
    info.format = formats[0];
    for (auto& format : formats) {
        if (format.format == vk::Format::eR8G8B8A8Srgb ||
            format.format == vk::Format::eB8G8R8A8Srgb) {
            info.format = format;
        }
    }
    info.extent.width = std::clamp<uint32_t>(w,
                info.capabilities.minImageExtent.width,
                info.capabilities.maxImageExtent.width);
    info.extent.height = std::clamp<uint32_t>(h,
                info.capabilities.minImageExtent.height,
                info.capabilities.maxImageExtent.height);

    info.imageCount = std::clamp<uint32_t>(2,
                info.capabilities.minImageCount,
                info.capabilities.maxImageCount);

    auto presentModes = phyDevice_.getSurfacePresentModesKHR(surface_);
    info.presentMode = vk::PresentModeKHR::eFifo;
    for (auto& present : presentModes) {
        if (present == vk::PresentModeKHR::eMailbox)  {
            info.presentMode = present;
        }
    }
    return info;
}

std::vector<vk::ImageView> Renderer::createImageViews() {
    std::vector<vk::ImageView> views(images_.size());
    for (int i = 0; i < views.size(); i++) {
        vk::ImageViewCreateInfo info;
        info.setImage(images_[i]);
        info.setFormat(requiredInfo_.format.format);
        info.setViewType(vk::ImageViewType::e2D);
        vk::ImageSubresourceRange range;
        range.setBaseMipLevel(0);
        range.setLevelCount(1);
        range.setBaseArrayLayer(0);
        range.setLayerCount(1);
        range.setAspectMask(vk::ImageAspectFlagBits::eColor);
        info.setSubresourceRange(range);
        vk::ComponentMapping mapping;
        info.setComponents(mapping);

        views[i] = device_.createImageView(info);
    }
    return views;
}

void Renderer::Quit() {
    device_.destroyFence(fence_);
    device_.destroySemaphore(imageAvaliableSem_);
    device_.destroySemaphore(renderFinishSem_);
    device_.freeCommandBuffers(cmdPool_, cmdBuf_);
    device_.destroyCommandPool(cmdPool_);
    for (auto& framebuffer : framebuffers_) {
        device_.destroyFramebuffer(framebuffer);
    }
    device_.destroyRenderPass(renderPass_);
    device_.destroyPipelineLayout(layout_);
    device_.destroyPipeline(pipeline_);
    for (auto& shader : shaderModules_) {
        device_.destroyShaderModule(shader);
    }
    for (auto& view : imageViews_) {
        device_.destroyImageView(view);
    }
    device_.destroySwapchainKHR(swapchain_);
    device_.destroy();
    instance_.destroySurfaceKHR(surface_);
    instance_.destroy();
}

void Renderer::CreatePipeline(vk::ShaderModule vertexShader, vk::ShaderModule fragShader) {
    vk::GraphicsPipelineCreateInfo info;

    // shader configurations
    std::array<vk::PipelineShaderStageCreateInfo, 2> stageInfos;
    stageInfos[0].setModule(vertexShader)
                 .setStage(vk::ShaderStageFlagBits::eVertex)
                 .setPName("main");
    stageInfos[1].setModule(fragShader)
                 .setStage(vk::ShaderStageFlagBits::eFragment)
                 .setPName("main");
    info.setStages(stageInfos);

    // Vertex Input
    vk::PipelineVertexInputStateCreateInfo vertexInput;
    info.setPVertexInputState(&vertexInput);

    // Input Assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAsm;
    inputAsm.setPrimitiveRestartEnable(false)
            .setTopology(vk::PrimitiveTopology::eTriangleList);
    info.setPInputAssemblyState(&inputAsm);

    // layout
    info.setLayout(layout_);

    // Viewport and Scissor
    vk::PipelineViewportStateCreateInfo viewportState;
    vk::Viewport viewport(0, 0,
                          requiredInfo_.extent.width, requiredInfo_.extent.height,
                          0, 1);
    vk::Rect2D scissor({0, 0}, requiredInfo_.extent);
    viewportState.setViewports(viewport)
                 .setScissors(scissor);
    info.setPViewportState(&viewportState);

    // Rasterization
    vk::PipelineRasterizationStateCreateInfo rastInfo;
    rastInfo.setRasterizerDiscardEnable(false)
            .setDepthClampEnable(false)
            .setDepthBiasEnable(false)
            .setLineWidth(1)
            .setCullMode(vk::CullModeFlagBits::eNone)
            .setPolygonMode(vk::PolygonMode::eFill);
    info.setPRasterizationState(&rastInfo);

    // Multisample
    vk::PipelineMultisampleStateCreateInfo multisample;
    multisample.setSampleShadingEnable(false)
               .setRasterizationSamples(vk::SampleCountFlagBits::e1);
    info.setPMultisampleState(&multisample);

    // DepthStencil
    info.setPDepthStencilState(nullptr);

    // Color blend
    vk::PipelineColorBlendStateCreateInfo colorBlend;
    vk::PipelineColorBlendAttachmentState attBlendState;
    attBlendState.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eA);
    colorBlend.setLogicOpEnable(false)
              .setAttachments(attBlendState);
    info.setPColorBlendState(&colorBlend);

    // RenderPass
    info.setRenderPass(renderPass_);

    // create pipeline
    auto result = device_.createGraphicsPipeline(nullptr, info);
    if (result.result != vk::Result::eSuccess) {
        throw std::runtime_error("pipeline create failed");
    }

    pipeline_ = result.value;
}

vk::ShaderModule Renderer::CreateShaderModule(const char* filename) {
    std::ifstream file(filename, std::ios::binary|std::ios::in);
    std::vector<char> content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
    file.close();

    vk::ShaderModuleCreateInfo info;
    info.pCode = (uint32_t*)content.data();
    info.codeSize = content.size();

    shaderModules_.push_back(device_.createShaderModule(info));
    return shaderModules_.back();
}

vk::PipelineLayout Renderer::createLayout() {
    vk::PipelineLayoutCreateInfo info;
    return device_.createPipelineLayout(info);
}

vk::RenderPass Renderer::createRenderPass() {
    vk::RenderPassCreateInfo createInfo;
    vk::AttachmentDescription attchmentDesc;
    attchmentDesc.setSamples(vk::SampleCountFlagBits::e1)
                 .setLoadOp(vk::AttachmentLoadOp::eClear)
                 .setStoreOp(vk::AttachmentStoreOp::eStore)
                 .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                 .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                 .setFormat(requiredInfo_.format.format)
                 .setInitialLayout(vk::ImageLayout::eUndefined)
                 .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
    createInfo.setAttachments(attchmentDesc);
    
    vk::SubpassDescription subpassDesc;
    vk::AttachmentReference refer;
    refer.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    refer.setAttachment(0);
    subpassDesc.setColorAttachments(refer);
    subpassDesc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

    createInfo.setSubpasses(subpassDesc);

    return device_.createRenderPass(createInfo);
}

std::vector<vk::Framebuffer> Renderer::createFramebuffers() {
    std::vector<vk::Framebuffer> result;

    for (int i = 0; i < imageViews_.size(); i++) {
        vk::FramebufferCreateInfo info;
        info.setRenderPass(renderPass_);
        info.setLayers(1);
        info.setWidth(requiredInfo_.extent.width);
        info.setHeight(requiredInfo_.extent.height);
        info.setAttachments(imageViews_[i]);

        result.push_back(device_.createFramebuffer(info));
    }

    return result;
}

vk::CommandPool Renderer::createCmdPool() {
    vk::CommandPoolCreateInfo info;
    info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(queueIndices_.graphicsIndices.value());

    return device_.createCommandPool(info);
}

vk::CommandBuffer Renderer::createCmdBuffer() {
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.setCommandPool(cmdPool_)
             .setCommandBufferCount(1)
             .setLevel(vk::CommandBufferLevel::ePrimary);

    return device_.allocateCommandBuffers(allocInfo)[0];
}

void Renderer::recordCmd(vk::CommandBuffer buf, vk::Framebuffer fbo) {
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    if (buf.begin(&beginInfo) != vk::Result::eSuccess) {
        throw std::runtime_error("command buffer record failed");
    }

    vk::RenderPassBeginInfo renderPassBegin;
    vk::ClearColorValue cvalue(std::array<float, 4>{0.1, 0.1, 0.1, 1});
    vk::ClearValue value(cvalue);
    renderPassBegin.setRenderPass(renderPass_)
                   .setRenderArea(vk::Rect2D({0, 0}, requiredInfo_.extent))
                   .setClearValues(value)
                   .setFramebuffer(fbo);

    buf.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);

    buf.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_);

    buf.draw(3, 1, 0, 0);

    buf.endRenderPass();

    buf.end();
}

void Renderer::Render() {
    device_.resetFences(fence_);

    // acquire a image from swapchain
    auto result = device_.acquireNextImageKHR(swapchain_, std::numeric_limits<uint64_t>::max(), imageAvaliableSem_, nullptr);
    if (result.result != vk::Result::eSuccess) {
        throw std::runtime_error("acquire image failed");
    }
    uint32_t imageIndex = result.value;

    cmdBuf_.reset();
    recordCmd(cmdBuf_, framebuffers_[imageIndex]);


    vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(cmdBuf_)
              .setSignalSemaphores(renderFinishSem_)
              .setWaitSemaphores(imageAvaliableSem_)
              .setWaitDstStageMask(flags);
    graphicQueue_.submit(submitInfo, fence_);

    vk::PresentInfoKHR presentInfo;
    presentInfo.setImageIndices(imageIndex)
               .setSwapchains(swapchain_)
               .setWaitSemaphores(renderFinishSem_);
    if (presentQueue_.presentKHR(presentInfo) != vk::Result::eSuccess) {
        throw std::runtime_error("present failed");
    }

    if (device_.waitForFences(fence_, true, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) {
        throw std::runtime_error("wait fence failed");
    }
}

vk::Semaphore Renderer::createSemaphore() {
    vk::SemaphoreCreateInfo info;
    return device_.createSemaphore(info);
}

vk::Fence Renderer::createFence() {
    vk::FenceCreateInfo createInfo;
    return device_.createFence(createInfo);
}

void Renderer::WaitIdle() {
    device_.waitIdle();
}