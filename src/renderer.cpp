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
    bufferData();
    createDescriptorPool(maxFlightCount);
    allocDescriptorSets(maxFlightCount);
    updateDescriptorSets();
    initMats();

    SetDrawColor(Color{0, 0, 0});
}

Renderer::~Renderer() {
    auto& device = Context::Instance().device;
    device.destroyDescriptorPool(descriptorPool_);
    verticesBuffer_.reset();
    indicesBuffer_.reset();
    uniformBuffers_.clear();
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

void Renderer::DrawRect(const Rect& rect) {
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

    auto model = Mat4::CreateTranslate(rect.position).Mul(Mat4::CreateScale(rect.size));
    bufferMVPData(model);

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
    cmd.bindVertexBuffers(0, verticesBuffer_->buffer, offset);
    cmd.bindIndexBuffer(indicesBuffer_->buffer, 0, vk::IndexType::eUint32);

    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                           Context::Instance().renderProcess->layout,
                           0, descriptorSets_[curFrame_], {});
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

    verticesBuffer_.reset(new Buffer(vk::BufferUsageFlagBits::eVertexBuffer,
                                     sizeof(float) * 8,
                                     vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent));

    indicesBuffer_.reset(new Buffer(vk::BufferUsageFlagBits::eIndexBuffer,
                                     sizeof(float) * 6,
                                     vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent));
}

void Renderer::createUniformBuffers(int flightCount) {
    uniformBuffers_.resize(flightCount);

    for (auto& buffer : uniformBuffers_) {
        buffer.reset(new Buffer(vk::BufferUsageFlagBits::eUniformBuffer,
                     // three mat4               one color
                     sizeof(float) * 4 * 4 * 3 + sizeof(float) * 3,
                     vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent));
    }
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
    bufferVertexData();
    bufferIndicesData();
}

void Renderer::bufferVertexData() {
    Vec vertices[] = {
        Vec{{-0.5, -0.5}},
        Vec{{0.5, -0.5}},
        Vec{{0.5, 0.5}},
        Vec{{-0.5, 0.5}},
    };
    auto& device = Context::Instance().device;
    void* ptr = device.mapMemory(verticesBuffer_->memory, 0, verticesBuffer_->size);
        memcpy(ptr, vertices, sizeof(vertices));
    device.unmapMemory(verticesBuffer_->memory);


}

void Renderer::bufferIndicesData() {
    std::uint32_t indices[] = {
        0, 1, 3,
        1, 2, 3,
    };
    auto& device = Context::Instance().device;
    void* ptr = device.mapMemory(indicesBuffer_->memory, 0, indicesBuffer_->size);
        memcpy(ptr, indices, sizeof(indices));
    device.unmapMemory(indicesBuffer_->memory);
}

void Renderer::bufferMVPData(const Mat4& model) {
    MVP mvp;
    mvp.project = projectMat_;
    mvp.view = viewMat_;
    mvp.model = model;
    auto& device = Context::Instance().device;
    auto& uniformBuffer = uniformBuffers_[curFrame_];
    void* ptr = device.mapMemory(uniformBuffer->memory, 0, uniformBuffer->size);
        memcpy(ptr, (void*)&mvp, sizeof(mvp));
    device.unmapMemory(uniformBuffer->memory);
}

void Renderer::SetDrawColor(const Color& color) {
    auto& uniformBuffer = uniformBuffers_[curFrame_];
    auto& device = Context::Instance().device;
    void* ptr = device.mapMemory(uniformBuffer->memory, 0, uniformBuffer->size);
        memcpy(((unsigned char*)ptr) + sizeof(float) * 4 * 4 * 3, (void*)&color, sizeof(color));
    device.unmapMemory(uniformBuffer->memory);
}

void Renderer::initMats() {
    viewMat_ = Mat4::CreateIdentity();
    projectMat_ = Mat4::CreateIdentity();
}

void Renderer::SetProject(int right, int left, int bottom, int top, int far, int near) {
    projectMat_ = Mat4::CreateOrtho(left, right, top, bottom, near, far);
}

void Renderer::createDescriptorPool(int flightCount) {
    vk::DescriptorPoolCreateInfo createInfo;
    vk::DescriptorPoolSize size;
    size.setDescriptorCount(flightCount * 2)
        .setType(vk::DescriptorType::eUniformBuffer);
    createInfo.setPoolSizes(size)
			  .setMaxSets(flightCount * 2);
    descriptorPool_ = Context::Instance().device.createDescriptorPool(createInfo);
}

std::vector<vk::DescriptorSet> Renderer::allocDescriptorSet(int flightCount) {
    std::vector layouts(flightCount, Context::Instance().shader->GetDescriptorSetLayouts()[0]);
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.setDescriptorPool(descriptorPool_)
			 .setSetLayouts(layouts);
    return Context::Instance().device.allocateDescriptorSets(allocInfo);
}

void Renderer::allocDescriptorSets(int flightCount) {
    descriptorSets_ = allocDescriptorSet(flightCount);
}

void Renderer::updateDescriptorSets() {
    for (int i = 0; i < descriptorSets_.size(); i++) {
        // bind MVP buffer
        vk::DescriptorBufferInfo bufferInfo1;
        bufferInfo1.setBuffer(uniformBuffers_[i]->buffer)
                   .setOffset(0)
				   .setRange(sizeof(float) * 4 * 4 * 3);

        std::vector<vk::WriteDescriptorSet> writeInfos(2);
        writeInfos[0].setBufferInfo(bufferInfo1)
                     .setDstBinding(0)
                     .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                     .setDescriptorCount(1)
                     .setDstArrayElement(0)
                     .setDstSet(descriptorSets_[i]);

        // bind Color buffer
        vk::DescriptorBufferInfo bufferInfo2;
        bufferInfo2.setBuffer(uniformBuffers_[i]->buffer)
            .setOffset(sizeof(float) * 4 * 4 * 3)
            .setRange(sizeof(float) * 3);

        writeInfos[1].setBufferInfo(bufferInfo2)
                     .setDstBinding(1)
                     .setDstArrayElement(0)
                     .setDescriptorCount(1)
                     .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                     .setDstSet(descriptorSets_[i]);

        Context::Instance().device.updateDescriptorSets(writeInfos, {});
    }
}

}