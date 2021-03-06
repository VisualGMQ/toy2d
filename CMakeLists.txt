cmake_minimum_required(VERSION 3.15)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Toy2D
        LANGUAGES CXX
        DESCRIPTION "a toy 2d renderer made in vulkan")

include(cmake/FindVulkan.cmake)
include(cmake/FindSDL2.cmake)

find_program(GLSLC_PROGRAM glslc REQUIRED)

message(STATUS "run glslc to compile shaders ...")
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/shader/shader.vert -o vert.spv)
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/shader/shader.frag -o frag.spv)
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
