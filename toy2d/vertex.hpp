#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

struct Vertex {
    float position[2];

    static vk::VertexInputAttributeDescription GetAttributeDescription();
    static vk::VertexInputBindingDescription GetBindingDescription();
};

}