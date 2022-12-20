#include "toy2d/context.hpp"
#include <vector>

namespace toy2d {

std::unique_ptr<Context> Context::instance_ = nullptr;

void Context::Init() {
    instance_.reset(new Context);
}

void Context::Quit() {
    instance_.reset();
}

Context::Context() {
    createInstance();
    pickupPhyiscalDevice();
    queryQueueFamilyIndices();
    createDevice();
    getQueues();
}

Context::~Context() {
    device.destroy();
    instance.destroy(); 
}

void Context::createInstance() {
    vk::InstanceCreateInfo createInfo;
    vk::ApplicationInfo appInfo;
    appInfo.setApiVersion(VK_VERSION_1_3);
    createInfo.setPApplicationInfo(&appInfo);
    instance = vk::createInstance(createInfo);

    std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};

    RemoveNosupportedElems<const char*, vk::LayerProperties>(layers, vk::enumerateInstanceLayerProperties(),
                           [](const char* e1, const vk::LayerProperties& e2) {
                                return std::strcmp(e1, e2.layerName) == 0;
                           });
    createInfo.setPEnabledLayerNames(layers);

    instance = vk::createInstance(createInfo);
}

void Context::pickupPhyiscalDevice() {
    auto devices = instance.enumeratePhysicalDevices();
    phyDevice = devices[0];

    std::cout << phyDevice.getProperties().deviceName << std::endl;
}

void Context::createDevice() {
    vk::DeviceCreateInfo createInfo;
    vk::DeviceQueueCreateInfo queueCreateInfo;
    float priorities = 1.0;
    queueCreateInfo.setPQueuePriorities(&priorities)
                   .setQueueCount(1)
                   .setQueueFamilyIndex(queueFamilyIndices.graphicsQueue.value());
    createInfo.setQueueCreateInfos(queueCreateInfo);

    device = phyDevice.createDevice(createInfo);
}

void Context::queryQueueFamilyIndices() {
    auto properties = phyDevice.getQueueFamilyProperties();
    for (int i = 0; i < properties.size(); i++) {
        const auto& property = properties[i];
        if (property.queueFlags | vk::QueueFlagBits::eGraphics) {
            queueFamilyIndices.graphicsQueue = i;
            break;
        }
    }
}

void Context::getQueues() {
    graphcisQueue = device.getQueue(queueFamilyIndices.graphicsQueue.value(), 0);
}

}
