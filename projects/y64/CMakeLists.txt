cmake_minimum_required(VERSION 3.0)

project(Y64 VERSION 1.0.0)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard to conform to")

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif (NOT CMAKE_BUILD_TYPE)

message(STATUS "CMake Build Type: ${CMAKE_BUILD_TYPE}")

add_subdirectory(src)
add_subdirectory(test)