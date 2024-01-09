#include <BlindCodeReview/git.hpp>

namespace git {

    int Pull(const std::filesystem::path& local_path_to_repo) {
        std::ofstream output("testrepo/errors.txt", std::ios_base::app);
        Fetch(local_path_to_repo);
        Merge(local_path_to_repo);
        return 0;
    }

    void PullByFile(const std::string& filename) {
        std::ifstream input(filename);
        if (!input.is_open()) {
            throw std::invalid_argument("Cannot open file " + filename);
        }

        std::string url;
        while (input >> url) {
            auto path = static_cast<std::filesystem::path>(GetRepoName(url));
            Pull(path);

            ++current_repo_pos;
            PrintProgressBar();
        }
        current_repo_pos = 0;
    }

} // namespace git