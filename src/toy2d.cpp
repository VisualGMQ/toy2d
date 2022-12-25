#include "toy2d/toy2d.hpp"

namespace toy2d {

void Init(const std::vector<const char*>& extensions, CreateSurfaceFunc func, int w, int h) {
    Context::Init(extensions, func);
    Context::GetInstance().InitSwapchain(w, h);
    Shader::Init(ReadWholeFile("./vert.spv"), ReadWholeFile("./frag.spv"));
    Context::GetInstance().renderProcess->InitPipeline(w, h);
}

void Quit() {
    Context::GetInstance().renderProcess->DestroyPipeline();
    Context::GetInstance().DestroySwapchain();
    Shader::Quit();
    Context::Quit();
}


std::vector<vk::PipelineShaderStageCreateInfo> Shader::GetStage() {
    return stage_;
}

}
