#pragma once

#include "vulkan/vulkan.hpp"

#include <vector>
#include <array>
#include <exception>
#include <iostream>
#include <optional>
#include <iterator>
#include <fstream>
#include <string>
#include <cmath>
#include <functional>
#include <cstring>

#define ASSERT(expr)  \
if (!(expr)) { \
    throw std::runtime_error(#expr " is nullptr"); \
}

#define Log(fmt, ...) printf("[%s][%d]: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

using SurfaceCreateCallback = std::function<vk::SurfaceKHR(vk::Instance)>;
