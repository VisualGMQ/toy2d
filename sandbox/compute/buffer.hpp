#pragma once

#include "vulkan/vulkan.hpp"

struct Buffer {
    vk::Buffer buffer;
    vk::DeviceMemory memory;
    void* map;
    size_t size;
    size_t requireSize;

    Buffer(vk::PhysicalDevice phyDevice, vk::Device device,
           vk::BufferUsageFlags usage, size_t size,
           vk::MemoryPropertyFlags memProperty);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

private:
    vk::Device device_;
    vk::PhysicalDevice phyDevice_;

    std::uint32_t queryBufferMemTypeIndex(std::uint32_t type, vk::MemoryPropertyFlags flag);
};

std::uint32_t QueryBufferMemTypeIndex(std::uint32_t requirementBit, vk::MemoryPropertyFlags);
