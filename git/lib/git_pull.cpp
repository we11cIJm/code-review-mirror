#include <BlindCodeReview/git.hpp>

namespace git {

    int Pull(const std::filesystem::path& local_path_to_repo) {
        int error = Fetch(local_path_to_repo);
        if (error != 0) {
            Error(git_error_last(), error, std::filesystem::path(local_path_to_repo) / "repos", local_path_to_repo.filename());
        }
        error = Merge(local_path_to_repo);
        if (error != 0) {
            Error(git_error_last(), error, std::filesystem::path(local_path_to_repo) / "repos", local_path_to_repo.filename());
        }

        return 0;
    }

    // TODO: path_to_all_repos contain only folder instead of absolute path
    void PullAll(const std::filesystem::path& path_to_all_repos, const std::vector<std::string>& urls) {
        for (auto& url : urls) {
            auto path = static_cast<std::filesystem::path>(path_to_all_repos / GetRepoName(url));
            Pull(path);
        }
    }

} // namespace git