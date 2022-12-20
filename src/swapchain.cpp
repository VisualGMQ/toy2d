#include "toy2d/swapchain.hpp"
#include "toy2d/context.hpp"

namespace toy2d {

Swapchain::Swapchain(int w, int h) {
    queryInfo(w, h);

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.setClipped(true)
              .setImageArrayLayers(1)
              .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
              .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
              .setSurface(Context::GetInstance().surface)
              .setImageColorSpace(info.format.colorSpace)
              .setImageFormat(info.format.format)
              .setImageExtent(info.imageExtent)
              .setMinImageCount(info.imageCount)
              .setPreTransform(info.transform)
              .setPresentMode(info.present);

    auto& queueIndicecs = Context::GetInstance().queueFamilyIndices;
    if (queueIndicecs.graphicsQueue.value() == queueIndicecs.presentQueue.value()) {
        createInfo
                  .setImageSharingMode(vk::SharingMode::eExclusive);
    } else {
        std::array indices = {queueIndicecs.graphicsQueue.value(), queueIndicecs.presentQueue.value()};
        createInfo.setQueueFamilyIndices(indices)
                  .setImageSharingMode(vk::SharingMode::eConcurrent);
    }

    swapchain = Context::GetInstance().device.createSwapchainKHR(createInfo);
}

Swapchain::~Swapchain() {
    for (auto& view : imageViews) {
        Context::GetInstance().device.destroyImageView(view);
    }
    Context::GetInstance().device.destroySwapchainKHR(swapchain);
}

void Swapchain::queryInfo(int w, int h) {
    auto& phyDevice = Context::GetInstance().phyDevice;
    auto& surface = Context::GetInstance().surface;
    auto formats = phyDevice.getSurfaceFormatsKHR(surface);
    info.format = formats[0];
    for (const auto& format : formats) {
        if (format.format == vk::Format::eB8G8R8A8Sint &&
            format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            info.format = format;
            break;
        }
    }

    auto capabilities = phyDevice.getSurfaceCapabilitiesKHR(surface);
    info.imageCount = std::clamp<uint32_t>(2, capabilities.minImageCount, capabilities.maxImageCount);

    info.imageExtent.width = std::clamp<uint32_t>(w, capabilities.minImageExtent.width, capabilities.minImageExtent.width);
    info.imageExtent.height = std::clamp<uint32_t>(h, capabilities.minImageExtent.height, capabilities.minImageExtent.height);
    
    info.transform = capabilities.currentTransform;

    auto presents = phyDevice.getSurfacePresentModesKHR(surface);
    info.present = vk::PresentModeKHR::eFifo;
    for (const auto& present : presents) {
        if (present == vk::PresentModeKHR::eMailbox) {
            info.present = present;
            break;
        }
    }
}

void Swapchain::getImages() {
    images = Context::GetInstance().device.getSwapchainImagesKHR(swapchain);
}

void Swapchain::createImageViews() {
    imageViews.resize(images.size());

    for (int i = 0; i < images.size(); i++) {
        vk::ImageViewCreateInfo createInfo;
        vk::ComponentMapping mapping;
        vk::ImageSubresourceRange range;
        range.setBaseMipLevel(0)
             .setLevelCount(1)
             .setBaseArrayLayer(0)
             .setLayerCount(1)
             .setAspectMask(vk::ImageAspectFlagBits::eColor);
        createInfo.setImage(images[i])
                  .setViewType(vk::ImageViewType::e2D)
                  .setComponents(mapping)
                  .setFormat(info.format.format)
                  .setSubresourceRange(range);
        imageViews[i] = Context::GetInstance().device.createImageView(createInfo);
    }
}

}
