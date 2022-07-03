cmake_minimum_required(VERSION 3.15)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Toy2D
        LANGUAGES CXX)

find_package(Vulkan QUIET)
include(cmake/FindSDL2.cmake)

if (NOT Vulkan_FOUND)
    find_package(PkgConfig)
    pkg_check_modules(Vulkan vulkan REQUIRED IMPORTED_TARGET)
    add_library(Vulkan::Vulkan ALIAS PkgConfig::Vulkan)
endif()

find_program(GLSLC_PROGRAM glslc REQUIRED)
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/shader/shader.vert -o vert.spv)
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/shader/shader.frag -o frag.spv)
message(STATUS "run glslc to compile shaders ...")
message(STATUS "compile shader OK")

aux_source_directory(src SRC)

add_library(toy2d STATIC ${SRC})
target_include_directories(
    toy2d
    PUBLIC include
)
target_link_libraries(toy2d PUBLIC Vulkan::Vulkan)
target_compile_features(toy2d PUBLIC cxx_std_17)

#######################
# test
#######################
enable_testing()
add_subdirectory(test)
