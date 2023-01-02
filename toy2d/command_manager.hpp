#pragma once

#include "vulkan/vulkan.hpp"
#include <functional>

namespace toy2d {

class CommandManager final {
public:
    CommandManager();
    ~CommandManager();

    vk::CommandBuffer CreateOneCommandBuffer();
    std::vector<vk::CommandBuffer> CreateCommandBuffers(std::uint32_t count);
    void ResetCmds();
    void FreeCmd(const vk::CommandBuffer&);

    using RecordCmdFunc = std::function<void(vk::CommandBuffer&)>;
    void ExecuteCmd(vk::Queue, RecordCmdFunc);

private:
    vk::CommandPool pool_;

    vk::CommandPool createCommandPool();
};

}
