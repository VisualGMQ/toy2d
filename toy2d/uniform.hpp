#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

struct Color final {
    float r, g, b;
};

struct Uniform final {
    Color color;

    static vk::DescriptorSetLayoutBinding GetBinding() {
        vk::DescriptorSetLayoutBinding binding;
        binding.setBinding(0)
               .setDescriptorType(vk::DescriptorType::eUniformBuffer)
               .setStageFlags(vk::ShaderStageFlagBits::eFragment)
               .setDescriptorCount(1);
        return binding;
    }
};

}
