#include "toy2d/context.hpp"

namespace toy2d {

Context* Context::instance_ = nullptr;

void Context::Init(std::vector<const char*>& extensions, GetSurfaceCallback cb) {
    instance_ = new Context(extensions, cb);
}

void Context::Quit() {
    delete instance_;
}

Context& Context::Instance() {
    return *instance_;
}

Context::Context(std::vector<const char*>& extensions, GetSurfaceCallback cb) {
    getSurfaceCb_ = cb;

    instance = createInstance(extensions);
    if (!instance) {
        std::cout << "instance create failed" << std::endl;
        exit(1);
    }

    phyDevice = pickupPhysicalDevice();
    if (!phyDevice) {
        std::cout << "pickup physical device failed" << std::endl;
        exit(1);
    }

    getSurface();

    device = createDevice(surface_);
    if (!device) {
        std::cout << "create device failed" << std::endl;
        exit(1);
    }

    graphicsQueue = device.getQueue(queueInfo.graphicsIndex.value(), 0);
    presentQueue = device.getQueue(queueInfo.presentIndex.value(), 0);
}

void Context::getSurface() {
    surface_ = getSurfaceCb_(instance);
    if (!surface_) {
        std::cout << "create surface failed" << std::endl;
        exit(1);
    }
}

vk::Instance Context::createInstance(std::vector<const char*>& extensions) {
    vk::InstanceCreateInfo info; 
    vk::ApplicationInfo appInfo;
    appInfo.setApiVersion(VK_API_VERSION_1_3);
    info.setPApplicationInfo(&appInfo)
        .setPEnabledExtensionNames(extensions);

    std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};
    info.setPEnabledLayerNames(layers);

    return vk::createInstance(info);
}

vk::PhysicalDevice Context::pickupPhysicalDevice() {
    auto devices = instance.enumeratePhysicalDevices();
    if (devices.size() == 0) {
        std::cout << "you don't have suitable device to support vulkan" << std::endl;
        exit(1);
    }
    return devices[0];
}

vk::Device Context::createDevice(vk::SurfaceKHR surface) {
    vk::DeviceCreateInfo deviceCreateInfo;
    queryQueueInfo(surface);
    std::array extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    deviceCreateInfo.setPEnabledExtensionNames(extensions);

    std::vector<vk::DeviceQueueCreateInfo> queueInfos;
    float priority = 1;
    if (queueInfo.graphicsIndex.value() == queueInfo.presentIndex.value()) {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.setPQueuePriorities(&priority);
        queueCreateInfo.setQueueCount(1);
        queueCreateInfo.setQueueFamilyIndex(queueInfo.graphicsIndex.value());
        queueInfos.push_back(queueCreateInfo);
    } else {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.setPQueuePriorities(&priority);
        queueCreateInfo.setQueueCount(1);
        queueCreateInfo.setQueueFamilyIndex(queueInfo.graphicsIndex.value());
        queueInfos.push_back(queueCreateInfo);

        queueCreateInfo.setQueueFamilyIndex(queueInfo.presentIndex.value());
        queueInfos.push_back(queueCreateInfo);
    }
    deviceCreateInfo.setQueueCreateInfos(queueInfos);

    return phyDevice.createDevice(deviceCreateInfo);
}

void Context::queryQueueInfo(vk::SurfaceKHR surface) {
    auto queueProps = phyDevice.getQueueFamilyProperties();
    for (int i = 0; i < queueProps.size(); i++) {
        if (queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            queueInfo.graphicsIndex = i;
        }

        if (phyDevice.getSurfaceSupportKHR(i, surface)) {
            queueInfo.presentIndex = i;
        }

        if (queueInfo.graphicsIndex.has_value() &&
            queueInfo.presentIndex.has_value()) {
            break;
        }
    }
}

void Context::initSwapchain(int windowWidth, int windowHeight) {
    swapchain = std::make_unique<Swapchain>(surface_, windowWidth, windowHeight);
}

void Context::initRenderProcess() {
    renderProcess = std::make_unique<RenderProcess>();
}

void Context::initGraphicsPipeline() {
    renderProcess->CreateGraphicsPipeline(*shader);
}

void Context::initCommandPool() {
    commandManager = std::make_unique<CommandManager>();
}

void Context::initShaderModules() {
    auto vertexSource = ReadWholeFile("./vert.spv");
    auto fragSource = ReadWholeFile("./frag.spv");
    shader = std::make_unique<Shader>(vertexSource, fragSource);
}

void Context::initSampler() {
    vk::SamplerCreateInfo createInfo;
    createInfo.setMagFilter(vk::Filter::eLinear)
              .setMinFilter(vk::Filter::eLinear)
              .setAddressModeU(vk::SamplerAddressMode::eRepeat)
              .setAddressModeV(vk::SamplerAddressMode::eRepeat)
              .setAddressModeW(vk::SamplerAddressMode::eRepeat)
              .setAnisotropyEnable(false)
              .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
              .setUnnormalizedCoordinates(false)
              .setCompareEnable(false)
              .setMipmapMode(vk::SamplerMipmapMode::eLinear);
    sampler = Context::Instance().device.createSampler(createInfo);
}

Context::~Context() {
    shader.reset();
    device.destroySampler(sampler);
    commandManager.reset();
    renderProcess.reset();
    swapchain.reset();
    device.destroy();
    instance.destroy();
}


}
