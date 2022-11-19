#include "toy2d/swapchain.hpp"
#include "toy2d/context.hpp"

namespace toy2d {

Swapchain::Swapchain(vk::SurfaceKHR surface, int windowWidth, int windowHeight): surface(surface) {
    querySurfaceInfo(windowWidth, windowHeight);
    swapchain = createSwapchain();
    createImageAndViews();
}

Swapchain::~Swapchain() {
    auto& ctx = Context::Instance();
    for (auto& img : images) {
        ctx.device.destroyImageView(img.view);
    }
    for (auto& framebuffer : framebuffers) {
        Context::Instance().device.destroyFramebuffer(framebuffer);
    }
    ctx.device.destroySwapchainKHR(swapchain);
    ctx.instance.destroySurfaceKHR(surface);
}

void Swapchain::InitFramebuffers() {
    createFramebuffers();
}

void Swapchain::querySurfaceInfo(int windowWidth, int windowHeight) {
    surfaceInfo_.format = querySurfaceeFormat();

    auto capability = Context::Instance().phyDevice.getSurfaceCapabilitiesKHR(surface);
    surfaceInfo_.count = std::clamp(capability.minImageCount + 1,
                                    capability.minImageCount, capability.maxImageCount);
    surfaceInfo_.transform = capability.currentTransform;
    surfaceInfo_.extent = querySurfaceExtent(capability, windowWidth, windowHeight);
}

vk::SurfaceFormatKHR Swapchain::querySurfaceeFormat() {
    auto formats = Context::Instance().phyDevice.getSurfaceFormatsKHR(surface);
    for (auto& format : formats) {
        if (format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return format;
        }
    }
    return formats[0];
}

vk::Extent2D Swapchain::querySurfaceExtent(const vk::SurfaceCapabilitiesKHR& capability, int windowWidth, int windowHeight) {
    if (capability.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capability.currentExtent;
    } else {
        auto extent = vk::Extent2D{
            static_cast<uint32_t>(windowWidth),
            static_cast<uint32_t>(windowHeight)
        };

        extent.width = std::clamp(extent.width, capability.minImageExtent.width, capability.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capability.minImageExtent.height, capability.maxImageExtent.height);
        return extent;
    }
}

vk::SwapchainKHR Swapchain::createSwapchain() {
    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.setClipped(true)
              .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
              .setImageExtent(surfaceInfo_.extent)
              .setImageColorSpace(surfaceInfo_.format.colorSpace)
              .setImageFormat(surfaceInfo_.format.format)
              .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
              .setMinImageCount(surfaceInfo_.count)
              .setImageArrayLayers(1)
              .setPresentMode(vk::PresentModeKHR::eFifo)
              .setPreTransform(surfaceInfo_.transform)
              .setSurface(surface);

    auto& ctx = Context::Instance();
    if (ctx.queueInfo.graphicsIndex.value() == ctx.queueInfo.presentIndex.value()) {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    } else {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        std::array queueIndices = {ctx.queueInfo.graphicsIndex.value(), ctx.queueInfo.presentIndex.value()};
        createInfo.setQueueFamilyIndices(queueIndices);
    }

    return ctx.device.createSwapchainKHR(createInfo);
}

void Swapchain::createImageAndViews() {
    auto& ctx = Context::Instance();
    auto images = ctx.device.getSwapchainImagesKHR(swapchain);
    for (auto& image : images) {
        Image img;
        img.image = image;
        vk::ImageViewCreateInfo viewCreateInfo;
        vk::ImageSubresourceRange range;
        range.setBaseArrayLayer(0)
             .setBaseMipLevel(0)
             .setLayerCount(1)
             .setLevelCount(1)
             .setAspectMask(vk::ImageAspectFlagBits::eColor);
        viewCreateInfo.setImage(image)
                      .setFormat(surfaceInfo_.format.format)
                      .setViewType(vk::ImageViewType::e2D)
                      .setSubresourceRange(range)
                      .setComponents(vk::ComponentMapping{});
        img.view = ctx.device.createImageView(viewCreateInfo);
        this->images.push_back(img);
    }
}

void Swapchain::createFramebuffers() {
    for (auto& img : images) {
        auto& view = img.view;

        vk::FramebufferCreateInfo createInfo;
        createInfo.setAttachments(view)
                  .setLayers(1)
                  .setHeight(GetExtent().height)
                  .setWidth(GetExtent().width)
                  .setRenderPass(Context::Instance().renderProcess->renderPass);

        framebuffers.push_back(Context::Instance().device.createFramebuffer(createInfo));
    }
}


}
