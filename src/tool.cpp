#include "toy2d/tool.hpp"

namespace toy2d {

std::vector<char> ReadWholeFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary|std::ios::ate);

    if (!file.is_open()) {
        std::cout << "read " << filename << " failed" << std::endl;
        return std::vector<char>{};
    }

    auto size = file.tellg();
    std::vector<char> content(size);
    file.seekg(0);

    file.read(content.data(), content.size());

    return content;
}

}