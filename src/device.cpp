#include "device.hpp"

namespace toy2d {

Device::Device(vk::PhysicalDevice phyDevice, int w, int h, vk::SurfaceKHR surface): phyDevice_(phyDevice) {
    queueIndices_ = queryPhysicalDevice(surface);
    device_ = createDevice();
    ASSERT(device_);
    graphicsQueue_ = device_.getQueue(queueIndices_.graphicsIndices.value(), 0);
    presentQueue_ = device_.getQueue(queueIndices_.presentIndices.value(), 0);

    requiredInfo_ = querySwapchainRequiredInfo(w, h, surface);
    swapchain_ = createSwapchain(surface);
    ASSERT(swapchain_);

    images_ = device_.getSwapchainImagesKHR(swapchain_);
    imageViews_ = createImageViews();
}


Device::QueueFamilyIndices Device::queryPhysicalDevice(vk::SurfaceKHR surface) {
    QueueFamilyIndices indices;
    auto families = phyDevice_.getQueueFamilyProperties(); 
    uint32_t idx = 0;
    for (auto family : families) {
        if (family.queueFlags | vk::QueueFlagBits::eGraphics) {
            indices.graphicsIndices = idx;
        }
        if (phyDevice_.getSurfaceSupportKHR(idx, surface)) {
            indices.presentIndices = idx;
        }
        if (indices.graphicsIndices && indices.presentIndices) {
            break;
        }
        idx ++;
    }
    return indices;
}

vk::Device Device::createDevice() {
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

    std::vector<const char*> extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#ifdef MACOS
    extensions.push_back("VK_KHR_portability_subset");
#endif

    vk::DeviceCreateInfo info;
    info.setPEnabledExtensionNames(extensions);
    info.setQueueCreateInfos(queueInfos);

    return phyDevice_.createDevice(info);
}

vk::SwapchainKHR Device::createSwapchain(vk::SurfaceKHR surface) {
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
    info.setSurface(surface);
    info.setImageArrayLayers(1);
    info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

    return device_.createSwapchainKHR(info);
}

Device::SwapchainRequiredInfo Device::querySwapchainRequiredInfo(int w, int h, vk::SurfaceKHR surface) {
    SwapchainRequiredInfo info;
    info.capabilities = phyDevice_.getSurfaceCapabilitiesKHR(surface);
    auto formats = phyDevice_.getSurfaceFormatsKHR(surface);
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

    auto presentModes = phyDevice_.getSurfacePresentModesKHR(surface);
    info.presentMode = vk::PresentModeKHR::eFifo;
    for (auto& present : presentModes) {
        if (present == vk::PresentModeKHR::eMailbox)  {
            info.presentMode = present;
        }
    }
    return info;
}

std::vector<vk::ImageView> Device::createImageViews() {
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

vk::CommandPool Device::CreateCmdPool() {
    vk::CommandPoolCreateInfo info;
    info.setQueueFamilyIndex(queueIndices_.graphicsIndices.value())
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

    return device_.createCommandPool(info);
}

void Device::DestroyCmdPool(vk::CommandPool pool) {
    device_.destroyCommandPool(pool);
}

vk::CommandBuffer Device::AllocateCmdBuffer(vk::CommandPool pool) {
    return AllocateCmdBuffers(pool, 1)[0];
}

std::vector<vk::CommandBuffer> Device::AllocateCmdBuffers(vk::CommandPool pool, uint32_t num) {
    vk::CommandBufferAllocateInfo info;
    info.setCommandBufferCount(num)
        .setCommandPool(pool)
        .setLevel(vk::CommandBufferLevel::ePrimary);

    return device_.allocateCommandBuffers(info);
}

void Device::FreeCmdBuffer(vk::CommandPool pool, vk::CommandBuffer buffer) {
    device_.freeCommandBuffers(pool, buffer);
}

void Device::FreeCmdBuffers(vk::CommandPool pool, const std::vector<vk::CommandBuffer>& buffers) {
    device_.freeCommandBuffers(pool, buffers);
}

vk::Semaphore Device::CreateSemaphore() {
    vk::SemaphoreCreateInfo info;

    return device_.createSemaphore(info);
}

void Device::DestroySemaphore(vk::Semaphore sem) {
    device_.destroySemaphore(sem);
}

vk::Fence Device::CreateFence(bool signaled) {
    vk::FenceCreateInfo info;
    if (signaled) {
        info.setFlags(vk::FenceCreateFlagBits::eSignaled);
    }

    return device_.createFence(info);
}

void Device::DestroyFence(vk::Fence fence) {
    device_.destroyFence(fence);
}

void Device::ResetFence(vk::Fence fence) {
    device_.resetFences(fence);
}

unsigned int Device::AcquireNextImage(vk::Semaphore sem) {
    auto result = device_.acquireNextImageKHR(swapchain_, std::numeric_limits<uint64_t>::max(), sem);
    ASSERT(result.result == vk::Result::eSuccess);
    return result.value;
}

Device::~Device() {
    for (auto& view : imageViews_) {
        device_.destroyImageView(view);
    }
    if (swapchain_) {
        device_.destroySwapchainKHR(swapchain_);
    }
    if (device_) {
        device_.destroy();
    }
}

Device::MemRerquiedInfo Device::queryMemInfo(vk::Buffer buffer, vk::MemoryPropertyFlags flag) {
    auto property = phyDevice_.getMemoryProperties(); 
    auto requirement = device_.getBufferMemoryRequirements(buffer);

    MemRerquiedInfo info;
    info.size = requirement.size;

    for (int i = 0; i < property.memoryTypeCount; i++) {
        if ((requirement.memoryTypeBits & (1 << i)) &&
             property.memoryTypes[i].propertyFlags & (flag)) {
            info.index = i;
        }
    }

    return info;
}

Buffer Device::CreateBuffer(vk::BufferUsageFlags usage,
                            uint32_t size,
                            vk::MemoryPropertyFlags property) {
    Buffer buffer;
    buffer.buffer = createBuffer(usage, size);
    ASSERT(buffer.buffer);
    buffer.memory = allocateMem(buffer.buffer, property);
    ASSERT(buffer.memory);
    buffer.size = size;

    device_.bindBufferMemory(buffer.buffer, buffer.memory, 0);

    return buffer;
}

void Device::DestroyBuffer(const Buffer& buffer) {
    device_.destroyBuffer(buffer.buffer);
    device_.freeMemory(buffer.memory);
}

vk::Buffer Device::createBuffer(vk::BufferUsageFlags usage, uint32_t size) {
    vk::BufferCreateInfo info;
    info.setSharingMode(vk::SharingMode::eExclusive)
        .setQueueFamilyIndices(queueIndices_.graphicsIndices.value())
        .setSize(size)
        .setUsage(usage);
    return device_.createBuffer(info);
}

vk::DeviceMemory Device::allocateMem(vk::Buffer buffer, vk::MemoryPropertyFlags property) {
    auto requirement = queryMemInfo(buffer, property);

    vk::MemoryAllocateInfo info;
    info.setAllocationSize(requirement.size)
        .setMemoryTypeIndex(requirement.index);

    return device_.allocateMemory(info);
}

void* Device::MapMemory(const Buffer& buffer) {
    return device_.mapMemory(buffer.memory, 0, buffer.size);
}

void Device::UnmapMemory(const Buffer& buffer) {
    device_.unmapMemory(buffer.memory);
}

void Device::WaitForFence(vk::Fence fence, uint32_t timeout) {
    ASSERT(device_.waitForFences(fence, true, timeout) == vk::Result::eSuccess);
}

void Device::WaitIdle() {
    device_.waitIdle();
}

vk::Framebuffer Device::CreateFramebuffer(vk::RenderPass renderPass, vk::ImageView view) {
    vk::FramebufferCreateInfo info;
    info.setRenderPass(renderPass);
    info.setLayers(1);
    info.setWidth(requiredInfo_.extent.width);
    info.setHeight(requiredInfo_.extent.height);
    info.setAttachments(view);

    return device_.createFramebuffer(info);
}

void Device::DestroyFramebuffer(vk::Framebuffer fb) {
    device_.destroyFramebuffer(fb);
}

vk::ShaderModule Device::CreateShaderModule(const char* filename) {
    std::ifstream file(filename, std::ios_base::binary);
    if (file.fail()) {
        Log("%s load failed", filename);
		ASSERT(!file.fail());
    }
    file.seekg(0, std::ios_base::end);
    size_t size = file.tellg();
    std::vector<char> content(size);
    file.seekg(0, std::ios_base::beg);
    file.read(content.data(), size);

    vk::ShaderModuleCreateInfo info;
    info.pCode = (uint32_t*)content.data();
    info.codeSize = content.size();

    file.close();

    return device_.createShaderModule(info);
}

void Device::DestroyShaderModule(vk::ShaderModule module) {
    device_.destroyShaderModule(module);
}

vk::Pipeline Device::CreatePipeline(vk::ShaderModule vertexShader, vk::ShaderModule fragShader, const Vec2& windowSize, vk::RenderPass renderPass, vk::PipelineLayout layout) {
    vk::GraphicsPipelineCreateInfo pipelineInfo;

    std::array<vk::PipelineShaderStageCreateInfo, 2> stage;
    stage[0].setStage(vk::ShaderStageFlagBits::eVertex)
            .setPName("main")
            .setModule(vertexShader);
    stage[1].setStage(vk::ShaderStageFlagBits::eFragment)
            .setPName("main")
            .setModule(fragShader);

    vk::PipelineVertexInputStateCreateInfo vertexInputState;
    auto vertexBinding = Vertex::GetBindingDescription();
    auto vertexAttribute = Vertex::GetAttrDescription();
    vertexInputState.setVertexBindingDescriptions(vertexBinding)
                    .setVertexAttributeDescriptions(vertexAttribute);

    vk::PipelineInputAssemblyStateCreateInfo inputAsmInfo;
    inputAsmInfo.setTopology(vk::PrimitiveTopology::eTriangleList)
                .setPrimitiveRestartEnable(false);

    vk::PipelineViewportStateCreateInfo viewportInfo;
    vk::Viewport viewport(0, 0, windowSize.x, windowSize.y);
    vk::Rect2D scissor({0, 0}, {(uint32_t)windowSize.x, (uint32_t)windowSize.y});
    viewportInfo.setViewports(viewport)
                .setScissors(scissor);

    vk::PipelineRasterizationStateCreateInfo rastInfo;
    rastInfo.setRasterizerDiscardEnable(false)
            .setDepthClampEnable(false)
            .setDepthBiasEnable(false)
            .setLineWidth(1)
            .setCullMode(vk::CullModeFlagBits::eNone)
            .setPolygonMode(vk::PolygonMode::eFill);

    vk::PipelineMultisampleStateCreateInfo multisampleInfo;
    multisampleInfo.setAlphaToCoverageEnable(false)
                   .setAlphaToOneEnable(false)
                   .setSampleShadingEnable(false)
                   .setRasterizationSamples(vk::SampleCountFlagBits::e1);

    vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
    vk::PipelineColorBlendAttachmentState attState;
    attState.setColorWriteMask(vk::ColorComponentFlagBits::eA |
                               vk::ColorComponentFlagBits::eB |
                               vk::ColorComponentFlagBits::eG |
                               vk::ColorComponentFlagBits::eR);
    colorBlendInfo.setLogicOpEnable(false)
                  .setAttachments(attState);

    pipelineInfo.setStages(stage)
                .setPVertexInputState(&vertexInputState)
                .setPInputAssemblyState(&inputAsmInfo)
                .setLayout(layout)
                .setPViewportState(&viewportInfo)
                .setPRasterizationState(&rastInfo)
                .setPMultisampleState(&multisampleInfo)
                .setPDepthStencilState(nullptr)
                .setPColorBlendState(&colorBlendInfo)
                .setRenderPass(renderPass);

    auto result = device_.createGraphicsPipeline(nullptr, pipelineInfo);
    ASSERT(result.result == vk::Result::eSuccess);

    return result.value;
}

void Device::DestroyPipeline(vk::Pipeline pipeline) {
    device_.destroyPipeline(pipeline);
}

vk::PipelineLayout Device::CreateLayout(vk::DescriptorSetLayout setLayout) {
    vk::PipelineLayoutCreateInfo info;
    vk::PushConstantRange constantRange;
    constantRange.setOffset(0)
                 .setSize(sizeof(float))
                 .setStageFlags(vk::ShaderStageFlagBits::eVertex);
    info.setSetLayouts(setLayout)
        .setPushConstantRanges(constantRange);

    return device_.createPipelineLayout(info);
}

void Device::DestroyLayout(vk::PipelineLayout layout) {
    device_.destroyPipelineLayout(layout);
}

vk::DescriptorSetLayout Device::CreateDescriptorSetLayout(vk::DescriptorSetLayoutBinding binding) {
    vk::DescriptorSetLayoutCreateInfo info;
    info.setBindings(binding);
    return device_.createDescriptorSetLayout(info);
}

void Device::DestroyDescriptorSetLayout(vk::DescriptorSetLayout layout) {
    device_.destroyDescriptorSetLayout(layout);
}

void Device::UpdateDescriptorSet(vk::WriteDescriptorSet writeSet) {
    device_.updateDescriptorSets(writeSet, {});
}

vk::DescriptorPool Device::CreateDescriptorPool() {
    vk::DescriptorPoolCreateInfo info;
    vk::DescriptorPoolSize size;
    size.setType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1);
    info.setPoolSizes(size)
        .setMaxSets(requiredInfo_.imageCount)
        .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
    return device_.createDescriptorPool(info);
}

void Device::DestroyDescriptorPool(vk::DescriptorPool pool) {
    device_.destroyDescriptorPool(pool);
}

vk::DescriptorSet Device::AllocateDescriptorSet(vk::DescriptorPool pool, vk::DescriptorSetLayout layout) {
    vk::DescriptorSetAllocateInfo info;
    info.setSetLayouts(layout)
        .setDescriptorPool(pool)
        .setDescriptorSetCount(1);
    return device_.allocateDescriptorSets(info)[0];
}

void Device::FreeDescriptorSet(vk::DescriptorPool pool, vk::DescriptorSet set) {
    device_.freeDescriptorSets(pool, set);
}

vk::RenderPass Device::CreateRenderPass() {
    vk::RenderPassCreateInfo info;

    vk::AttachmentDescription attDesc;
    attDesc.setSamples(vk::SampleCountFlagBits::e1)
           .setLoadOp(vk::AttachmentLoadOp::eClear)
           .setStoreOp(vk::AttachmentStoreOp::eStore)
           .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
           .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
           .setFormat(requiredInfo_.format.format)
           .setInitialLayout(vk::ImageLayout::eUndefined)
           .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    info.setAttachments(attDesc);

    vk::SubpassDescription subpassDesc;
    vk::AttachmentReference reference;
    reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
             .setAttachment(0);
    subpassDesc.setColorAttachments(reference)
               .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

    vk::SubpassDependency subpassDep;
    subpassDep.setSrcSubpass(VK_SUBPASS_EXTERNAL)
              .setDstSubpass(0)
              .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
              .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
              .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

    info.setSubpasses(subpassDesc)
        .setDependencies(subpassDep);

    return device_.createRenderPass(info);
}

void Device::DestroyRenderPass(vk::RenderPass renderPass) {
    device_.destroyRenderPass(renderPass);
}

}
