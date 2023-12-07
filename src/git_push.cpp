#include <BlindCodeReview/git.hpp>

namespace git {

    int Push(const char* path_to_repo) {
        git_repository* repo = nullptr;
        git_push_options options = GIT_PUSH_OPTIONS_INIT;
        git_remote* remote = nullptr;
        char* refspec = "refs/heads/main";
        const git_strarray refspecs = {
                &refspec,
                1
        };

        int error = git_repository_open(&repo, path_to_repo);
        if (error != 0) {
            return Error(git_error_last(), error);
        }

        error = git_remote_lookup(&remote, repo, "origin");
        if (error != 0) {
            return Error(git_error_last(), error);
        }

        error = git_push_options_init(&options, GIT_PUSH_OPTIONS_VERSION);
        if (error != 0) {
            return Error(git_error_last(), error);
        }

        options.callbacks.credentials = credentials_callback;

        error = git_remote_push(remote, &refspecs, &options);
        if (error != 0) {
            return Error(git_error_last(), error);
        }

        return 0;
    }

} // namespace git