set (source_files
    git.cpp
    parse.cpp
    git_clone.cpp
    git_add_commit.cpp
    git_push_pull.cpp
)

add_library(${PROJECT_NAME} ${source_files} include/BlindCodeReview/git.hpp)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${LIBGIT2_INCLUDE_DIRS} # including libgit2 headers
)
install(TARGETS ${PROJECT_NAME})