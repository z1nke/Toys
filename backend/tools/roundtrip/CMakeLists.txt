cmake_minimum_required(VERSION 2.8)

project(roundtrip CXX)

add_compile_options(-g)
add_compile_options(-Wall)
add_compile_options(-Werror)

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNODEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_executable(roundtrip roundtrip.cpp)
target_link_libraries(roundtrip pthread)
