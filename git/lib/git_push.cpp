#include <BlindCodeReview/git.hpp>

namespace git {
    int AddCommitPush(const std::filesystem::path& local_path_to_repo, const std::string& message) {
        int error = Add(local_path_to_repo);
        if (error != 0) {
            return Error(git_error_last(), error, local_path_to_repo.parent_path(), local_path_to_repo.filename());
        }
        error = Commit(local_path_to_repo.c_str(), message.c_str());
        if (error != 0) {
            return Error(git_error_last(), error, local_path_to_repo.parent_path(), local_path_to_repo.filename());
        }
        error = Push(local_path_to_repo.c_str());
        if (error != 0) {
            return Error(git_error_last(), error, local_path_to_repo.parent_path(), local_path_to_repo.filename());
        }

        return 0;
    }

    int Push(const std::filesystem::path& local_path_to_repo) {
        git_repository* repo = nullptr;
        git_push_options options = GIT_PUSH_OPTIONS_INIT;
        git_remote* remote = nullptr;
        const char* refspec = "refs/heads/main:refs/heads/main";
        git_strarray refspecs = {
                const_cast<char**>(&refspec),
                1
        };

        int error = git_repository_open(&repo, local_path_to_repo.string().c_str());
        if (error != 0) {
            CleanUp(repo, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, remote, &refspecs);
            return error;
        }

        error = git_remote_lookup(&remote, repo, "origin");
        if (error != 0) {
            CleanUp(repo, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, remote, &refspecs);
            return error;
        }

        error = git_push_options_init(&options, GIT_PUSH_OPTIONS_VERSION);
        if (error != 0) {
            CleanUp(repo, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, remote, &refspecs);
            return error;
        }

        options.callbacks.credentials = CredentialsCallback;

        error = git_remote_push(remote, &refspecs, &options);
        if (error != 0) {
            CleanUp(repo, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, remote);
            return error;
        }

        return 0;
    }

    void PushAll(const std::filesystem::path& path_to_all_repos, const std::vector<std::string>& urls) {
        std::string message = "files for review has been sent";
        for (auto& url : urls) {
            std::filesystem::path full_path = path_to_all_repos / "repos" / static_cast<std::filesystem::path>(GetRepoName(url));
            AddCommitPush(full_path.string(), message);
        }
    }

} // namespace git