#include <BlindCodeReview/git.hpp>

namespace git {

    int Add(const char* path_to_repo) {
        git_repository* repo = nullptr;
        git_index* index = nullptr;

        int error = git_repository_open(&repo, path_to_repo);
        if (error != 0) {
            return Error(git_error_last(), error);
        }

        error = git_repository_index(&index, repo);
        if (error != 0) {
            return Error(git_error_last(), error);
        }

        git_index_add_all(index, nullptr, 0, nullptr, nullptr);
        git_index_write(index);
        git_index_free(index);
        git_repository_free(repo);

        return 0;
    }

} // namespace git