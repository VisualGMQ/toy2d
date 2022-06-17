#pragma once

#include "pch.hpp"
#include "context.hpp"
#include "vertex.hpp"

namespace toy2d {

class RawRenderer final {
public:
    RawRenderer();
    ~RawRenderer();

    void Render();

private:
    std::array<Vertex, 4> vertices_;
    std::array<uint16_t, 6> indices_;

    vk::PipelineLayout layout_;
    vk::RenderPass renderPass_;
    vk::Pipeline pipeline_;
    vk::ShaderModule vertexShader_;
    vk::ShaderModule fragShader_;
    vk::CommandPool cmdPool_;
    std::vector<vk::CommandBuffer> drawCmdBufs_;

    Buffer vertexBuffer_;
    Buffer indexBuffer_;

    std::vector<vk::Semaphore> imageAvaliableSems_;
    std::vector<vk::Semaphore> drawFinishSems_;
    std::vector<vk::Fence> fences_;
    std::vector<vk::Framebuffer> framebuffers_;

    unsigned int curInFlightIndex_ = 0;

    template <typename T>
    void copyData2Device(const T& data, size_t size, Buffer);

    void copyBuffer2Device(Buffer src, Buffer dst, vk::CommandBuffer cmd);
    
    void recordCmd(vk::CommandBuffer, vk::Framebuffer);
};

template <typename T>
void RawRenderer::copyData2Device(const T& data, size_t size, Buffer buffer) {
    auto& device = Context::GetInstance().GetDevice();

    auto transBuf = device.CreateBuffer(vk::BufferUsageFlagBits::eTransferSrc,
                                        size,
                                        vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
    ASSERT(transBuf.buffer);
    ASSERT(transBuf.memory);
    void* ptr = device.MapMemory(transBuf);
        memcpy(ptr, data, size);
    device.UnmapMemory(transBuf);

    vk::CommandBuffer transCmdBuf = device.AllocateCmdBuffer(cmdPool_);

    copyBuffer2Device(transBuf, buffer, transCmdBuf);

    device.DestroyBuffer(transBuf);
    device.FreeCmdBuffer(cmdPool_, transCmdBuf);
}

}
