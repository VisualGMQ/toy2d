macro(AddTest NAME)
    set(EXE_NAME "test_${NAME}")
    add_executable(${EXE_NAME} ${NAME}.cpp)
    target_link_libraries(${EXE_NAME} PRIVATE toy2d)
    add_test(NAME ${EXE_NAME}
             COMMAND ${EXE_NAME}
             WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endmacro()

AddTest(helloworld)
