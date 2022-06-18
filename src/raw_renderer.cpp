#include "raw_renderer.hpp"

namespace toy2d {

static vk::PipelineLayout layout_;

RawRenderer::RawRenderer() {
    vertices_[0] = Vertex{{-0.5, -0.5}, {1, 0, 0}};
    vertices_[1] = Vertex{{ 0.5, -0.5}, {0, 1, 0}};
    vertices_[2] = Vertex{{ 0.5,  0.5}, {0, 0, 1}};
    vertices_[3] = Vertex{{-0.5,  0.5}, {0, 0, 1}};

    indices_ = std::array<uint16_t, 6> {
        0, 1, 2, 0, 2, 3
    };

    auto& device = Context::GetInstance().GetDevice();

    cmdPool_ = device.CreateCmdPool();
    ASSERT(cmdPool_);

    setLayout_ = device.CreateDescriptorSetLayout(Uniform::GetBinding());
    ASSERT(setLayout_);

    layout_ = device.CreateLayout(setLayout_);
    ASSERT(layout_);

    auto windowSize = Context::GetInstance().GetWindowSize();
    ubo_.project = CreateOrthoMat(0, windowSize.x,
                                  windowSize.y, 0,
                                  1.0, -1.0);
    ubo_.view = CreateEyeMat();
    ubo_.model = CreateSRT(Vec2{200, 150}, 0, Vec2{200, 200});

    uniformBuffer_ = device.CreateBuffer(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
                                         sizeof(ubo_),
                                         vk::MemoryPropertyFlagBits::eDeviceLocal);
    ASSERT(uniformBuffer_.buffer);
    ASSERT(uniformBuffer_.memory);

    copyData2Device(&ubo_,
                    sizeof(ubo_),
                    uniformBuffer_);

    descriptorPool_ = device.CreateDescriptorPool();
    ASSERT(descriptorPool_);
    set_ = device.AllocateDescriptorSet(descriptorPool_, setLayout_);
    ASSERT(set_);

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.setOffset(0)
              .setRange(uniformBuffer_.size)
              .setBuffer(uniformBuffer_.buffer);
    vk::WriteDescriptorSet writeSet;
    writeSet.setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDstSet(set_)
            .setDstArrayElement(0)
            .setDstBinding(0)
            .setBufferInfo(bufferInfo);

    device.UpdateDescriptorSet(writeSet);

    renderPass_ = device.CreateRenderPass();
    ASSERT(renderPass_);

    vertexShader_ = device.CreateShaderModule("vert.spv");
    fragShader_ = device.CreateShaderModule("frag.spv");
    pipeline_ = device.CreatePipeline(vertexShader_, fragShader_, Context::GetInstance().GetWindowSize(), renderPass_, layout_);

    size_t inFlightNum = device.GetImageViews().size();
    imageAvaliableSems_.resize(inFlightNum);
    drawFinishSems_.resize(inFlightNum);
    fences_.resize(inFlightNum);

    for (auto& sem : imageAvaliableSems_) {
        sem = device.CreateSemaphore();
    }
    for (auto& sem : drawFinishSems_) {
        sem = device.CreateSemaphore();
    }
    for (auto& fence : fences_) {
        fence = device.CreateFence(false);
    }

    vertexBuffer_ = device.CreateBuffer(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                                        sizeof(vertices_),
                                        vk::MemoryPropertyFlagBits::eDeviceLocal);
    ASSERT(vertexBuffer_.buffer);
    ASSERT(vertexBuffer_.memory);
    indexBuffer_ = device.CreateBuffer(vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                                       sizeof(indices_),
                                       vk::MemoryPropertyFlagBits::eDeviceLocal);
    ASSERT(indexBuffer_.buffer);
    ASSERT(indexBuffer_.memory);

    copyData2Device(vertices_.data(), sizeof(vertices_), vertexBuffer_);
    copyData2Device(indices_.data(), sizeof(indices_), indexBuffer_);

    drawCmdBufs_ = device.AllocateCmdBuffers(cmdPool_, inFlightNum);
    for (auto& buf : drawCmdBufs_) {
        ASSERT(buf);
    }

    auto& imageViews = device.GetImageViews();
    framebuffers_.resize(imageViews.size());
    for (size_t i = 0; i < imageViews.size(); i++) {
        framebuffers_[i] = device.CreateFramebuffer(renderPass_, imageViews[i]);
        ASSERT(framebuffers_[i]);
    }
}

void RawRenderer::copyBuffer2Device(Buffer src, Buffer dst, vk::CommandBuffer cmd) {
    ASSERT(src.size == dst.size);
    auto& device = Context::GetInstance().GetDevice();

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    cmd.begin(beginInfo);
        vk::BufferCopy region(0, 0, src.size);
        cmd.copyBuffer(src.buffer, dst.buffer, region);
    cmd.end();

    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(cmd);
    device.GetGraphicsQueue().submit(submitInfo);
    device.WaitIdle();
}

RawRenderer::~RawRenderer() {
    auto& device = Context::GetInstance().GetDevice();

    device.FreeDescriptorSet(descriptorPool_, set_);
    device.DestroyDescriptorPool(descriptorPool_);
    device.DestroyDescriptorSetLayout(setLayout_);
    for (auto& fbo : framebuffers_) {
        device.DestroyFramebuffer(fbo);
    }

    device.DestroyBuffer(uniformBuffer_);
    device.DestroyBuffer(vertexBuffer_);
    device.DestroyBuffer(indexBuffer_);
    device.FreeCmdBuffers(cmdPool_, drawCmdBufs_);
    device.DestroyCmdPool(cmdPool_);
    for (auto& sem : imageAvaliableSems_) {
        device.DestroySemaphore(sem);
    }
    for (auto& sem : drawFinishSems_) {
        device.DestroySemaphore(sem);
    }
    for (auto& fence : fences_) {
        device.DestroyFence(fence);
    }
    device.DestroyShaderModule(vertexShader_);
    device.DestroyShaderModule(fragShader_);
    device.DestroyRenderPass(renderPass_);
    device.DestroyLayout(layout_);
    device.DestroyPipeline(pipeline_);
}

void RawRenderer::Render() {
    auto& device = Context::GetInstance().GetDevice();

    auto fence = fences_[curInFlightIndex_];
    device.ResetFence(fence);

    unsigned int imageIndex = device.AcquireNextImage(imageAvaliableSems_[curInFlightIndex_]);

    auto drawCmd = drawCmdBufs_[curInFlightIndex_];
    drawCmd.reset();

    recordCmd(drawCmd, framebuffers_[imageIndex]);

    auto drawFinishSem = drawFinishSems_[imageIndex];
    auto imageAvaliableSem = imageAvaliableSems_[imageIndex];
    vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(drawCmd)
              .setSignalSemaphores(drawFinishSem)
              .setWaitSemaphores(imageAvaliableSem)
              .setWaitDstStageMask(flags);
    device.GetGraphicsQueue().submit(submitInfo, fence);

    vk::PresentInfoKHR presentInfo;
    auto swapchain = device.GetSwapchain();
    presentInfo.setImageIndices(imageIndex)
               .setSwapchains(swapchain)
               .setWaitSemaphores(drawFinishSem);
    if (device.GetPresentQueue().presentKHR(presentInfo) != vk::Result::eSuccess) {
        throw std::runtime_error("present failed");
    }

    device.WaitForFence(fence);

    curInFlightIndex_ = (curInFlightIndex_ + 1) % device.GetImageViews().size();
}

void RawRenderer::recordCmd(vk::CommandBuffer cmd, vk::Framebuffer framebuffer) {
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    if (cmd.begin(&beginInfo) != vk::Result::eSuccess) {
        throw std::runtime_error("command buffer record failed");
    }

    vk::RenderPassBeginInfo renderPassBegin;
    vk::ClearColorValue cvalue(std::array<float, 4>{0.1, 0.1, 0.1, 1});
    vk::ClearValue value(cvalue);
    renderPassBegin.setRenderPass(renderPass_)
                   .setRenderArea(vk::Rect2D({0, 0}, Context::GetInstance().GetDevice().GetSwapchainExtent()))
                   .setClearValues(value)
                   .setFramebuffer(framebuffer);

    static float c = 0.5;
    cmd.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_);

    vk::DeviceSize size = 0;
    cmd.bindVertexBuffers(0, vertexBuffer_.buffer, size);
    cmd.bindIndexBuffer(indexBuffer_.buffer, 0, vk::IndexType::eUint16);
    cmd.pushConstants(layout_, vk::ShaderStageFlagBits::eVertex, 0, sizeof(c), &c);
    uint32_t offset = 0;
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                           layout_, 0, set_, {});

    cmd.drawIndexed(indices_.size(), 1, 0, 0, 0);

    cmd.endRenderPass();

    cmd.end();
}

}
