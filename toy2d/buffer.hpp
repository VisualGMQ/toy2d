#pragma once

#include "toy2d/context.hpp"

namespace toy2d {

struct Buffer {
    vk::Buffer buffer;
    vk::DeviceMemory memory;
    size_t size;
    size_t requireSize;

    Buffer(vk::BufferUsageFlags usage, size_t size, vk::MemoryPropertyFlags memProperty);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

private:
    std::uint32_t queryBufferMemTypeIndex(std::uint32_t requirementBit, vk::MemoryPropertyFlags);
};

}