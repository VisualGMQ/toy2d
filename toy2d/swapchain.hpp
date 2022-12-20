#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

class Swapchain final {
public:
    vk::SwapchainKHR swapchain;

    Swapchain(int w, int h);
    ~Swapchain();

    struct SwapchainInfo {
        vk::Extent2D imageExtent;
        uint32_t imageCount;
        vk::SurfaceFormatKHR format;
        vk::SurfaceTransformFlagBitsKHR transform;
        vk::PresentModeKHR present;
    };

    SwapchainInfo info;

    void queryInfo(int w, int h);
};

}
