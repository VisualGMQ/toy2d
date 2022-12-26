#include "toy2d/toy2d.hpp"
#include "SDL.h"
#include "SDL_vulkan.h"
#include <SDL_video.h>

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

    unsigned int count;
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    std::vector<const char*> extensions(count);
    SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());

    toy2d::Init(extensions,
        [&](VkInstance instance){
            VkSurfaceKHR surface;
            SDL_Vulkan_CreateSurface(window, instance, &surface);
            return surface;
        }, 1024, 720);
    auto renderer = toy2d::GetRenderer();

    bool shouldClose = false;
    SDL_Event event;

    while (!shouldClose) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldClose = true;
            }
        }
        renderer->DrawTriangle();
    }
    toy2d::Quit();

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
