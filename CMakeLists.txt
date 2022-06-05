cmake_minimum_required(VERSION 3.15)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(StepIntoVulkan
        LANGUAGES CXX)

find_package(Vulkan QUIET)
if (NOT Vulkan_FOUND)
    find_package(PkgConfig)
    pkg_check_modules(Vulkan vulkan REQUIRED IMPORTED_TARGET)
    add_library(Vulkan::Vulkan ALIAS PkgConfig::Vulkan)
endif()

find_package(SDL2 REQUIRED)

find_program(GLSLC_PROGRAM glslc REQUIRED)

aux_source_directory(src SRC)

add_library(stepintovulkan SHARED ${SRC})
target_include_directories(
    stepintovulkan
    PUBLIC include
)
target_compile_features(stepintovulkan PUBLIC cxx_std_17)
target_link_libraries(stepintovulkan PUBLIC Vulkan::Vulkan SDL2::SDL2)

#######################
# test
#######################
enable_testing()
add_subdirectory(test)
