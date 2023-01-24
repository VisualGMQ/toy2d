#pragma once

#include <memory>
#include <iostream>
#include <optional>
#include <functional>
#include <vector>
#include <array>

#include "vulkan/vulkan.hpp"
#include "swapchain.hpp"
#include "render_process.hpp"
#include "tool.hpp"
#include "command_manager.hpp"
#include "shader.hpp"

namespace toy2d {

class Context {
public:
    using GetSurfaceCallback = std::function<VkSurfaceKHR(VkInstance)>;
    friend void Init(std::vector<const char*>&, GetSurfaceCallback, int, int);
    friend void ResizeSwapchainImage(int w, int h);

    static void Init(std::vector<const char*>& extensions, GetSurfaceCallback);
    static void Quit();
    static Context& Instance();

    struct QueueInfo {
        std::optional<std::uint32_t> graphicsIndex;
        std::optional<std::uint32_t> presentIndex;
    } queueInfo;

    vk::Instance instance;
    vk::PhysicalDevice phyDevice;
    vk::Device device;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    std::unique_ptr<Swapchain> swapchain;
    std::unique_ptr<RenderProcess> renderProcess;
    std::unique_ptr<CommandManager> commandManager;
    std::unique_ptr<Shader> shader;
    vk::Sampler sampler;

private:
    static Context* instance_;
    vk::SurfaceKHR surface_ = nullptr;

    GetSurfaceCallback getSurfaceCb_ = nullptr;

    Context(std::vector<const char*>& extensions, GetSurfaceCallback);
    ~Context();

    void initRenderProcess();
    void initSwapchain(int windowWidth, int windowHeight);
    void initGraphicsPipeline();
    void initCommandPool();
    void initShaderModules();
    void initSampler();
    void getSurface();

    vk::Instance createInstance(std::vector<const char*>& extensions);
    vk::PhysicalDevice pickupPhysicalDevice();
    vk::Device createDevice(vk::SurfaceKHR);

    void queryQueueInfo(vk::SurfaceKHR);
};

}
