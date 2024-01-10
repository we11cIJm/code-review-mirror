#include <BlindCodeReview/git.hpp>

namespace git {

    int Pull(const std::filesystem::path& local_path_to_repo) {
//        std::ofstream output("../errors.txt", std::ios_base::app);
        Fetch(local_path_to_repo);
        Merge(local_path_to_repo);
        return 0;
    }

    void PullAll(const std::filesystem::path& path_to_all_repos, const std::vector<std::string>& urls) {
        for (auto& url : urls) {
            auto path = static_cast<std::filesystem::path>(GetRepoName(url));
            Pull(path);

//            ++current_repo_pos;
//            PrintProgressBar();
        }
        current_repo_pos = 0;
    }

} // namespace git