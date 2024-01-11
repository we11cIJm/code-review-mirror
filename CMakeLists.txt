cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(
    BlindCodeReview
    LANGUAGES CXX
)
add_subdirectory(git)
add_subdirectory(gui)

