#include "toy2d/buffer.hpp"
#include "toy2d/context.hpp"

namespace toy2d {

Buffer::Buffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags property): size(size) {
    createBuffer(size, usage);
    auto info = queryMemoryInfo(property);
    allocateMemory(info);
    bindingMem2Buf();
}

Buffer::~Buffer() {
    Context::Instance().device.freeMemory(memory);
    Context::Instance().device.destroyBuffer(buffer);
}

void Buffer::createBuffer(size_t size, vk::BufferUsageFlags usage) {
    vk::BufferCreateInfo createInfo;
    createInfo.setSize(size)
              .setUsage(usage)
              .setSharingMode(vk::SharingMode::eExclusive);

    buffer = Context::Instance().device.createBuffer(createInfo);
}

void Buffer::allocateMemory(MemoryInfo info) {
    vk::MemoryAllocateInfo allocInfo;
    allocInfo.setMemoryTypeIndex(info.index)
             .setAllocationSize(info.size);
    memory = Context::Instance().device.allocateMemory(allocInfo);
}

Buffer::MemoryInfo Buffer::queryMemoryInfo(vk::MemoryPropertyFlags property) {
    MemoryInfo info;
    auto requirements = Context::Instance().device.getBufferMemoryRequirements(buffer);
    info.size = requirements.size;

    auto properties = Context::Instance().phyDevice.getMemoryProperties();
    for (int i = 0; i < properties.memoryTypeCount; i++) {
        if ((1 << i) & requirements.memoryTypeBits &&
            properties.memoryTypes[i].propertyFlags & property) {
            info.index = i;
            break;
        }
    }

    return info;
}

void Buffer::bindingMem2Buf() {
    Context::Instance().device.bindBufferMemory(buffer, memory, 0);
}

}
