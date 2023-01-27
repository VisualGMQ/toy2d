#include "toy2d/renderer.hpp"
#include "toy2d/math.hpp"
#include "toy2d/context.hpp"

namespace toy2d {

Renderer::Renderer(int maxFlightCount): maxFlightCount_(maxFlightCount), curFrame_(0) {
    createFences();
    createSemaphores();
    createCmdBuffers();
    createBuffers();
    createUniformBuffers(maxFlightCount);
    descriptorSets_ = DescriptorSetManager::Instance().AllocBufferSets(maxFlightCount);
    updateDescriptorSets();
    initMats();
    createWhiteTexture();

    SetDrawColor(Color{0, 0, 0});
}

Renderer::~Renderer() {
    auto& device = Context::Instance().device;
    device.destroySampler(sampler);
    rectVerticesBuffer_.reset();
    rectIndicesBuffer_.reset();
    uniformBuffers_.clear();
    colorBuffers_.clear();
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

void Renderer::StartRender() {
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
    imageIndex_ = resultValue.value;

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
                   .setFramebuffer(swapchain->framebuffers[imageIndex_])
                   .setClearValues(clearValue)
                   .setRenderArea(vk::Rect2D({}, swapchain->GetExtent()));
    cmd.beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);
}

void Renderer::DrawTexture(const Rect& rect, Texture& texture) {
    auto& ctx = Context::Instance();
    auto& device = ctx.device;
    auto& cmd = cmdBufs_[curFrame_];
    vk::DeviceSize offset = 0;

    bufferRectData();

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->graphicsPipelineWithTriangleTopology);
    cmd.bindVertexBuffers(0, rectVerticesBuffer_->buffer, offset);
    cmd.bindIndexBuffer(rectIndicesBuffer_->buffer, 0, vk::IndexType::eUint32);

    auto& layout = Context::Instance().renderProcess->layout;
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                           layout,
                           0, {descriptorSets_[curFrame_].set, texture.set.set}, {});
    auto model = Mat4::CreateTranslate(rect.position).Mul(Mat4::CreateScale(rect.size));
    cmd.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(Mat4), model.GetData());
    cmd.drawIndexed(6, 1, 0, 0, 0);
}

void Renderer::DrawLine(const Vec& p1, const Vec& p2) {
    auto& ctx = Context::Instance();
    auto& device = ctx.device;
    auto& cmd = cmdBufs_[curFrame_];
    vk::DeviceSize offset = 0;

    bufferLineData(p1, p2);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->graphicsPipelineWithLineTopology);
    cmd.bindVertexBuffers(0, lineVerticesBuffer_->buffer, offset);

    auto& layout = Context::Instance().renderProcess->layout;
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                           layout,
                           0, {descriptorSets_[curFrame_].set, whiteTexture->set.set}, {});
    auto model = Mat4::CreateIdentity();
    cmd.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(Mat4), model.GetData());
    cmd.draw(2, 1, 0, 0);
}

void Renderer::EndRender() {
    auto& ctx = Context::Instance();
    auto& swapchain = ctx.swapchain;
    auto& cmd = cmdBufs_[curFrame_];
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
               .setImageIndices(imageIndex_);
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

    rectVerticesBuffer_.reset(new Buffer(vk::BufferUsageFlagBits::eVertexBuffer,
                                     sizeof(Vertex) * 4,
                                     vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent));

    rectIndicesBuffer_.reset(new Buffer(vk::BufferUsageFlagBits::eIndexBuffer,
                                     sizeof(uint32_t) * 6,
                                     vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent));

    lineVerticesBuffer_.reset(new Buffer(vk::BufferUsageFlagBits::eVertexBuffer,
                                         sizeof(Vertex) * 2,
                                         vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent));
}

void Renderer::createUniformBuffers(int flightCount) {
    uniformBuffers_.resize(flightCount);
    //            two mat4                  one color
    size_t size = sizeof(Mat4) * 2;
    for (auto& buffer : uniformBuffers_) {
        buffer.reset(new Buffer(vk::BufferUsageFlagBits::eTransferSrc,
                     size,
                     vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent));
    }
    deviceUniformBuffers_.resize(flightCount);
    for (auto& buffer : deviceUniformBuffers_) {
        buffer.reset(new Buffer(vk::BufferUsageFlagBits::eTransferDst|vk::BufferUsageFlagBits::eUniformBuffer,
                     size,
                     vk::MemoryPropertyFlagBits::eDeviceLocal));
    }

    colorBuffers_.resize(flightCount);
    size = sizeof(Color);
    for (auto& buffer : colorBuffers_) {
        buffer.reset(new Buffer(vk::BufferUsageFlagBits::eTransferSrc,
                     size,
                     vk::MemoryPropertyFlagBits::eHostCoherent|vk::MemoryPropertyFlagBits::eHostVisible));
    }

    deviceColorBuffers_.resize(flightCount);
    for (auto& buffer : deviceColorBuffers_) {
        buffer.reset(new Buffer(vk::BufferUsageFlagBits::eTransferDst|vk::BufferUsageFlagBits::eUniformBuffer,
                     size,
                     vk::MemoryPropertyFlagBits::eDeviceLocal));
    }
}

