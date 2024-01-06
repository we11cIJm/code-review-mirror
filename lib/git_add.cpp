#include <BlindCodeReview/git.hpp>

namespace git {

    int Add(const std::filesystem::path& local_path_to_repo) {
        git_repository* repo = nullptr;
        git_index* index = nullptr;

        int error = git_repository_open(&repo, local_path_to_repo.c_str());
        if (error != 0) {
            CleanUp(repo, index);
            return Error(git_error_last(), error, static_cast<const std::string>(local_path_to_repo));
        }

        error = git_repository_index(&index, repo);
        if (error != 0) {
            CleanUp(repo, index);
            return Error(git_error_last(), error, static_cast<const std::string>(local_path_to_repo));
        }

        git_index_add_all(index, nullptr, 0, nullptr, nullptr);
        git_index_write(index);
        CleanUp(repo, index);

        return 0;
    }

} // namespace git