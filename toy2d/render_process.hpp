#pragma once

#include "vulkan/vulkan.hpp"
#include "toy2d/shader.hpp"
#include <fstream>

namespace toy2d {

class RenderProcess {
public:
    vk::Pipeline graphicsPipelineWithTriangleTopology = nullptr;
    vk::Pipeline graphicsPipelineWithLineTopology = nullptr;
    vk::RenderPass renderPass = nullptr;
    vk::PipelineLayout layout = nullptr;

    RenderProcess();
    ~RenderProcess();

    void CreateGraphicsPipeline(const Shader& shader);
    void CreateRenderPass();

private:
    vk::PipelineCache pipelineCache_ = nullptr;

    vk::PipelineLayout createLayout();
    vk::Pipeline createGraphicsPipeline(const Shader& shader, vk::PrimitiveTopology);
    vk::RenderPass createRenderPass();
    vk::PipelineCache createPipelineCache();
};

}
