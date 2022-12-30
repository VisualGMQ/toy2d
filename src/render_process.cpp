#include "toy2d/render_process.hpp"
#include "toy2d/context.hpp"
#include "toy2d/swapchain.hpp"
#include "toy2d/vertex.hpp"

namespace toy2d {

RenderProcess::RenderProcess() {
    layout = createLayout();
    renderPass = createRenderPass();
    graphicsPipeline = nullptr;
}

RenderProcess::~RenderProcess() {
    auto& ctx = Context::Instance();
    ctx.device.destroyRenderPass(renderPass);
    ctx.device.destroyPipelineLayout(layout);
    ctx.device.destroyPipeline(graphicsPipeline);
}

void RenderProcess::RecreateGraphicsPipeline(const std::vector<char>& vertexSource, const std::vector<char>& fragSource) {
    if (graphicsPipeline) {
        Context::Instance().device.destroyPipeline(graphicsPipeline);
    }
    graphicsPipeline = createGraphicsPipeline(vertexSource, fragSource);
}

void RenderProcess::RecreateRenderPass() {
    if (renderPass) {
        Context::Instance().device.destroyRenderPass(renderPass);
    }
    renderPass = createRenderPass();
}

vk::PipelineLayout RenderProcess::createLayout() {
    vk::PipelineLayoutCreateInfo createInfo;
    createInfo.setPushConstantRangeCount(0)
              .setSetLayoutCount(0);

    return Context::Instance().device.createPipelineLayout(createInfo);
}

vk::Pipeline RenderProcess::createGraphicsPipeline(const std::vector<char>& vertexSource, const std::vector<char>& fragSource) {
    auto& ctx = Context::Instance();

    vk::GraphicsPipelineCreateInfo createInfo;

    // 0. shader prepare
    vk::ShaderModuleCreateInfo vertexModuleCreateInfo, fragModuleCreateInfo;
    vertexModuleCreateInfo.codeSize = vertexSource.size();
    vertexModuleCreateInfo.pCode = (std::uint32_t*)vertexSource.data();
    fragModuleCreateInfo.codeSize = fragSource.size();
    fragModuleCreateInfo.pCode = (std::uint32_t*)fragSource.data();

    auto vertexModule = ctx.device.createShaderModule(vertexModuleCreateInfo);
    auto fragModule = ctx.device.createShaderModule(fragModuleCreateInfo);

    std::array<vk::PipelineShaderStageCreateInfo, 2> stageCreateInfos;
    stageCreateInfos[0].setModule(vertexModule)
                       .setPName("main")
                       .setStage(vk::ShaderStageFlagBits::eVertex);
    stageCreateInfos[1].setModule(fragModule)
                       .setPName("main")
                       .setStage(vk::ShaderStageFlagBits::eFragment);

    // 1. vertex input
    vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    auto attribute = Vertex::GetAttribute();
    auto binding = Vertex::GetBinding();
    vertexInputCreateInfo.setVertexBindingDescriptions(binding)
                         .setVertexAttributeDescriptions(attribute);

    // 2. vertex assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAsmCreateInfo;
    inputAsmCreateInfo.setPrimitiveRestartEnable(false)
                      .setTopology(vk::PrimitiveTopology::eTriangleList);

    // 3. viewport and scissor
    vk::PipelineViewportStateCreateInfo viewportInfo;
    vk::Viewport viewport(0, 0, ctx.swapchain->GetExtent().width, ctx.swapchain->GetExtent().height, 0, 1);
    vk::Rect2D scissor(vk::Rect2D({0, 0}, ctx.swapchain->GetExtent()));
    viewportInfo.setViewports(viewport)
                .setScissors(scissor);

    // 4. rasteraizer
    vk::PipelineRasterizationStateCreateInfo rasterInfo;
    rasterInfo.setCullMode(vk::CullModeFlagBits::eFront)
              .setFrontFace(vk::FrontFace::eCounterClockwise)
              .setDepthClampEnable(false)
              .setLineWidth(1)
              .setPolygonMode(vk::PolygonMode::eFill)
              .setRasterizerDiscardEnable(false);

    // 5. multisampler
    vk::PipelineMultisampleStateCreateInfo multisampleInfo;
    multisampleInfo.setSampleShadingEnable(false)
                   .setRasterizationSamples(vk::SampleCountFlagBits::e1);

    // 6. depth and stencil buffer
    // We currently don't need depth and stencil buffer

    // 7. blending
    vk::PipelineColorBlendAttachmentState blendAttachmentState;
    blendAttachmentState.setBlendEnable(false)
                        .setColorWriteMask(vk::ColorComponentFlagBits::eA|
                                           vk::ColorComponentFlagBits::eB|
                                           vk::ColorComponentFlagBits::eG|
                                           vk::ColorComponentFlagBits::eR);
    vk::PipelineColorBlendStateCreateInfo blendInfo;
    blendInfo.setAttachments(blendAttachmentState)
             .setLogicOpEnable(false);

    // create graphics pipeline
    createInfo.setStages(stageCreateInfos)
              .setLayout(layout)
              .setPVertexInputState(&vertexInputCreateInfo)
              .setPInputAssemblyState(&inputAsmCreateInfo)
              .setPViewportState(&viewportInfo)
              .setPRasterizationState(&rasterInfo)
              .setPMultisampleState(&multisampleInfo)
              .setPColorBlendState(&blendInfo)
              .setRenderPass(renderPass);

    auto result = ctx.device.createGraphicsPipeline(nullptr, createInfo);
    if (result.result != vk::Result::eSuccess) {
        std::cout << "create graphics pipeline failed: " << result.result << std::endl;
    }

    // clear shader module
    ctx.device.destroyShaderModule(vertexModule);
    ctx.device.destroyShaderModule(fragModule);

    return result.value;
}

vk::RenderPass RenderProcess::createRenderPass() {
    auto& ctx = Context::Instance();

    vk::RenderPassCreateInfo createInfo;

    vk::SubpassDependency dependency;
    dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    vk::AttachmentDescription attachDescription;
    attachDescription.setFormat(ctx.swapchain->GetFormat().format)
                     .setSamples(vk::SampleCountFlagBits::e1)
                     .setLoadOp(vk::AttachmentLoadOp::eClear)
                     .setStoreOp(vk::AttachmentStoreOp::eStore)
                     .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                     .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                     .setInitialLayout(vk::ImageLayout::eUndefined)
                     .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
    vk::AttachmentReference reference;
    reference.setAttachment(0)
             .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpassDesc;
    subpassDesc.setColorAttachments(reference)
               .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

    subpassDesc.setColorAttachments(reference);
    createInfo.setAttachments(attachDescription)
              .setDependencies(dependency)
              .setSubpasses(subpassDesc);

    return Context::Instance().device.createRenderPass(createInfo);
}

}
