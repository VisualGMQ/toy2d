#pragma once

#include "toy2d/context.hpp"

namespace toy2d {

struct Buffer {
    vk::Buffer buffer;
    vk::DeviceMemory memory;
    void* map;
    size_t size;
    size_t requireSize;

    Buffer(vk::BufferUsageFlags usage, size_t size, vk::MemoryPropertyFlags memProperty);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
};

std::uint32_t QueryBufferMemTypeIndex(std::uint32_t requirementBit, vk::MemoryPropertyFlags);

}
