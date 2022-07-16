#pragma once

#include "pch.hpp"

namespace toy2d {

struct Vec2 {
    float x, y;
};

struct Color {
    float r, g, b;
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

class Mat44 final {
public:
    Mat44() { memset(data_, 0, sizeof(data_)); }
    Mat44(const std::initializer_list<float>& elems) {
        auto it = elems.begin();
        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                Set(x, y, *(it + (y * 4 + x)));
            }
        }
    }
    float Get(int x, int y) { return data_[x * 4 + y]; }
    void Set(int x, int y, float value) { data_[x * 4 + y] = value; }
    float* Data() { return data_; }

private:
    float data_[4 * 4];
};

inline Mat44 CreateEyeMat(float value = 1.0f) {
    return Mat44{
        value,     0,     0,     0,
            0, value,     0,     0,
            0,     0, value,     0,
            0,     0,     0, value,
    };
}

inline Mat44 CreateOrthoMat(float left, float right, float top, float bottom, float near, float far) {
    return Mat44{
        2 / (right - left),                  0,                0, -(right + left) / (right - left),
                         0, 2 / (top - bottom),                0, -(bottom + top) / (top - bottom),
                         0,                  0, 2 / (near - far),   -(near + far) / (near - far),
                         0,                  0,                0,                      1,
    };
}

inline Mat44 CreateSRT(const Vec2& translate, float rotation, const Vec2& scale) {
    // TODO add rotate mat
    return Mat44{
        scale.x,       0, 0, translate.x,
              0, scale.y, 0, translate.y,
              0,       0, 1,           0,
              0,       0, 0,           1,
    };
}

}
