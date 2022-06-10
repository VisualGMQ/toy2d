#include <iostream>
#include "renderer.hpp"

int main(int, char**) {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("hello world",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN|SDL_WINDOW_VULKAN);
    Renderer::Init(window);
    auto vertexShader = Renderer::CreateShaderModule("vert.spv");
    auto fragShader = Renderer::CreateShaderModule("frag.spv");

    Renderer::CreatePipeline(vertexShader, fragShader);

    bool isquit = false;
    SDL_Event event;
    while (!isquit) {
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isquit = true; 
            }
        }
        Renderer::Render();
    }
    Renderer::WaitIdle();

    Renderer::Quit();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
