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
    void DrawRect(const Rect&);
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
    std::unique_ptr<Buffer> verticesBuffer_;
    std::unique_ptr<Buffer> indicesBuffer_;
    Mat4 projectMat_;
    Mat4 viewMat_;
    std::vector<std::unique_ptr<Buffer>> uniformBuffers_;
    std::vector<std::unique_ptr<Buffer>> colorBuffers_;
    std::vector<std::unique_ptr<Buffer>> deviceUniformBuffers_;
    std::vector<std::unique_ptr<Buffer>> deviceColorBuffers_;
    std::vector<DescriptorSetManager::SetInfo> descriptorSets_;

    std::unique_ptr<Texture> texture;
    vk::Sampler sampler;

    void createFences();
    void createSemaphores();
    void createCmdBuffers();
    void createBuffers();
    void createUniformBuffers(int flightCount);
    void bufferData();
    void bufferVertexData();
    void bufferIndicesData();
    void bufferMVPData();
    void initMats();
    void updateDescriptorSets();
    void transformBuffer2Device(Buffer& src, Buffer& dst, size_t srcOffset, size_t dstOffset, size_t size);
    void createTexture();

    std::uint32_t queryBufferMemTypeIndex(std::uint32_t, vk::MemoryPropertyFlags);
};

}
