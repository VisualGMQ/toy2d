#pragma once

#include "vulkan/vulkan.hpp"
#include "buffer.hpp"
#include "descriptor_manager.hpp"
#include <string_view>
#include <string>

namespace toy2d {

class TextureManager;

class Texture final {
public:
    friend class TextureManager;
    ~Texture();

    vk::Image image;
    vk::DeviceMemory memory;
    vk::ImageView view;
    vk::Sampler sampler;
    DescriptorSetManager::SetInfo set;

private:
    Texture(std::string_view filename);
    void createImage(uint32_t w, uint32_t h);
    void createImageView();
    void allocMemory();
    uint32_t queryImageMemoryIndex();
    void transitionImageLayoutFromUndefine2Dst();
    void transitionImageLayoutFromDst2Optimal();
    void transformData2Image(Buffer&, uint32_t w, uint32_t h);
    void updateDescriptorSet();
    void createSampler();
};

class TextureManager final {
public:
    static TextureManager& Instance() {
        if (!instance_) {
            instance_.reset(new TextureManager);
        }
        return *instance_;
    }

    Texture* Load(const std::string& filename);
    void Destroy(Texture*);
    void Clear();

private:
    static std::unique_ptr<TextureManager> instance_;

    std::vector<std::unique_ptr<Texture>> datas_;
};

}
