# toy2D

Toy2d is a toy renderer imitates `SDL_Renderer` when I'm learning Vulkan.

## compile

use CMake to compile:

Under Linux and MacOSX, compile is manually:

```cmake
cmake -S . -B cmake-build
cmake --build cmake-build
```

Under Windows, you should use Visual Studio to compile. You should point out the SDL2 devel Root:

```text
SDL2_ROOT = D:/Program/3rdlibs/SDL2-2.0.22-VC
```

and use cmake to compile:

```cmake
cmake -S . -B cmake-build
cmake --build cmake-build
```

the example program is `sandbox.exe`, please run it at this directory