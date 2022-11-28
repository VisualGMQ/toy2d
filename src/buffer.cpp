#include "toy2d/buffer.hpp"

namespace toy2d {

Buffer::Buffer(vk::BufferUsageFlags usage, size_t size, vk::MemoryPropertyFlags memProperty) {
    auto& device = Context::Instance().device;

    this->size = size;
    vk::BufferCreateInfo createInfo;
    createInfo.setUsage(usage)
              .setSize(size)
              .setSharingMode(vk::SharingMode::eExclusive);

    buffer = device.createBuffer(createInfo);

    auto requirements = device.getBufferMemoryRequirements(buffer);
    requireSize = requirements.size;
    auto index = queryBufferMemTypeIndex(requirements.memoryTypeBits, memProperty);
    vk::MemoryAllocateInfo allocInfo;
    allocInfo.setMemoryTypeIndex(index)
             .setAllocationSize(requirements.size);
    memory = device.allocateMemory(allocInfo);

    device.bindBufferMemory(buffer, memory, 0);

    if (memProperty & vk::MemoryPropertyFlagBits::eHostVisible) {
        map = device.mapMemory(memory, 0, size);
    } else {
        map = nullptr;
    }
}

Buffer::~Buffer() {
    auto& device = Context::Instance().device;
    if (map) {
        device.unmapMemory(memory);
    }
    device.freeMemory(memory);
    device.destroyBuffer(buffer);
}

std::uint32_t Buffer::queryBufferMemTypeIndex(std::uint32_t type, vk::MemoryPropertyFlags flag) {
    auto property = Context::Instance().phyDevice.getMemoryProperties();

    for (std::uint32_t i = 0; i < property.memoryTypeCount; i++) {
        if ((1 << i) & type &&
            property.memoryTypes[i].propertyFlags & flag) {
                return i;
        }
    }

    return 0;
}



}