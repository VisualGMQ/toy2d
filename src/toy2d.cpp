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
    ctx.initSampler();

    int maxFlightCount = 2;
    DescriptorSetManager::Init(maxFlightCount);
    renderer_ = std::make_unique<Renderer>(maxFlightCount);
    renderer_->SetProject(windowWidth, 0, 0, windowHeight, -1, 1);
}

void Quit() {
    Context::Instance().device.waitIdle();
    renderer_.reset();
    TextureManager::Instance().Clear();
    DescriptorSetManager::Quit();
    Context::Quit();
}

Texture* LoadTexture(const std::string& filename) {
    return TextureManager::Instance().Load(filename);
}

void DestroyTexture(Texture* texture) {
    TextureManager::Instance().Destroy(texture);
}

Renderer* GetRenderer() {
    return renderer_.get();
}

void ResizeSwapchainImage(int w, int h) {
    auto& ctx = Context::Instance();
    ctx.device.waitIdle();

    ctx.swapchain.reset();
    ctx.getSurface();
    ctx.initSwapchain(w, h);
    ctx.swapchain->InitFramebuffers();
}

}
