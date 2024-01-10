#include <BlindCodeReview/git.hpp>

namespace git {
    int Merge(const std::filesystem::path& local_path_to_repo) {
        struct help_stuff::MergeOptions opts{};
        git_index* index = nullptr;
        git_repository* repo = nullptr;
        git_merge_analysis_t analysis;
        git_merge_preference_t preference;

        int error = git_repository_open(&repo, local_path_to_repo.c_str());
        if (error != 0) {
            CleanUp(repo);
            return Error(git_error_last(), error, local_path_to_repo.c_str());
        }

        help_stuff::MergeOptionsInit(&opts);
        help_stuff::OptsAddRefish(&opts, "refs/remotes/origin/main");

        if (git_repository_state(repo) != GIT_REPOSITORY_STATE_NONE) {
            std::cerr << "repository is in unexpected state " << git_repository_state(repo) << std::endl;
        }

        error = help_stuff::ResolveHeads(repo, &opts);
        if (error != 0) {
            CleanUp(repo, index);
            free((char**)opts.heads);
            return Error(git_error_last(), error, local_path_to_repo.c_str());
        }

        error = git_merge_analysis(&analysis, &preference, repo
                , (const git_annotated_commit**)opts.annotated
                , opts.annotated_count);
        if (error != 0) {
            std::cout << git_error_last()->message << std::endl;
            CleanUp(repo, index);
            free((char**)opts.heads);
            free(opts.annotated);
            return Error(git_error_last(), error, local_path_to_repo.c_str());
        }

        if (analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) {
            std::cout << "Already up-to-date\n";
            return 0;
        } else if (analysis & GIT_MERGE_ANALYSIS_UNBORN ||
                   (analysis & GIT_MERGE_ANALYSIS_FASTFORWARD &&
                    !(preference & GIT_MERGE_PREFERENCE_NO_FASTFORWARD))) {
            const git_oid* target_oid;
            if (analysis & GIT_MERGE_ANALYSIS_UNBORN) {
                std::cout << "Unborn\n";
            } else {
                std::cout << "Fast-forward\n";
            }

            /* Since this is a fast-forward, there can be only one merge head */
            target_oid = git_annotated_commit_id(opts.annotated[0]);
            assert(opts.annotated_count == 1);

            return help_stuff::PerformFastforward(repo, target_oid, (analysis & GIT_MERGE_ANALYSIS_UNBORN));
        } else if (analysis & GIT_MERGE_ANALYSIS_NORMAL) {
            git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;
            git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

            merge_opts.flags = 0;
            merge_opts.file_flags = GIT_MERGE_FILE_STYLE_DIFF3;

            checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE|GIT_CHECKOUT_ALLOW_CONFLICTS;

            if (preference & GIT_MERGE_PREFERENCE_FASTFORWARD_ONLY) {
                std::cout << "Fast-forward is preferred, but only a merge is possible\n";
                return -1;
            }

            error = git_merge(repo,
                              (const git_annotated_commit **)opts.annotated, opts.annotated_count,
                              &merge_opts, &checkout_opts);
            if (error != 0) {
                CleanUp(repo, index);
                return Error(git_error_last(), error, local_path_to_repo.c_str());
            }
        }

        if (git_repository_index(&index, repo) != 0) {
            CleanUp(repo, index);
            return Error(git_error_last(), error, local_path_to_repo.c_str());
        } else {
            help_stuff::CreateMergeCommit(repo, index, &opts);
            std::cout << "Merge made\n";
        }

        return 0;
    }

} // namespace git