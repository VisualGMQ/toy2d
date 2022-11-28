#pragma once

#include "vulkan/vulkan.hpp"
#include "toy2d/context.hpp"
#include "toy2d/command_manager.hpp"
#include "toy2d/swapchain.hpp"
#include "toy2d/math.hpp"
#include "toy2d/buffer.hpp"
#include <limits>

namespace toy2d {

class Renderer {
public:
    Renderer(int maxFlightCount = 2);
    ~Renderer();

    void SetProject(int right, int left, int bottom, int top, int far, int near);
    void DrawRect(const Rect&);
    void SetDrawColor(const Color&);

private:
    struct MVP {
        Mat4 project;
        Mat4 view;
        Mat4 model;
    };

    int maxFlightCount_;
    int curFrame_;
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
    vk::DescriptorPool descriptorPool_;
    std::vector<vk::DescriptorSet> descriptorSets_;

    void createFences();
    void createSemaphores();
    void createCmdBuffers();
    void createBuffers();
    void createUniformBuffers(int flightCount);
    void bufferData();
    void bufferVertexData();
    void bufferIndicesData();
    void bufferMVPData(const Mat4& model);
    void initMats();
    void createDescriptorPool(int flightCount);
    std::vector<vk::DescriptorSet> allocDescriptorSet(int flightCount);
    void allocDescriptorSets(int flightCount);
    void updateDescriptorSets();
    void transformBuffer2Device(Buffer& src, Buffer& dst, size_t srcOffset, size_t dstOffset, size_t size);

    std::uint32_t queryBufferMemTypeIndex(std::uint32_t, vk::MemoryPropertyFlags);
};

}
