#pragma once

#include "vulkan/vulkan.hpp"
#include "toy2d/context.hpp"
#include "toy2d/command_manager.hpp"
#include "toy2d/swapchain.hpp"
#include <limits>

namespace toy2d {

class Renderer {
public:
    Renderer();
    ~Renderer();

    void DrawTriangle();

private:
    vk::Fence fence_;
    vk::Semaphore imageAvaliableSem_;
    vk::Semaphore renderFinishSem_;
    vk::CommandBuffer cmdBuf_;

    void createFence();
    void createSemaphores();
};

}
