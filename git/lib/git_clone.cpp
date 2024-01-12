#include <BlindCodeReview/git.hpp>

namespace git {

    int Clone(const char* url, const std::filesystem::path& work_dir) {
        git_repository* cloned_repo = nullptr;
        git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
        git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

        checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
        clone_opts.checkout_opts = checkout_opts;
        clone_opts.fetch_opts.callbacks.credentials = CredentialsCallback;
        const char* mbstr = nullptr;
#ifdef _WIN32
        char tmp[256] = "";
        wcstombs(tmp, work_dir.filename().c_str(), 256);
        mbstr = tmp;
#else
        mbstr = work_dir.string().c_str();
#endif
        int error = git_clone(&cloned_repo, url, mbstr, &clone_opts);
        if (error != 0) {
            CleanUp(cloned_repo);
            Error(git_error_last(), error, work_dir.parent_path(), GetRepoName(url));
        } else if (cloned_repo) {
            git_repository_free(cloned_repo);
        }

        return error;
    }

    void CloneByFile(const std::filesystem::path& path_to_urls_file, const std::filesystem::path& local_path /* = "." */) {
        std::ifstream input(path_to_urls_file);
        if (!input.is_open()) {
            throw std::invalid_argument(("Cannot open file " + path_to_urls_file.string()));
        }

        int32_t total_repos_count = 0, current_repo_pos = 0;
        std::string url;
        while (input >> url) {
            ++total_repos_count;
        }
        input.clear();
        input.seekg(0);

        PrintProgressBar(total_repos_count);

        while (input >> url) {
            std::filesystem::path work_dir =
                    std::filesystem::path(local_path) / "repos" / static_cast<std::filesystem::path>(GetRepoName(url));
            Clone(url.c_str(), work_dir);
            PrintProgressBar(total_repos_count, ++current_repo_pos);
        }
    }

} // namespace git