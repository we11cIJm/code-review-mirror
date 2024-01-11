set(source_files
    git.cpp
    git_clone.cpp
    git_add.cpp
    git_commit.cpp
    git_push.cpp
    git_pull.cpp
    git_merge.cpp
    git_fetch.cpp
    help_stuff.cpp
)

set(header_files
    include/BlindCodeReview/git.hpp
)

add_library(gitlib ${source_files} ${header_files})
target_include_directories(gitlib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${LIBGIT2_INCLUDE_DIRS} # including libgit2 headers
)
install(TARGETS gitlib)
