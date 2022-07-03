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

    auto supportExtensions = vk::enumerateInstanceExtensionProperties();
    std::function<bool(const char*, vk::ExtensionProperties)> equalLambda = [](const char* e1, vk::ExtensionProperties e2) -> bool {
        return std::strcmp(e1, e2.extensionName.data()) == 0;
    };
    if (!CheckElemsInList(extensions, supportExtensions, equalLambda)) {
        Log("have don't support extensions");
    }

    instance_ = createInstance(extensions, debugMode);
    ASSERT(instance_);

    ASSERT(surfaceCb);
    surface_ = surfaceCb(instance_);
    ASSERT(surface_);

    phyDevice_ = pickupPhysicalDevice();
    ASSERT(phyDevice_);
    Log("pickup %s", phyDevice_.getProperties().deviceName.data());

    device_ = new Device(phyDevice_, windowSize.x, windowSize.y, surface_);
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
        std::vector<const char*> layers{"VK_LAYER_KHRONOS_validation"};

        auto supportLayers = vk::enumerateInstanceLayerProperties();
        std::function<bool(const char*, vk::LayerProperties)> equalLambda =
            [](const char* e1, vk::LayerProperties e2) {
                return std::strcmp(e1, e2.layerName.data()) == 0;
            };

        if (!CheckElemsInList(layers, supportLayers, equalLambda)) {
            Log("has don't support layers");
        } else {
            info.setPEnabledLayerNames(layers);
        }
    }
	info.setPEnabledExtensionNames(extensions);
    
    vk::Instance result = vk::createInstance(info);
    return result;
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
