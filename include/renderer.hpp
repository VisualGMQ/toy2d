#pragma once

#include "context.hpp"
#include "raw_renderer.hpp"

namespace toy2d {

class Renderer final {
public:
    static void Init(std::vector<const char*> extensions,
                     SurfaceCreateCallback,
                     const Vec2& windowSize,
                     bool debugMode = true);
    static void Quit();

    static void Render();

private:
    static RawRenderer* renderer_;
};

}
