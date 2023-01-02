#include "toy2d/texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "toy2d/stb_image.h"

namespace toy2d {

Texture::Texture(std::string_view filename) {
    int w, h, channel;
    stbi_uc* pixels = stbi_load(filename.data(), &w, &h, &channel, STBI_rgb_alpha);
    size_t size = w * h * 4;

    if (!pixels) {
        throw std::runtime_error("image load failed");
    }

    std::unique_ptr<Buffer> buffer(new Buffer(vk::BufferUsageFlagBits::eTransferSrc,
                                   size,
                                   vk::MemoryPropertyFlagBits::eHostCoherent|vk::MemoryPropertyFlagBits::eHostVisible));
    memcpy(buffer->map, pixels, size);

    createImage(w, h);
    allocMemory();
    Context::Instance().device.bindImageMemory(image, memory, 0);

    transitionImageLayoutFromUndefine2Dst();
    transformData2Image(*buffer, w, h);
    transitionImageLayoutFromDst2Optimal();

    createImageView();

    stbi_image_free(pixels);
}

Texture::~Texture() {
    auto& device = Context::Instance().device;
    device.destroyImageView(view);
    device.freeMemory(memory);
    device.destroyImage(image);
}

void Texture::createImage(uint32_t w, uint32_t h) {
    vk::ImageCreateInfo createInfo;
    createInfo.setImageType(vk::ImageType::e2D)
              .setArrayLayers(1)
              .setMipLevels(1)
              .setExtent({w, h, 1})
              .setFormat(vk::Format::eR8G8B8A8Srgb)
              .setTiling(vk::ImageTiling::eOptimal)
              .setInitialLayout(vk::ImageLayout::eUndefined)
              .setUsage(vk::ImageUsageFlagBits::eTransferDst|vk::ImageUsageFlagBits::eSampled)
              .setSamples(vk::SampleCountFlagBits::e1);
    image = Context::Instance().device.createImage(createInfo);
}

void Texture::allocMemory() {
    auto& device = Context::Instance().device;
    vk::MemoryAllocateInfo allocInfo;

    auto requirements = device.getImageMemoryRequirements(image);
    allocInfo.setAllocationSize(requirements.size);

    auto index = QueryBufferMemTypeIndex(requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
    allocInfo.setMemoryTypeIndex(index);

    memory = device.allocateMemory(allocInfo);
}

void Texture::transformData2Image(Buffer& buffer, uint32_t w, uint32_t h) {
    Context::Instance().commandManager->ExecuteCmd(Context::Instance().graphicsQueue,
        [&](vk::CommandBuffer cmdBuf){
            vk::BufferImageCopy region;
            vk::ImageSubresourceLayers subsource;
            subsource.setAspectMask(vk::ImageAspectFlagBits::eColor)
                     .setBaseArrayLayer(0)
                     .setMipLevel(0)
                     .setLayerCount(1);
            region.setBufferImageHeight(0)
                  .setBufferOffset(0)
                  .setImageOffset(0)
                  .setImageExtent({w, h, 1})
                  .setBufferRowLength(0)
                  .setImageSubresource(subsource);
            cmdBuf.copyBufferToImage(buffer.buffer, image,
                                     vk::ImageLayout::eTransferDstOptimal,
                                     region);
        });
}

void Texture::transitionImageLayoutFromUndefine2Dst() {
    Context::Instance().commandManager->ExecuteCmd(Context::Instance().graphicsQueue,
        [&](vk::CommandBuffer cmdBuf){
            vk::ImageMemoryBarrier barrier;
            vk::ImageSubresourceRange range;
            range.setLayerCount(1)
                 .setBaseArrayLayer(0)
                 .setLevelCount(1)
                 .setBaseMipLevel(0)
                 .setAspectMask(vk::ImageAspectFlagBits::eColor);
            barrier.setImage(image)
                   .setOldLayout(vk::ImageLayout::eUndefined)
                   .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                   .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                   .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                   .setDstAccessMask((vk::AccessFlagBits::eTransferWrite))
                   .setSubresourceRange(range);
            cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer,
                                   {}, {}, nullptr, barrier);
        });
}

void Texture::transitionImageLayoutFromDst2Optimal() {
    Context::Instance().commandManager->ExecuteCmd(Context::Instance().graphicsQueue,
        [&](vk::CommandBuffer cmdBuf){
            vk::ImageMemoryBarrier barrier;
            vk::ImageSubresourceRange range;
            range.setLayerCount(1)
                 .setBaseArrayLayer(0)
                 .setLevelCount(1)
                 .setBaseMipLevel(0)
                 .setAspectMask(vk::ImageAspectFlagBits::eColor);
            barrier.setImage(image)
                   .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                   .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                   .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                   .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                   .setSrcAccessMask((vk::AccessFlagBits::eTransferWrite))
                   .setDstAccessMask((vk::AccessFlagBits::eShaderRead))
                   .setSubresourceRange(range);
            cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                                   {}, {}, nullptr, barrier);
        });
}

void Texture::createImageView() {
    vk::ImageViewCreateInfo createInfo;
    vk::ComponentMapping mapping;
    vk::ImageSubresourceRange range;
    range.setAspectMask(vk::ImageAspectFlagBits::eColor)
         .setBaseArrayLayer(0)
         .setLayerCount(1)
         .setLevelCount(1)
         .setBaseMipLevel(0);
    createInfo.setImage(image)
              .setViewType(vk::ImageViewType::e2D)
              .setComponents(mapping)
              .setFormat(vk::Format::eR8G8B8A8Srgb)
              .setSubresourceRange(range);
    view = Context::Instance().device.createImageView(createInfo);
}

}
