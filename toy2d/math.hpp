#pragma once

#include "vulkan/vulkan.hpp"
#include <initializer_list>

namespace toy2d {

struct Vec {
    union {
        struct { float x, y; };
        struct { float w, h; };
    };

    static vk::VertexInputAttributeDescription GetAttributeDescription();
    static vk::VertexInputBindingDescription GetBindingDescription();
};

struct Color {
    float r, g, b;
};

using Size = Vec;

class Mat4 {
public:
    static Mat4 CreateIdentity();
    static Mat4 CreateOnes();
    static Mat4 CreateOrtho(int left, int right, int top, int bottom, int near, int far);
    static Mat4 CreateTranslate(const Vec&);
    static Mat4 CreateScale(const Vec&);
    static Mat4 Create(const std::initializer_list<float>&);

    Mat4();
    const float* GetData() const { return data_; }
    void Set(int x, int y, float value) {
        data_[x * 4 + y] = value;
    }
    float Get(int x, int y) const {
        return data_[x * 4 + y];
    }

    Mat4 Mul(const Mat4& m) const;

private:
    float data_[4 * 4];
};

struct Rect {
    Vec position;
    Size size;
};

}
