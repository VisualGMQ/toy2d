#include "toy2d/toy2d.hpp"
#include <iostream>

#include "vulkan/vulkan.hpp"

namespace toy2d {

void Init(const std::vector<const char*>& extensions) {
    Context::Init(extensions);
}

void Quit() {
    Context::Quit();
}

}