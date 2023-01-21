#include "toy2d/toy2d.hpp"

namespace toy2d {

std::unique_ptr<Renderer> renderer_;

void Init(std::vector<const char*>& extensions, Context::GetSurfaceCallback cb, int windowWidth, int windowHeight) {
    Context::Init(extensions, cb);
    auto& ctx = Context::Instance();
    ctx.initSwapchain(windowWidth, windowHeight);
    ctx.initShaderModules();
    ctx.initRenderProcess();
    ctx.initGraphicsPipeline();
    ctx.swapchain->InitFramebuffers();
    ctx.initCommandPool();

    int maxFlightCount = 2;
    DescriptorSetManager::Init(maxFlightCount);
    renderer_ = std::make_unique<Renderer>(maxFlightCount);
    renderer_->SetProject(windowWidth, 0, 0, windowHeight, -1, 1);
}

void Quit() {
    Context::Instance().device.waitIdle();
    renderer_.reset();
    DescriptorSetManager::Quit();
    Context::Quit();
}

Renderer* GetRenderer() {
    return renderer_.get();
}

}
