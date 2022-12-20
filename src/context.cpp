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

Context::~Context() {
    instance.destroy(); 
}

}
