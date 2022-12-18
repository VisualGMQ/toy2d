#pragma once
#include "vulkan/vulkan.hpp"
#include <memory>
#include <iostream>
#include <functional>

namespace toy2d {

class Context final {
public:
    static void Init(const std::vector<const char*>& extensions);
    static void Quit();
    static Context& GetInstance();

    vk::Instance instance;

    ~Context();

private:
    static std::unique_ptr<Context> instance_;

    Context(const std::vector<const char*>& extensions);

    vk::Instance createInstance(const std::vector<const char*>& extensions);
    bool checkExtensionsAndLayers(const std::vector<const char*>& extensions, const std::vector<const char*>& layers);

    template <typename T1, typename T2>
    bool checkElemsInList(const std::vector<T1>& elems, const std::vector<T2>& list,
                          std::function<bool(const T1&, const T2&)> eq) {
        for (auto& elem1 : elems) {
            bool found = false;
            for (auto& elem2 : list) {
                if (eq(elem1, elem2)) {
                    found = true;
                }
            }
            if (!found) {
                return false;
            }
        }
        return true;
    }
};

}
