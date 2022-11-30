#include "SDL.h"
#include "SDL_vulkan.h"
#include "toy2d/toy2d.hpp"
#include <iostream>

#include <vector>

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("sandbox",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          1024, 720,
                                          SDL_WINDOW_SHOWN|SDL_WINDOW_VULKAN);
    if (!window) {
        SDL_Log("create window failed");
        exit(2);
    }
    bool shouldClose = false;
    SDL_Event event;

    std::vector<const char*> extensions;
    unsigned int extensionCount;
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
    extensions.resize(extensionCount);
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data());

    for (auto& extension : extensions) {
        std::cout << extension << std::endl;
    }

    toy2d::Init(extensions);

    while (!shouldClose) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldClose = true;
            }
        }
    }

    toy2d::Quit();

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
