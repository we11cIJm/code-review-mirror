cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(
    BlindCodeReview
    LANGUAGES CXX
)

find_package(imgui REQUIRED)
find_package(OpenGL REQUIRED)

find_package(PkgConfig REQUIRED)
include_directories(${LIBGIT2_INCLUDE_DIRS})
pkg_check_modules(LIBGIT2 REQUIRED IMPORTED_TARGET libgit2)

add_subdirectory(git)
add_subdirectory(gui)
add_subdirectory(distribution)
