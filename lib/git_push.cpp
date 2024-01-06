#include <BlindCodeReview/git.hpp>

namespace git {

    int Push(const std::filesystem::path& local_path_to_repo) {
        git_repository* repo = nullptr;
        git_push_options options = GIT_PUSH_OPTIONS_INIT;
        git_remote* remote = nullptr;
        const char* refspec = "+refs/heads/main:refs/heads/main"; // force push
        git_strarray refspecs = {
                const_cast<char**>(&refspec),
                1
        };

        int error = git_repository_open(&repo, local_path_to_repo.c_str());
        if (error != 0) {
            CleanUp(repo, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, remote, &refspecs);
            return Error(git_error_last(), error, static_cast<const std::string>(local_path_to_repo));
        }

        error = git_remote_lookup(&remote, repo, "origin");
        if (error != 0) {
            CleanUp(repo, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, remote, &refspecs);
            return Error(git_error_last(), error, static_cast<const std::string>(local_path_to_repo));
        }

        error = git_push_options_init(&options, GIT_PUSH_OPTIONS_VERSION);
        if (error != 0) {
            CleanUp(repo, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, remote, &refspecs);
            return Error(git_error_last(), error, static_cast<const std::string>(local_path_to_repo));
        }

        options.callbacks.credentials = credentials_callback;

        error = git_remote_push(remote, &refspecs, &options);
        if (error != 0) {
            CleanUp(repo, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, remote, &refspecs);
            return Error(git_error_last(), error, static_cast<const std::string>(local_path_to_repo));
        }

        return 0;
    }

    void PushByFile(std::string& path_to_file) {
        std::ifstream input(path_to_file);
        if (!input.is_open()) {
            throw std::invalid_argument("Cannot open file " + path_to_file);
        }

        std::string repo_url;
        std::string message = "files for review";
        while (input >> repo_url) {
            std::filesystem::path full_path = "repos" / static_cast<std::filesystem::path>(GetRepoName(repo_url));
            int result = AddCommitPush(full_path, message);

            PrintProgressBar(++current_repo_pos, total_repos_count);
//            if (result) {
//                std::cout << "Cannot push " << repo_url << std::endl;
//            } else {
//                std::cout << "Pushed " << repo_url << std::endl;
//            }
        }
        current_repo_pos = 0;
    }

} // namespace git