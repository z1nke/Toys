cmake_minimum_required(VERSION 2.8)
project(LEB128 CXX)

add_compile_options(-g)
add_compile_options(-Wall)
add_compile_options(-Werror)
add_compile_options(-std=c++14)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNODEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

find_package(GTest CONFIG REQUIRED)

file(GLOB srcs "*.cpp")

add_executable(leb128test ${srcs})

target_link_libraries(leb128test PRIVATE
    gtest
    pthread
)
