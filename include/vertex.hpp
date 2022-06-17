#pragma once

#include "pch.hpp"

namespace toy2d {

struct Vec2 {
    float x, y;
};

struct Color {
    float r, g, b, a;
};

struct Vertex {
    Vec2 position;
    Color color;

    static vk::VertexInputBindingDescription GetBindingDescription() {
        static vk::VertexInputBindingDescription description;
        description.setBinding(0)
                   .setInputRate(vk::VertexInputRate::eVertex)
                   .setStride(sizeof(Vertex));
        return description;
    }

    static auto GetAttrDescription() {
        static std::array<vk::VertexInputAttributeDescription, 2> desc;
        desc[0].setBinding(0)
               .setLocation(0)
               .setFormat(vk::Format::eR32G32Sfloat)
               .setOffset(offsetof(Vertex, position));
        desc[1].setBinding(0)
               .setLocation(1)
               .setFormat(vk::Format::eR32G32B32Sfloat)
               .setOffset(offsetof(Vertex, color));
        return desc;
    }
};

}
