#include <BlindCodeReview/git.hpp>

namespace git {
    int Fetch(const std::filesystem::path& local_path_to_repo) {
        git_repository* repo = nullptr;
        git_remote* remote = nullptr;
        git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;

        int error = git_repository_open(&repo, local_path_to_repo.string().c_str());
        if (error != 0) {
            CleanUp( repo, nullptr, nullptr, nullptr
                    , nullptr, nullptr, nullptr, remote);
            return error;
        }

        error = git_remote_lookup(&remote, repo, "origin");
        if (error != 0) {
            CleanUp( repo, nullptr, nullptr, nullptr
                    , nullptr, nullptr, nullptr, remote);
            return error;
        }

        fetch_opts.callbacks.update_tips = &help_stuff::UpdateCallback;
        fetch_opts.callbacks.credentials = CredentialsCallback;

        if (git_remote_fetch(remote, nullptr, &fetch_opts, "fetch") != 0) {
            CleanUp( repo, nullptr, nullptr, nullptr
                    , nullptr, nullptr, nullptr, remote);
            return error;
        }

        CleanUp( repo, nullptr, nullptr, nullptr
                , nullptr, nullptr, nullptr, remote);
        return 0;
    }
}