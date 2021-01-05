cmake_minimum_required(VERSION 2.8)
project(ttcp CXX)

add_compile_options(-g)
add_compile_options(-Wall)
add_compile_options(-Werror)
add_compile_options(-std=c++11)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNODEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

file(GLOB srcs "*.cpp")

add_executable(ttcp ${srcs})
target_link_libraries(ttcp boost_program_options)
