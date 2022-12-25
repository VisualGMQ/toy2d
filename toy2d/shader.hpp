#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

class Shader final {
public:
    static void Init(const std::string& vertexSource, const std::string& fragSource);
    static void Quit();

    static Shader& GetInstance() {
        return *instance_;
    }

    vk::ShaderModule vertexModule;
    vk::ShaderModule fragmentModule;

    ~Shader();

    std::vector<vk::PipelineShaderStageCreateInfo> GetStage();

private:
    static std::unique_ptr<Shader> instance_;
    std::vector<vk::PipelineShaderStageCreateInfo> stage_;

    Shader(const std::string& vertexSource, const std::string& fragSource);
    void initStage();
};

}
