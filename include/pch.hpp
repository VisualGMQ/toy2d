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


template <typename ElemT1, typename ElemT2>
using EqualFn = std::function<bool(ElemT1, ElemT2 r2)>;

template <typename ElemT1, typename ElemT2>
bool CheckElemsInList(const std::vector<ElemT1>& elems,
                      const std::vector<ElemT2>& list,
                      EqualFn<ElemT1, ElemT2> equalFn = [](ElemT1 e1, ElemT2 e2) { return e1 == e2; }) {
    int validCount = 0;
    for (auto& checkElem: elems) {
        for (auto& elem: list) {
            if (equalFn && equalFn(checkElem, elem)) {
                validCount++;
                break;
            }
        }
    }
    return validCount == elems.size();
}

