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

    void DrawTriangle();

private:
    int maxFlightCount_;
    int curFrame_;
    std::vector<vk::Fence> fences_;
    std::vector<vk::Semaphore> imageAvaliableSems_;
    std::vector<vk::Semaphore> renderFinishSems_;
    std::vector<vk::CommandBuffer> cmdBufs_;

    void createFences();
    void createSemaphores();
    void createCmdBuffers();
};

}
