#include "toy2d/command_manager.hpp"
#include "toy2d/context.hpp"

namespace toy2d {

CommandManager::CommandManager() {
    pool_ = createCommandPool();
}

CommandManager::~CommandManager() {
    auto& ctx = Context::Instance();
    ctx.device.destroyCommandPool(pool_);
}

void CommandManager::ResetCmds() {
    Context::Instance().device.resetCommandPool(pool_);
}

vk::CommandPool CommandManager::createCommandPool() {
    auto& ctx = Context::Instance();

    vk::CommandPoolCreateInfo createInfo;

    createInfo.setQueueFamilyIndex(ctx.queueInfo.graphicsIndex.value())
              .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

    return ctx.device.createCommandPool(createInfo);
}

std::vector<vk::CommandBuffer> CommandManager::CreateCommandBuffers(std::uint32_t count) {
    auto& ctx = Context::Instance();

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.setCommandPool(pool_)
             .setCommandBufferCount(1)
             .setLevel(vk::CommandBufferLevel::ePrimary);

    return ctx.device.allocateCommandBuffers(allocInfo);
}

vk::CommandBuffer CommandManager::CreateOneCommandBuffer() {
    return CreateCommandBuffers(1)[0];
}

void CommandManager::FreeCmd(vk::CommandBuffer buf) {
    Context::Instance().device.freeCommandBuffers(pool_, buf);
}

}
