#include <BlindCodeReview/git.hpp>

namespace git {

    int Clone(const char* url, const std::filesystem::path& local_path) {
        git_repository* cloned_repo = nullptr;
        git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
        git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

        checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
        clone_opts.checkout_opts = checkout_opts;
        clone_opts.fetch_opts.callbacks.credentials = CredentialsCallback;

        int error = git_clone(&cloned_repo, url, local_path.c_str(), &clone_opts);
        if (error != 0) {
            const std::string lp = local_path;
            CleanUp(cloned_repo);
            return Error(git_error_last(), error, lp.substr(lp.size() - GetRepoName(url).size(), lp.npos));
        } else if (cloned_repo) {
            git_repository_free(cloned_repo);
        }

        return error;
    }

    void CloneByFile(const std::filesystem::path& path_to_urls_file, const std::filesystem::path& local_path /* = "." */) {
        std::ifstream input(path_to_urls_file);
        if (!input.is_open()) {
            throw std::invalid_argument("Cannot open file " + path_to_urls_file);
        }

        std::string url;
        while (input >> url) {
            ++total_repos_count;
        }
        input.clear();
        input.seekg(0);

        PrintProgressBar();

        while (input >> url) {
            std::filesystem::path full_local_path =
                    std::filesystem::path(local_path) / "repos" / static_cast<std::filesystem::path>(GetRepoName(url));
            Clone(url.c_str(), full_local_path);

            ++current_repo_pos;
            PrintProgressBar();
        }
        current_repo_pos = 0;
    }

} // namespace git