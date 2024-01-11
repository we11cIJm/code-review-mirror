#include <BlindCodeReview/git.hpp>

namespace git {
    int Fetch(const std::filesystem::path& local_path_to_repo) {
        git_repository* repo = nullptr;
        git_remote* remote = nullptr;
        const git_indexer_progress* stats;
        git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;

        int error = git_repository_open(&repo, local_path_to_repo.string().c_str());
        if (error != 0) {
            CleanUp( repo, nullptr, nullptr, nullptr
                    , nullptr, nullptr, nullptr, remote);
            return Error(git_error_last(), error, local_path_to_repo.c_str());
        }

        error = git_remote_lookup(&remote, repo, "origin");
        if (error != 0) {
            CleanUp( repo, nullptr, nullptr, nullptr
                    , nullptr, nullptr, nullptr, remote);
            return Error(git_error_last(), error, local_path_to_repo.c_str());
        }

        fetch_opts.callbacks.update_tips = &help_stuff::UpdateCallback;
        fetch_opts.callbacks.sideband_progress = &help_stuff::ProgressCallback;
        fetch_opts.callbacks.transfer_progress = help_stuff::TransferProgressCallback;
        fetch_opts.callbacks.credentials = CredentialsCallback;

        if (git_remote_fetch(remote, nullptr, &fetch_opts, "fetch") != 0) {
            CleanUp( repo, nullptr, nullptr, nullptr
                    , nullptr, nullptr, nullptr, remote);
            return Error(git_error_last(), error, local_path_to_repo.c_str());
        }

        CleanUp( repo, nullptr, nullptr, nullptr
                , nullptr, nullptr, nullptr, remote);
        return 0;
    }
}