#include "context.hpp"

namespace toy2d {

Context* Context::context = nullptr;

void Context::Init(std::vector<const char*> extensions,
                   SurfaceCreateCallback cb,
                   const Vec2& windowSize,
                   bool debugMode) {
    context = new Context(extensions, cb, windowSize, debugMode);
}

void Context::Quit() {
    delete context;
}

Context& Context::GetInstance() {
    ASSERT(context);
    return *context;
}


Context::Context(std::vector<const char*> extensions,
                 SurfaceCreateCallback surfaceCb,
                 const Vec2& windowSize,
                 bool debugMode): windowSize_(windowSize) {
#ifdef MACOS
    extensions.push_back("VK_KHR_get_physical_device_properties2");
#endif
    instance_ = createInstance(extensions, debugMode);
    ASSERT(instance_);
    Log("instance create OK");

    ASSERT(surfaceCb);
    surface_ = surfaceCb(instance_);
    ASSERT(surface_);
    Log("surface create OK");

    phyDevice_ = pickupPhysicalDevice();
    ASSERT(phyDevice_);
    Log("pickup %s", phyDevice_.getProperties().deviceName.data());

    device_ = new Device(phyDevice_, windowSize.x, windowSize.y, surface_);
    ASSERT(device_);
    Log("device create OK");
}

Device& Context::GetDevice() {
    ASSERT(device_);
    return *device_;
}

void Context::UpdateWindowSize(const Vec2& size) {
    windowSize_ = size;
}

vk::PhysicalDevice Context::pickupPhysicalDevice() {
    return instance_.enumeratePhysicalDevices()[0];
}

vk::Instance Context::createInstance(const std::vector<const char*>& extensions, bool debugMode) {
    vk::InstanceCreateInfo info;

    if (debugMode) {
        std::array<const char*, 1> layers{"VK_LAYER_KHRONOS_validation"};
        info.setPEnabledLayerNames(layers);
    }
	info.setPEnabledExtensionNames(extensions);

    return vk::createInstance(info);
}

Context::~Context() {
    delete device_;
    if (surface_) {
        instance_.destroySurfaceKHR(surface_);
    }
    if (instance_) {
        instance_.destroy();
    }
}

}
