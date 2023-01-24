#include "toy2d/render_process.hpp"
#include "toy2d/context.hpp"
#include "toy2d/swapchain.hpp"
#include "toy2d/math.hpp"

namespace toy2d {

RenderProcess::RenderProcess() {
    layout = createLayout();
    CreateRenderPass();
    graphicsPipeline = nullptr;
}

RenderProcess::~RenderProcess() {
    auto& ctx = Context::Instance();
    auto& device = ctx.device;
    device.destroyRenderPass(renderPass);
    device.destroyPipelineLayout(layout);
    device.destroyPipeline(graphicsPipeline);
}

void RenderProcess::CreateGraphicsPipeline(const Shader& shader) {
    graphicsPipeline = createGraphicsPipeline(shader);
}

void RenderProcess::CreateRenderPass() {
    renderPass = createRenderPass();
}

vk::PipelineLayout RenderProcess::createLayout() {
    vk::PipelineLayoutCreateInfo createInfo;
    auto range = Context::Instance().shader->GetPushConstantRange();
    createInfo.setSetLayouts(Context::Instance().shader->GetDescriptorSetLayouts())
              .setPushConstantRanges(range);

    return Context::Instance().device.createPipelineLayout(createInfo);
}

vk::Pipeline RenderProcess::createGraphicsPipeline(const Shader& shader) {
    auto& ctx = Context::Instance();

    vk::GraphicsPipelineCreateInfo createInfo;

    // 0. shader prepare
    std::array<vk::PipelineShaderStageCreateInfo, 2> stageCreateInfos;
    stageCreateInfos[0].setModule(shader.GetVertexModule())
                       .setPName("main")
                       .setStage(vk::ShaderStageFlagBits::eVertex);
    stageCreateInfos[1].setModule(shader.GetFragModule())
                       .setPName("main")
                       .setStage(vk::ShaderStageFlagBits::eFragment);

    // 1. vertex input
    vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    auto attributeDesc = Vec::GetAttributeDescription();
    auto bindingDesc = Vec::GetBindingDescription();
    vertexInputCreateInfo.setVertexAttributeDescriptions(attributeDesc)
                         .setVertexBindingDescriptions(bindingDesc);

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
    /*
     * newRGB = (srcFactor * srcRGB) <op> (dstFactor * dstRGB)
     * newA = (srcFactor * srcA) <op> (dstFactor * dstA)
     *
     * newRGB = 1 * srcRGB + (1 - srcA) * dstRGB
     * newA = srcA === 1 * srcA + 0 * dstA
     */
    vk::PipelineColorBlendAttachmentState blendAttachmentState;
    blendAttachmentState.setBlendEnable(true)
                        .setColorWriteMask(vk::ColorComponentFlagBits::eA|
                                           vk::ColorComponentFlagBits::eB|
                                           vk::ColorComponentFlagBits::eG|
                                           vk::ColorComponentFlagBits::eR)
                        .setSrcColorBlendFactor(vk::BlendFactor::eOne)
                        .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                        .setColorBlendOp(vk::BlendOp::eAdd)
                        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
                        .setAlphaBlendOp(vk::BlendOp::eAdd);

    vk::PipelineColorBlendStateCreateInfo blendInfo;
    blendInfo.setAttachments(blendAttachmentState)
             .setLogicOpEnable(false);

    // dynamic changing state of pipeline
    vk::PipelineDynamicStateCreateInfo dynamicState;
    std::array states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    dynamicState.setDynamicStates(states);

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
              // .setPDynamicState(&dynamicState);

    auto result = ctx.device.createGraphicsPipeline(nullptr, createInfo);
    if (result.result != vk::Result::eSuccess) {
        std::cout << "create graphics pipeline failed: " << result.result << std::endl;
    }

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
