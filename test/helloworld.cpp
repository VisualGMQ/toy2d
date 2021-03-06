#include "renderer.hpp"

#include "SDL.h"
#include "SDL_vulkan.h"

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_EVERYTHING);
    Log("SDL init OK");
    SDL_Window* window = SDL_CreateWindow("hello world",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN|SDL_WINDOW_VULKAN);
    Log("SDL window create OK");

    unsigned int count;
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    std::vector<const char*> extensions(count);
    SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());

    toy2d::Renderer::Init(extensions,
                          [&](vk::Instance instance) -> vk::SurfaceKHR {
                              VkSurfaceKHR surface;
                              SDL_Vulkan_CreateSurface(window, instance, &surface);
                              return surface;
                          },
                          toy2d::Vec2{800, 600});
    Log("renderer init OK");

    bool isquit = false;
    SDL_Event event;
    while (!isquit) {
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isquit = true; 
            }
        }
        toy2d::Renderer::Render();
    }

    toy2d::Renderer::Quit();
    Log("renderer quit");
    SDL_DestroyWindow(window);
    SDL_Quit();
    Log("SDL quit");
    return 0;
}
