cmake_minimum_required(VERSION 3.8)

project(Unreachable VERSION 1.0.0 DESCRIPTION "Unreachable macro" LANGUAGES C CXX)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

set(CMAKE_CXX_STANDARD 11 CACHE STRING "C++ standard to conform to")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(Test
  unreachable.hpp
  unreachable.cpp
  test.cpp
)
