#include "toy2d/renderer.hpp"

namespace toy2d {

Renderer::Renderer() {
    createFence();
    createSemaphores();
    cmdBuf_ = Context::Instance().commandManager->CreateOneCommandBuffer();
}

Renderer::~Renderer() {
    auto& device = Context::Instance().device;
    device.destroySemaphore(imageAvaliableSem_);
    device.destroySemaphore(renderFinishSem_);
    device.destroyFence(fence_);
}

void Renderer::DrawTriangle() {
    auto& ctx = Context::Instance();
    auto& device = ctx.device;
    if (device.waitForFences(fence_, true, std::numeric_limits<std::uint64_t>::max()) != vk::Result::eSuccess) {
        throw std::runtime_error("wait for fence failed");
    }
    device.resetFences(fence_);

    auto& swapchain = ctx.swapchain;
    auto resultValue = device.acquireNextImageKHR(swapchain->swapchain, std::numeric_limits<std::uint64_t>::max(), imageAvaliableSem_, nullptr);
    if (resultValue.result != vk::Result::eSuccess) {
        throw std::runtime_error("wait for image in swapchain failed");
    }
    auto imageIndex = resultValue.value;

    auto& cmdMgr = ctx.commandManager;
    cmdBuf_.reset();

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmdBuf_.begin(beginInfo);
    vk::ClearValue clearValue;
    clearValue.setColor(vk::ClearColorValue(std::array<float, 4>{0.1, 0.1, 0.1, 1}));
    vk::RenderPassBeginInfo renderPassBegin;
    renderPassBegin.setRenderPass(ctx.renderProcess->renderPass)
                   .setFramebuffer(swapchain->framebuffers[imageIndex])
                   .setClearValues(clearValue)
                   .setRenderArea(vk::Rect2D({}, swapchain->GetExtent()));
    cmdBuf_.beginRenderPass(&renderPassBegin, vk::SubpassContents::eInline);
    cmdBuf_.bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->graphicsPipeline);
    cmdBuf_.draw(3, 1, 0, 0);
    cmdBuf_.endRenderPass();
    cmdBuf_.end();

    vk::SubmitInfo submit;
    vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    submit.setCommandBuffers(cmdBuf_)
          .setWaitSemaphores(imageAvaliableSem_)
          .setWaitDstStageMask(flags)
          .setSignalSemaphores(renderFinishSem_);
    ctx.graphicsQueue.submit(submit, fence_);

    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphores(renderFinishSem_)
               .setSwapchains(swapchain->swapchain)
               .setImageIndices(imageIndex);
    if (ctx.presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess) {
        throw std::runtime_error("present queue execute failed");
    }
}

void Renderer::createFence() {
    vk::FenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
    fence_ = Context::Instance().device.createFence(fenceCreateInfo);
}

void Renderer::createSemaphores() {
    auto& device = Context::Instance().device;
    vk::SemaphoreCreateInfo info;
    imageAvaliableSem_ = device.createSemaphore(info);
    renderFinishSem_ = device.createSemaphore(info);
}

}
