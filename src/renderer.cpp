#include "renderer.hpp"

namespace toy2d {

RawRenderer* Renderer::renderer_ = nullptr;

void Renderer::Init(std::vector<const char*> extensions,
                    SurfaceCreateCallback cb,
                    const Vec2& windowSize,
                    bool debugMode) {
    Context::Init(extensions, cb, windowSize, debugMode);
    renderer_ = new RawRenderer;
}

void Renderer::Quit() {
    delete renderer_;
    Context::GetInstance().GetDevice().WaitIdle();
    Context::Quit();
}

void Renderer::Render() {
    ASSERT(renderer_);
    renderer_->Render();
}

}
