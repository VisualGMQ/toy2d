#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

class Swapchain final {
public:
    struct Image {
        vk::Image image;
        vk::ImageView view;
    };

    vk::SurfaceKHR surface = nullptr;
    vk::SwapchainKHR swapchain = nullptr;
    std::vector<Image> images;
    std::vector<vk::Framebuffer> framebuffers;

    const auto& GetExtent() const { return surfaceInfo_.extent; }
    const auto& GetFormat() const { return surfaceInfo_.format; }

    Swapchain(vk::SurfaceKHR, int windowWidth, int windowHeight);
    ~Swapchain();

    void InitFramebuffers();

private:
    struct SurfaceInfo {
        vk::SurfaceFormatKHR format;
        vk::Extent2D extent;
        std::uint32_t count;
        vk::SurfaceTransformFlagBitsKHR transform;
    } surfaceInfo_;

    vk::SwapchainKHR createSwapchain();

    void querySurfaceInfo(int windowWidth, int windowHeight);
    vk::SurfaceFormatKHR querySurfaceeFormat();
    vk::Extent2D querySurfaceExtent(const vk::SurfaceCapabilitiesKHR& capability, int windowWidth, int windowHeight);
    void createImageAndViews();
    void createFramebuffers();
};

}
