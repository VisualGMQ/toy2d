#include "toy2d/renderer.hpp"
#include "toy2d/vertex.hpp"

namespace toy2d {

Renderer::Renderer(int maxFlightCount): maxFlightCount_(maxFlightCount), curFrame_(0) {
    createFences();
    createSemaphores();
    createCmdBuffers();
    createBuffers();
    bufferData();
}

Renderer::~Renderer() {
    auto& device = Context::Instance().device;
    device.destroyBuffer(verticesBuffer_.buffer);
    device.freeMemory(verticesBuffer_.memory);
    device.destroyBuffer(indicesBuffer_.buffer);
    device.freeMemory(indicesBuffer_.memory);
    for (auto& sem : imageAvaliableSems_) {
        device.destroySemaphore(sem);
    }
    for (auto& sem : renderFinishSems_) {
        device.destroySemaphore(sem);
    }
    for (auto& fence : fences_) {
        device.destroyFence(fence);
    }
}

void Renderer::DrawRect() {
    auto& ctx = Context::Instance();
    auto& device = ctx.device;
    if (device.waitForFences(fences_[curFrame_], true, std::numeric_limits<std::uint64_t>::max()) != vk::Result::eSuccess) {
        throw std::runtime_error("wait for fence failed");
    }
    device.resetFences(fences_[curFrame_]);

    auto& swapchain = ctx.swapchain;
    auto resultValue = device.acquireNextImageKHR(swapchain->swapchain, std::numeric_limits<std::uint64_t>::max(), imageAvaliableSems_[curFrame_], nullptr);
    if (resultValue.result != vk::Result::eSuccess) {
        throw std::runtime_error("wait for image in swapchain failed");
    }
    auto imageIndex = resultValue.value;

    auto& cmdMgr = ctx.commandManager;
    auto& cmd = cmdBufs_[curFrame_];
    cmd.reset();

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmd.begin(beginInfo);
    vk::ClearValue clearValue;
    clearValue.setColor(vk::ClearColorValue(std::array<float, 4>{0.1, 0.1, 0.1, 1}));
    vk::RenderPassBeginInfo renderPassBegin;
    renderPassBegin.setRenderPass(ctx.renderProcess->renderPass)
                   .setFramebuffer(swapchain->framebuffers[imageIndex])
                   .setClearValues(clearValue)
                   .setRenderArea(vk::Rect2D({}, swapchain->GetExtent()));
    cmd.beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);
    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->graphicsPipeline);

    vk::DeviceSize offset = 0;
    cmd.bindVertexBuffers(0, verticesBuffer_.buffer, offset);
    cmd.bindIndexBuffer(indicesBuffer_.buffer, 0, vk::IndexType::eUint32);

    cmd.drawIndexed(6, 1, 0, 0, 0);
    cmd.endRenderPass();
    cmd.end();

    vk::SubmitInfo submit;
    vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    submit.setCommandBuffers(cmd)
          .setWaitSemaphores(imageAvaliableSems_[curFrame_])
          .setWaitDstStageMask(flags)
          .setSignalSemaphores(renderFinishSems_[curFrame_]);
    ctx.graphicsQueue.submit(submit, fences_[curFrame_]);

    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphores(renderFinishSems_[curFrame_])
               .setSwapchains(swapchain->swapchain)
               .setImageIndices(imageIndex);
    if (ctx.presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess) {
        throw std::runtime_error("present queue execute failed");
    }

    curFrame_ = (curFrame_ + 1) % maxFlightCount_;
}

void Renderer::createFences() {
    fences_.resize(maxFlightCount_, nullptr);

    for (auto& fence : fences_) {
        vk::FenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
        fence = Context::Instance().device.createFence(fenceCreateInfo);
    }
}

void Renderer::createSemaphores() {
    auto& device = Context::Instance().device;
    vk::SemaphoreCreateInfo info;

    imageAvaliableSems_.resize(maxFlightCount_);
    renderFinishSems_.resize(maxFlightCount_);

    for (auto& sem : imageAvaliableSems_) {
        sem = device.createSemaphore(info);
    }

    for (auto& sem : renderFinishSems_) {
        sem = device.createSemaphore(info);
    }
}

void Renderer::createCmdBuffers() {
    cmdBufs_.resize(maxFlightCount_);

    for (auto& cmd : cmdBufs_) {
        cmd = Context::Instance().commandManager->CreateOneCommandBuffer();
    }
}

void Renderer::createBuffers() {
    auto& device = Context::Instance().device;

    vk::BufferCreateInfo createInfo;
    createInfo.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
              .setSize(sizeof(float) * 8)
              .setSharingMode(vk::SharingMode::eExclusive);

    verticesBuffer_.buffer = device.createBuffer(createInfo);

    auto requirements = device.getBufferMemoryRequirements(verticesBuffer_.buffer);
    verticesBuffer_.size = requirements.size;
    auto index = queryBufferMemTypeIndex(requirements.memoryTypeBits,
                                         vk::MemoryPropertyFlagBits::eHostCoherent|vk::MemoryPropertyFlagBits::eHostVisible);
    vk::MemoryAllocateInfo allocInfo;
    allocInfo.setMemoryTypeIndex(index)
             .setAllocationSize(requirements.size);
    verticesBuffer_.memory = device.allocateMemory(allocInfo);

    device.bindBufferMemory(verticesBuffer_.buffer, verticesBuffer_.memory, 0);

    createInfo.setUsage(vk::BufferUsageFlagBits::eIndexBuffer)
              .setSize(sizeof(std::uint32_t) * 6);
    indicesBuffer_.buffer = device.createBuffer(createInfo);
    requirements = device.getBufferMemoryRequirements(indicesBuffer_.buffer);
    indicesBuffer_.size = requirements.size;
    index = queryBufferMemTypeIndex(requirements.memoryTypeBits,
                                    vk::MemoryPropertyFlagBits::eHostCoherent|vk::MemoryPropertyFlagBits::eHostVisible);
    indicesBuffer_.memory = device.allocateMemory(allocInfo);
    device.bindBufferMemory(indicesBuffer_.buffer, indicesBuffer_.memory, 0);
}

std::uint32_t Renderer::queryBufferMemTypeIndex(std::uint32_t type, vk::MemoryPropertyFlags flag) {
    auto property = Context::Instance().phyDevice.getMemoryProperties();

    for (std::uint32_t i = 0; i < property.memoryTypeCount; i++) {
        if ((1 << i) & type &&
            property.memoryTypes[i].propertyFlags & flag) {
                return i;
        }
    }

    return 0;
}

void Renderer::bufferData() {
    Vertex vertices[] = {
        Vertex{{-0.5, -0.5}},
        Vertex{{0.5, -0.5}},
        Vertex{{0.5, 0.5}},
        Vertex{{-0.5, 0.5}},
    };
    auto& device = Context::Instance().device;
    void* ptr = device.mapMemory(verticesBuffer_.memory, 0, verticesBuffer_.size);
        memcpy(ptr, vertices, sizeof(vertices));
    device.unmapMemory(verticesBuffer_.memory);

    std::uint32_t indices[] = {
        0, 1, 3,
        1, 2, 3,
    };
    ptr = device.mapMemory(indicesBuffer_.memory, 0, indicesBuffer_.size);
        memcpy(ptr, indices, sizeof(indices));
    device.unmapMemory(indicesBuffer_.memory);
}

}