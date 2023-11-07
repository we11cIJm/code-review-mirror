cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)

project(
        BlindCodeReview
        LANGUAGES CXX
)

#find_package(unofficial-libgit2 CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
include_directories(${LIBGIT2_INCLUDE_DIRS})
pkg_check_modules(LIBGIT2 REQUIRED IMPORTED_TARGET libgit2)

add_subdirectory(src)
add_subdirectory(bin)
