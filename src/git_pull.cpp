#include <BlindCodeReview/git.hpp>

namespace git {

    int Pull(const char* path_to_repo) {
        git_repository* repo = nullptr;
        int error = git_repository_open(&repo, path_to_repo);
        if (error < 0) {
            const git_error* e = git_error_last();
            std::cout << "Error opening repository: " << e->message << std::endl;
            return error;
        }

        git_remote* remote = nullptr;
        error = git_remote_lookup(&remote, repo, "origin");
        if (error < 0) {
            const git_error* e = git_error_last();
            std::cout << "Error looking up remote: " << e->message << std::endl;
            git_repository_free(repo);
            return error;
        }

        git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
        fetch_opts.callbacks.credentials = credentials_callback;
        error = git_remote_fetch(remote, nullptr, &fetch_opts, nullptr);
        if (error < 0) {
            const git_error* e = git_error_last();
            std::cout << "Error fetching remote: " << e->message << std::endl;
            git_remote_free(remote);
            git_repository_free(repo);
            return error;
        }

        git_reference* head_ref = nullptr;
        error = git_repository_head(&head_ref, repo);
        if (error < 0) {
            const git_error* e = git_error_last();
            std::cout << "Error getting head reference: " << e->message << std::endl;
            git_remote_free(remote);
            git_repository_free(repo);
            return error;
        }

        git_reference* remote_branch_ref = nullptr;
        error = git_branch_lookup(&remote_branch_ref, repo, "origin/main", GIT_BRANCH_REMOTE);
        if (error < 0) {
            const git_error* e = git_error_last();
            std::cout << "Error looking up remote branch: " << e->message << std::endl;
            git_reference_free(head_ref);
            git_remote_free(remote);
            git_repository_free(repo);
            return error;
        }

        git_oid fetched_commit_oid = {0};
        error = git_reference_name_to_id(&fetched_commit_oid, repo, "FETCH_HEAD");
        if (error < 0) {
            const git_error* e = git_error_last();
            std::cout << "Error getting fetched commit OID: " << e->message << std::endl;
            git_reference_free(remote_branch_ref);
            git_reference_free(head_ref);
            git_remote_free(remote);
            git_repository_free(repo);
            return error;
        }

        git_commit* fetched_commit = nullptr;
        error = git_commit_lookup(&fetched_commit, repo, &fetched_commit_oid);
        if (error < 0) {
            const git_error* e = git_error_last();
            std::cout << "Error looking up fetched commit: " << e->message << std::endl;
            git_reference_free(remote_branch_ref);
            git_reference_free(head_ref);
            git_remote_free(remote);
            git_repository_free(repo);
            return error;
        }

        git_commit* local_commit = nullptr;
        error = git_commit_lookup(&local_commit, repo, git_reference_target(head_ref));
        if (error < 0) {
            const git_error* e = git_error_last();
            std::cout << "Error looking up local commit: " << e->message << std::endl;
            git_commit_free(fetched_commit);
            git_reference_free(remote_branch_ref);
            git_reference_free(head_ref);
            git_remote_free(remote);
            git_repository_free(repo);
            return error;
        }

        if (git_oid_equal(git_commit_id(fetched_commit), git_commit_id(local_commit))) {
            std::cout << "Already up-to-date" << std::endl;
            git_commit_free(fetched_commit);
            git_commit_free(local_commit);
            git_reference_free(remote_branch_ref);
            git_reference_free(head_ref);
            git_remote_free(remote);
            git_repository_free(repo);
            return 0;
        }

        git_merge_options merge_options = GIT_MERGE_OPTIONS_INIT;
        git_checkout_options checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
        checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE;

        git_index* index = nullptr;
        error = git_repository_index(&index, repo);
        if (error != 0) {
            return Error(git_error_last(), error);
        }

        // converting git_commit to git_annotated_commit
        git_annotated_commit* annotated_commit = nullptr;
        const git_oid* commit_oid = git_commit_id(fetched_commit);
        git_annotated_commit_lookup(&annotated_commit, repo, commit_oid);

        error = git_merge(repo, (const git_annotated_commit**)(&annotated_commit)/*(&fetched_commit)*/, 1, &merge_options, &checkout_options);
        if (error < 0) {
            const git_error* e = git_error_last();
            std::cout << "Error merging changes: " << e->message << std::endl;
            git_commit_free(fetched_commit);
            git_commit_free(local_commit);
            git_reference_free(remote_branch_ref);
            git_reference_free(head_ref);
            git_remote_free(remote);
            git_repository_free(repo);
            return error;
        }

        git_signature* signature;
        error = git_signature_default(&signature, repo);
        if (error != 0) {
            return Error(git_error_last(), error);
        }

        git_tree* tree = nullptr;
        git_oid tree_id;
        git_index_write_tree(&tree_id, index);
        git_index_write(index);

        error = git_tree_lookup(&tree, repo, &tree_id);
        if (error != 0) {
            return Error(git_error_last(), error);
        }

        git_oid fetched_commit_oid_ = *git_commit_id(fetched_commit);
        char fetched_commit_hash[GIT_OID_HEXSZ + 1];
        git_oid_tostr(fetched_commit_hash, sizeof(fetched_commit_hash), &fetched_commit_oid_);

        Merge(path_to_repo, fetched_commit_hash);

        Push(path_to_repo);

        git_repository_state_cleanup(repo);

        git_commit_free(fetched_commit);
        git_commit_free(local_commit);
        git_reference_free(remote_branch_ref);
        git_reference_free(head_ref);
        git_remote_free(remote);
        git_repository_free(repo);

        return 0;
    }

} // namespace git