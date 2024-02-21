cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
project(
    distributer C CXX
)

add_subdirectory(lib)

# add_library(distributer INTERFACE)
target_include_directories(distributer INTERFACE .)