void Renderer::transformBuffer2Device(Buffer& src, Buffer& dst, size_t srcOffset, size_t dstOffset, size_t size) {
    Context::Instance().commandManager->ExecuteCmd(Context::Instance().graphicsQueue,
            [&](vk::CommandBuffer& cmdBuf) {
                vk::BufferCopy region;
                region.setSrcOffset(srcOffset)
                      .setDstOffset(dstOffset)
                      .setSize(size);
                cmdBuf.copyBuffer(src.buffer, dst.buffer, region);
            });
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

void Renderer::bufferRectData() {
    bufferRectVertexData();
    bufferRectIndicesData();
}

void Renderer::bufferRectVertexData() {
    Vertex vertices[] = {
        {Vec{-0.5, -0.5},Vec{0, 0}},
        {Vec{0.5, -0.5} ,Vec{1, 0}},
        {Vec{0.5, 0.5}  ,Vec{1, 1}},
        {Vec{-0.5, 0.5} ,Vec{0, 1}},
    };
    auto& device = Context::Instance().device;
    memcpy(rectVerticesBuffer_->map, vertices, sizeof(vertices));
}

void Renderer::bufferRectIndicesData() {
    std::uint32_t indices[] = {
        0, 1, 3,
        1, 2, 3,
    };
    auto& device = Context::Instance().device;
    memcpy(rectIndicesBuffer_->map, indices, sizeof(indices));
}

void Renderer::bufferLineData(const Vec& p1, const Vec& p2) {
    Vertex vertices[] = {
        {p1,Vec{0, 0}},
        {p2,Vec{0, 0}},
    };
    auto& device = Context::Instance().device;
    memcpy(lineVerticesBuffer_->map, vertices, sizeof(vertices));
}

void Renderer::bufferMVPData() {
    struct Matrices {
        Mat4 project;
        Mat4 view;
    } matrices;
    auto& device = Context::Instance().device;
    for (int i = 0; i < uniformBuffers_.size(); i++) {
        auto& buffer = uniformBuffers_[i];
        memcpy(buffer->map, (void*)&projectMat_, sizeof(Mat4));
        memcpy(((float*)buffer->map + 4 * 4), (void*)&viewMat_, sizeof(Mat4));
        transformBuffer2Device(*buffer, *deviceUniformBuffers_[i], 0, 0, buffer->size);
    }
}

void Renderer::SetDrawColor(const Color& color) {
    for (int i = 0; i < colorBuffers_.size(); i++) {
        auto& buffer = colorBuffers_[i];
        auto& device = Context::Instance().device;
        memcpy(buffer->map, (void*)&color, sizeof(float) * 3);

        transformBuffer2Device(*buffer, *deviceColorBuffers_[i], 0, 0, buffer->size);
    }

}

void Renderer::initMats() {
    viewMat_ = Mat4::CreateIdentity();
    projectMat_ = Mat4::CreateIdentity();
}

void Renderer::SetProject(int right, int left, int bottom, int top, int far, int near) {
    projectMat_ = Mat4::CreateOrtho(left, right, top, bottom, near, far);
    bufferMVPData();
}

void Renderer::updateDescriptorSets() {
    for (int i = 0; i < descriptorSets_.size(); i++) {
        // bind MVP buffer
        vk::DescriptorBufferInfo bufferInfo1;
        bufferInfo1.setBuffer(deviceUniformBuffers_[i]->buffer)
                   .setOffset(0)
				   .setRange(sizeof(Mat4) * 2);

        std::vector<vk::WriteDescriptorSet> writeInfos(2);
        writeInfos[0].setBufferInfo(bufferInfo1)
                     .setDstBinding(0)
                     .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                     .setDescriptorCount(1)
                     .setDstArrayElement(0)
                     .setDstSet(descriptorSets_[i].set);

        // bind Color buffer
        vk::DescriptorBufferInfo bufferInfo2;
        bufferInfo2.setBuffer(deviceColorBuffers_[i]->buffer)
            .setOffset(0)
            .setRange(sizeof(Color));

        writeInfos[1].setBufferInfo(bufferInfo2)
                     .setDstBinding(1)
                     .setDescriptorCount(1)
                     .setDstArrayElement(0)
                     .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                     .setDstSet(descriptorSets_[i].set);

        Context::Instance().device.updateDescriptorSets(writeInfos, {});
    }
}

void Renderer::createWhiteTexture() {
    unsigned char data[] = {0xFF, 0xFF, 0xFF, 0xFF};
    whiteTexture = TextureManager::Instance().Create((void*)data, 1, 1);
}

}
