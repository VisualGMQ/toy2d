#include "toy2d/render_process.hpp"
#include "toy2d/shader.hpp"
#include "toy2d/context.hpp"

namespace toy2d {

void RenderProcess::InitPipeline(int width, int height) {
    vk::GraphicsPipelineCreateInfo createInfo;

    // 1. Vertex Input
    vk::PipelineVertexInputStateCreateInfo inputState;
    createInfo.setPVertexInputState(&inputState);

    // 2. Vertex Assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAsm;
    inputAsm.setPrimitiveRestartEnable(false)
            .setTopology(vk::PrimitiveTopology::eTriangleList);
    createInfo.setPInputAssemblyState(&inputAsm);

    // 3. Shader
    auto stages = Shader::GetInstance().GetStage();
    createInfo.setStages(stages);

    // 4. viewport
    vk::PipelineViewportStateCreateInfo viewportState;
    vk::Viewport viewport(0, 0, width, height, 0, 1);
    viewportState.setViewports(viewport);
    vk::Rect2D rect({0, 0}, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
    viewportState.setScissors(rect);
    createInfo.setPViewportState(&viewportState);

    // 5. Rasterization
    vk::PipelineRasterizationStateCreateInfo rastInfo;
    rastInfo.setRasterizerDiscardEnable(false)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eCounterClockwise)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setLineWidth(1);
    createInfo.setPRasterizationState(&rastInfo);

    // 6. multisample
    vk::PipelineMultisampleStateCreateInfo multisample;
    multisample.setSampleShadingEnable(false)
               .setRasterizationSamples(vk::SampleCountFlagBits::e1);
    createInfo.setPMultisampleState(&multisample);

    // 7. test - stencil test, depth test

    // 8. color blending
    vk::PipelineColorBlendStateCreateInfo blend;
    vk::PipelineColorBlendAttachmentState attachs;
    attachs.setBlendEnable(false)
           .setColorWriteMask(vk::ColorComponentFlagBits::eA|
                              vk::ColorComponentFlagBits::eB|
                              vk::ColorComponentFlagBits::eG|
                              vk::ColorComponentFlagBits::eR);
           
    blend.setLogicOpEnable(false)
         .setAttachments(attachs);
    createInfo.setPColorBlendState(&blend);

    auto result = Context::GetInstance().device.createGraphicsPipeline(nullptr, createInfo);
    if (result.result != vk::Result::eSuccess) {
        throw std::runtime_error("create graphics pipeline failed");
    }
    pipeline = result.value;
}

void RenderProcess::DestroyPipeline() {
    Context::GetInstance().device.destroyPipeline(pipeline);
}

}
