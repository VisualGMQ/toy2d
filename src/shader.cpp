#include "toy2d/shader.hpp"
#include "toy2d/context.hpp"
#include "toy2d/math.hpp"

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
    std::vector<vk::DescriptorSetLayoutBinding> bindings(1);
    bindings[0].setBinding(0)
               .setDescriptorCount(1)
               .setDescriptorType(vk::DescriptorType::eUniformBuffer)
               .setStageFlags(vk::ShaderStageFlagBits::eVertex);
    createInfo.setBindings(bindings);

    layouts_.push_back(Context::Instance().device.createDescriptorSetLayout(createInfo));

    bindings.resize(1);
    bindings[0].setBinding(0)
               .setDescriptorCount(1)
               .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
               .setStageFlags(vk::ShaderStageFlagBits::eFragment);
    createInfo.setBindings(bindings);

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

std::vector<vk::PushConstantRange> Shader::GetPushConstantRange() const {
    std::vector<vk::PushConstantRange> ranges(2);
    ranges[0].setOffset(0)
             .setSize(sizeof(Mat4))
             .setStageFlags(vk::ShaderStageFlagBits::eVertex);
    ranges[1].setOffset(sizeof(Mat4))
             .setSize(sizeof(Color))
             .setStageFlags(vk::ShaderStageFlagBits::eFragment);
    return ranges;
}

}
