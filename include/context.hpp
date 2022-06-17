#pragma once

#include "pch.hpp"
#include "device.hpp"
#include "vertex.hpp"

namespace toy2d {

class Context final {
public:
    friend class Device;

    static void Init(std::vector<const char*> extensions,
                     SurfaceCreateCallback,
                     const Vec2& windowSize,
                     bool debugMode = true);
    static void Quit();

    static Context& GetInstance();

    Context(std::vector<const char*> extensions,
            SurfaceCreateCallback,
            const Vec2& windowSize,
            bool debugMode);
    Context(const Context&) = delete;
    Context& operator=(const Device&) = delete;
    ~Context();

    Device& GetDevice();
    const Vec2& GetWindowSize() const { return windowSize_; }

    void UpdateWindowSize(const Vec2&);

private:
    static Context* context;

    vk::Instance instance_;
    vk::SurfaceKHR surface_;
    vk::PhysicalDevice phyDevice_;
    Vec2 windowSize_;
    Device* device_;

    vk::Instance createInstance(const std::vector<const char*>& extensions, bool debugMode);
    vk::PhysicalDevice pickupPhysicalDevice();
};

}
