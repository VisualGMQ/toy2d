# toy2D

Toy2d是一个模仿`SDL2_Renderer`功能的2D玩具渲染器，是我在学习Vulkan过程中编写的。

我把我的学习过程录制成视频放在[B站](https://space.bilibili.com/256768793/channel/collectiondetail?sid=404887)了，有兴趣的可以看看。每个视频对应一个分支，可前往不同分支获得不同阶段的代码。

主分支是最终代码。

## 编译

工程使用CMake。需要预先安装好VulkanSDK

Linux和MacOSX下安装好SDL2，然后运行

```cmake
cmake -S . -B cmake-build
cmake --build cmake-build
```

Windows下我只使用VS编译了（其他平台未测试）。下载[编译好的SDL2文件](https://github.com/libsdl-org/SDL/releases/tag/release-2.26.5), 然后再CMake的时候指定SDL2路径：

```text
SDL2_ROOT = <your dir to SDL2>/SDL2-2.0.22-VC
```

然后编译

```cmake
cmake -S . -B cmake-build
cmake --build cmake-build
```

产生`sandbox`可执行文件。请在工程根目录下运行（便于找到资源文件）。
