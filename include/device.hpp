#pragma once

#include "pch.hpp"
#include "vertex.hpp"

namespace toy2d {

struct Buffer {
    vk::Buffer buffer;
    vk::DeviceMemory memory;
    uint32_t size;
};

class Device final {
public:
    Device(vk::PhysicalDevice phyDevice, int w, int h, vk::SurfaceKHR surface);
    ~Device();
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    vk::Queue GetGraphicsQueue() const { return graphicsQueue_; }
    vk::Queue GetPresentQueue() const { return presentQueue_; }

    std::vector<vk::ImageView>& GetImageViews() { return imageViews_; }

    vk::CommandPool CreateCmdPool();
    void DestroyCmdPool(vk::CommandPool pool);
    vk::CommandBuffer AllocateCmdBuffer(vk::CommandPool);
    std::vector<vk::CommandBuffer> AllocateCmdBuffers(vk::CommandPool, uint32_t num);
    void FreeCmdBuffer(vk::CommandPool, vk::CommandBuffer);
    void FreeCmdBuffers(vk::CommandPool, const std::vector<vk::CommandBuffer>&);

    vk::Semaphore CreateSemaphore();
    void DestroySemaphore(vk::Semaphore);

    vk::Fence CreateFence(bool signaled);
    void DestroyFence(vk::Fence);
    void ResetFence(vk::Fence);

    void WaitForFence(vk::Fence, uint32_t timeout = std::numeric_limits<uint32_t>::max());

    vk::SwapchainKHR GetSwapchain() { return swapchain_; }

    vk::Extent2D GetSwapchainExtent() const { return requiredInfo_.extent; }

    Buffer CreateBuffer(vk::BufferUsageFlags, uint32_t size, vk::MemoryPropertyFlags);
    void DestroyBuffer(const Buffer&);

    void* MapMemory(const Buffer&);
    void UnmapMemory(const Buffer&);

    vk::ShaderModule CreateShaderModule(const char* filename);
    void DestroyShaderModule(vk::ShaderModule);

    vk::Framebuffer CreateFramebuffer(vk::RenderPass, vk::ImageView);
    void DestroyFramebuffer(vk::Framebuffer);

    vk::Pipeline CreatePipeline(vk::ShaderModule vertexShader, vk::ShaderModule fragShader, const Vec2& windowSize, vk::RenderPass, vk::PipelineLayout);
    void DestroyPipeline(vk::Pipeline);

    vk::PipelineLayout CreateLayout();
    void DestroyLayout(vk::PipelineLayout);

    vk::RenderPass CreateRenderPass();
    void DestroyRenderPass(vk::RenderPass renderPass);

    void WaitIdle();

    unsigned int AcquireNextImage(vk::Semaphore sem);

private:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsIndices;
        std::optional<uint32_t> presentIndices;
    } queueIndices_;

    struct SwapchainRequiredInfo {
        vk::SurfaceCapabilitiesKHR capabilities;
        vk::Extent2D extent;
        vk::SurfaceFormatKHR format;
        vk::PresentModeKHR presentMode;
        uint32_t imageCount;
    } requiredInfo_;

    struct MemRerquiedInfo {
        uint32_t index;
        uint32_t size;
    };

    vk::PhysicalDevice phyDevice_;
    vk::Device device_;
    vk::SwapchainKHR swapchain_;
    std::vector<vk::Image> images_;
    std::vector<vk::ImageView> imageViews_;
    vk::Queue graphicsQueue_;
    vk::Queue presentQueue_;

    vk::Device createDevice();
    vk::SwapchainKHR createSwapchain(vk::SurfaceKHR);
    std::vector<vk::ImageView> createImageViews();

    QueueFamilyIndices queryPhysicalDevice(vk::SurfaceKHR);
    SwapchainRequiredInfo querySwapchainRequiredInfo(int w, int h, vk::SurfaceKHR);
    MemRerquiedInfo queryMemInfo(vk::Buffer, vk::MemoryPropertyFlags);

    vk::Buffer createBuffer(vk::BufferUsageFlags flag, uint32_t size);
    vk::DeviceMemory allocateMem(vk::Buffer, vk::MemoryPropertyFlags flag);
};

}
