#pragma once

#include "vulkan/vulkan.hpp"
#include "toy2d/context.hpp"
#include "toy2d/command_manager.hpp"
#include "toy2d/swapchain.hpp"
#include "toy2d/math.hpp"
#include "toy2d/buffer.hpp"
#include "toy2d/texture.hpp"
#include <limits>

namespace toy2d {

class Renderer {
public:
    Renderer(int maxFlightCount);
    ~Renderer();

    void SetProject(int right, int left, int bottom, int top, int far, int near);
    void DrawTexture(const Rect&, Texture& texture);
    void DrawLine(const Vec& p1, const Vec& p2);
    void SetDrawColor(const Color&);

    void StartRender();
    void EndRender();

private:
    int maxFlightCount_;
    int curFrame_;
    uint32_t imageIndex_;
    std::vector<vk::Fence> fences_;
    std::vector<vk::Semaphore> imageAvaliableSems_;
    std::vector<vk::Semaphore> renderFinishSems_;
    std::vector<vk::CommandBuffer> cmdBufs_;
    std::unique_ptr<Buffer> rectVerticesBuffer_;
    std::unique_ptr<Buffer> rectIndicesBuffer_;
    std::unique_ptr<Buffer> lineVerticesBuffer_;
    Mat4 projectMat_;
    Mat4 viewMat_;
    std::vector<std::unique_ptr<Buffer>> uniformBuffers_;
    std::vector<std::unique_ptr<Buffer>> deviceUniformBuffers_;
    std::vector<DescriptorSetManager::SetInfo> descriptorSets_;
    vk::Sampler sampler;
    Texture* whiteTexture;
    Color drawColor_ = {1, 1, 1};

    void createFences();
    void createSemaphores();
    void createCmdBuffers();
    void createBuffers();
    void createUniformBuffers(int flightCount);

    void bufferRectData();
    void bufferRectVertexData();
    void bufferRectIndicesData();

    void bufferLineData(const Vec& p1, const Vec& p2);

    void bufferMVPData();
    void initMats();
    void updateDescriptorSets();
    void transformBuffer2Device(Buffer& src, Buffer& dst, size_t srcOffset, size_t dstOffset, size_t size);
    void createWhiteTexture();

    std::uint32_t queryBufferMemTypeIndex(std::uint32_t, vk::MemoryPropertyFlags);
};

}
