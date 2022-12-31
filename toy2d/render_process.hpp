#pragma once

#include "vulkan/vulkan.hpp"
#include <fstream>

namespace toy2d {

class RenderProcess {
public:
    vk::Pipeline graphicsPipeline = nullptr;
    vk::RenderPass renderPass = nullptr;
    vk::PipelineLayout layout = nullptr;
    vk::DescriptorSetLayout setLayout = nullptr;

    RenderProcess();
    ~RenderProcess();

    void RecreateGraphicsPipeline(const std::vector<char>& vertexSource, const std::vector<char>& fragSource);
    void RecreateRenderPass();

private:
    vk::PipelineLayout createLayout();
    vk::Pipeline createGraphicsPipeline(const std::vector<char>& vertexSource, const std::vector<char>& fragSource);
    vk::RenderPass createRenderPass();
    vk::DescriptorSetLayout createSetLayout();
};

}
