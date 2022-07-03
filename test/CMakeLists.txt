option(SDL2_DIR "SDL2 drectory" "")

macro(AddTest NAME)
    set(EXE_NAME "test_${NAME}")
    add_executable(${EXE_NAME} ${NAME}.cpp)
    target_link_libraries(${EXE_NAME} PRIVATE SDL2 toy2d)
    add_test(NAME ${EXE_NAME}
             COMMAND ${EXE_NAME}
             WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    if (APPLE)
        target_compile_definitions(${EXE_NAME} PRIVATE "MACOS")
    elseif (WIN32)
        target_compile_definitions(${EXE_NAME} PRIVATE "WINDOWS")
    else()
        target_compile_definitions(${EXE_NAME} PRIVATE "LINUX")
    endif()
endmacro()

AddTest(helloworld)
