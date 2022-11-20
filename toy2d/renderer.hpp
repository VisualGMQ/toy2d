#pragma once

#include "vulkan/vulkan.hpp"
#include "toy2d/context.hpp"
#include "toy2d/command_manager.hpp"
#include "toy2d/swapchain.hpp"
#include <limits>

namespace toy2d {

class Renderer {
public:
    Renderer(int maxFlightCount = 2);
    ~Renderer();

    void DrawRect();

private:
    struct Buffer {
        vk::Buffer buffer = nullptr;
        vk::DeviceMemory memory = nullptr;
        std::uint32_t size = 0;
    };

    struct MemInfo {
        vk::DeviceSize size;
        std::uint32_t index;
    };

    int maxFlightCount_;
    int curFrame_;
    std::vector<vk::Fence> fences_;
    std::vector<vk::Semaphore> imageAvaliableSems_;
    std::vector<vk::Semaphore> renderFinishSems_;
    std::vector<vk::CommandBuffer> cmdBufs_;
    Buffer verticesBuffer_;
    Buffer indicesBuffer_;

    void createFences();
    void createSemaphores();
    void createCmdBuffers();
    void createBuffers();
    void bufferData();

    std::uint32_t queryBufferMemTypeIndex(std::uint32_t, vk::MemoryPropertyFlags);
};

}
