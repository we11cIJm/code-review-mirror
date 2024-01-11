# git/cmake
#find_package(PkgConfig REQUIRED)
include_directories(${LIBGIT2_INCLUDE_DIRS})
#pkg_check_modules(LIBGIT2 REQUIRED IMPORTED_TARGET libgit2)

add_subdirectory(lib)
add_subdirectory(bin)

