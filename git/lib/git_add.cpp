#include <BlindCodeReview/git.hpp>

namespace git {

    void Check(git_repository* repo, const std::filesystem::path& local_path_to_repo) {
        if(git_oid_cmp(help_stuff::GetLastCommits(repo).first, help_stuff::GetLastCommits(repo).second)) {
            Pull(local_path_to_repo);
        }
    }

    int Add(const std::filesystem::path& local_path_to_repo) {
        git_repository* repo = nullptr;
        git_index* index = nullptr;

        int error = git_repository_open(&repo, local_path_to_repo.string().c_str());
        if (error != 0) {
            CleanUp(repo, index);
            return error;
        }

        // checks that last remote repo commit the same as last local repo commit
        Check(repo, local_path_to_repo);

        error = git_repository_index(&index, repo);
        if (error != 0) {
            CleanUp(repo, index);
            return error;
        }

        git_index_add_all(index, nullptr, 0, nullptr, nullptr);
        git_index_write(index);
        CleanUp(repo, index);

        return 0;
    }

} // namespace git