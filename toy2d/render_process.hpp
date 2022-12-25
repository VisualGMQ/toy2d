#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

class RenderProcess final {
public:
    vk::Pipeline pipeline;
    vk::PipelineLayout layout;
    vk::RenderPass renderPass;

    ~RenderProcess();

    void InitLayout();
    void InitRenderPass();
    void InitPipeline(int width, int height);
};

}
