set(source_files
    git.cpp
    git_clone.cpp
    git_add.cpp
    git_commit.cpp
    git_push.cpp
    git_pull.cpp
    git_merge.cpp
)

add_library(${PROJECT_NAME} ${source_files} include/BlindCodeReview/git.hpp)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${LIBGIT2_INCLUDE_DIRS} # including libgit2 headers
)
install(TARGETS ${PROJECT_NAME})