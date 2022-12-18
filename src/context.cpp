#include "toy2d/context.hpp"

namespace toy2d {

std::unique_ptr<Context> Context::instance_ = nullptr;

void Context::Init(const std::vector<const char*>& extensions) {
    instance_.reset(new Context(extensions));
}

void Context::Quit() {
    instance_.reset();
}

Context& Context::GetInstance() {
    if (!instance_) {
        throw std::runtime_error("Vulkan context create failed!");
    }
    return *instance_;
}

Context::Context(const std::vector<const char*>& extensions) {
    instance = createInstance(extensions);
    if (!instance) {
        throw std::runtime_error("instance create failed!");
    }
}

vk::Instance Context::createInstance(const std::vector<const char*>& extensions) {
    std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};

    vk::InstanceCreateInfo createInfo;
    vk::ApplicationInfo appInfo;
    appInfo.setApiVersion(VK_VERSION_1_3);
    createInfo.setPApplicationInfo(&appInfo)
              .setPEnabledExtensionNames(extensions)
              .setPEnabledLayerNames(layers);

    auto exts = vk::enumerateInstanceExtensionProperties();

    checkExtensionsAndLayers(extensions, layers);
        
    return vk::createInstance(createInfo);
}

bool Context::checkExtensionsAndLayers(const std::vector<const char*>& extensions, const std::vector<const char*>& layers) {
    using LiteralStr = const char*;
    return checkElemsInList<LiteralStr, vk::ExtensionProperties>(
            extensions, vk::enumerateInstanceExtensionProperties(),
            [](const LiteralStr& elem1, const vk::ExtensionProperties& elem2) {
                return std::strcmp(elem1, elem2.extensionName) == 0;
            }) &&
        checkElemsInList<LiteralStr, vk::LayerProperties>(
                layers, vk::enumerateInstanceLayerProperties(), 
                [](const LiteralStr& elem1, const vk::LayerProperties& elem2) {
                return std::strcmp(elem1, elem2.layerName) == 0;
            });


}

Context::~Context() {
    instance.destroy();
}

}
