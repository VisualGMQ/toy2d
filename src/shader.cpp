#include "toy2d/shader.hpp"
#include "toy2d/context.hpp"

namespace toy2d {

Shader::Shader(const std::vector<char>& vertexSource, const std::vector<char>& fragSource) {
    vk::ShaderModuleCreateInfo vertexModuleCreateInfo, fragModuleCreateInfo;
    vertexModuleCreateInfo.codeSize = vertexSource.size();
    vertexModuleCreateInfo.pCode = (std::uint32_t*)vertexSource.data();
    fragModuleCreateInfo.codeSize = fragSource.size();
    fragModuleCreateInfo.pCode = (std::uint32_t*)fragSource.data();

    auto& device = Context::Instance().device;
    vertexModule_ = device.createShaderModule(vertexModuleCreateInfo);
    fragModule_ = device.createShaderModule(fragModuleCreateInfo);

    initDescriptorSetLayouts();
}

void Shader::initDescriptorSetLayouts() {
    vk::DescriptorSetLayoutCreateInfo createInfo;
    vk::DescriptorSetLayoutBinding binding;
    binding.setBinding(0)
           .setDescriptorCount(1)
           .setDescriptorType(vk::DescriptorType::eUniformBuffer)
           .setStageFlags(vk::ShaderStageFlagBits::eVertex);
    createInfo.setBindings(binding);

    layouts_.push_back(Context::Instance().device.createDescriptorSetLayout(createInfo));
}

Shader::~Shader() {
    auto& device = Context::Instance().device;
    for (auto& layout : layouts_) {
        device.destroyDescriptorSetLayout(layout);
    }
    layouts_.clear();
    device.destroyShaderModule(vertexModule_);
    device.destroyShaderModule(fragModule_);
}

}