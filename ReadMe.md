# toy2D

学习Vulkan时编写极简2D渲染器。

目前的效果：

![snapshot][./snapshot/snapshot.png]

我录制的[B站学习视频](https://www.bilibili.com/video/BV1R44y1M7e2?share_source=copy_web)

## 编译

使用CMake进行编译：

```cmake
cmake -S . -B build
cmake --build build
```

生成的程序为`build/test/test_helloworld`


需要在根目录运行此程序（或者将spv文件放到程序的同一目录）
