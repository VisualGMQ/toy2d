#include "toy2d/vertex.hpp"

namespace toy2d {

vk::VertexInputAttributeDescription Vertex::GetAttributeDescription() {
    vk::VertexInputAttributeDescription description;
    description.setBinding(0)
                .setFormat(vk::Format::eR32G32Sfloat)
                .setLocation(0)
                .setOffset(0);
    return description;
}

vk::VertexInputBindingDescription Vertex::GetBindingDescription() {
    vk::VertexInputBindingDescription description;
    description.setBinding(0)
               .setStride(sizeof(float) * 2)
               .setInputRate(vk::VertexInputRate::eVertex);

    return description;
}

}