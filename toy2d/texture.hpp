#pragma once

#include "vulkan/vulkan.hpp"
#include "buffer.hpp"
#include <string_view>

namespace toy2d {

class Texture final {
public:
    Texture(std::string_view filename);
    ~Texture();

    vk::Image image;
    vk::DeviceMemory memory;
    vk::ImageView view;

private:
    void createImage(uint32_t w, uint32_t h);
    void createImageView();
    void allocMemory();
    uint32_t queryImageMemoryIndex();
    void transitionImageLayoutFromUndefine2Dst();
    void transitionImageLayoutFromDst2Optimal();
    void transformData2Image(Buffer&, uint32_t w, uint32_t h);
};


}
