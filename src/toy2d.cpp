#include "toy2d/toy2d.hpp"

namespace toy2d {

void Init(const std::vector<const char*>& extensions, CreateSurfaceFunc func, int w, int h) {
    Context::Init(extensions, func);
    Context::GetInstance().InitSwapchain(w, h);
}

void Quit() {
    Context::GetInstance().DestroySwapchain();
    Context::Quit();
}

}
