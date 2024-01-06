#include <BlindCodeReview/git.hpp>

namespace git {

    int Merge(const char* local_path_to_repo, const char* fetched_commit_hash) {
        git_repository* repo = nullptr;
        int error = git_repository_open(&repo, local_path_to_repo);
        if (error < 0) {
            std::cerr << "Error opening repository: " << giterr_last()->message << std::endl;
            git_libgit2_shutdown();
            return error;
        }

        git_reference* head_ref = nullptr;
        error = git_repository_head(&head_ref, repo);
        if (error < 0) {
            std::cerr << "Error getting head reference: " << giterr_last()->message << std::endl;
            git_repository_free(repo);
            git_libgit2_shutdown();
            return error;
        }

        git_oid local_commit_oid = *git_reference_target(head_ref);

        git_commit* fetched_commit = nullptr;
        git_oid fetched_commit_oid;
        git_oid_fromstr(&fetched_commit_oid, fetched_commit_hash); // Replace with your fetched commit hash
        error = git_commit_lookup(&fetched_commit, repo, &fetched_commit_oid);
        if (error < 0) {
            std::cerr << "Error looking up fetched commit: " << giterr_last()->message << std::endl;
            git_reference_free(head_ref);
            git_repository_free(repo);
            git_libgit2_shutdown(); // TODO: ???
            return error;
        }

        git_commit* commit = nullptr;
        git_commit_lookup(&commit, repo, &local_commit_oid);

        git_signature* signature;
        error = git_signature_default(&signature, repo);
        if (error != 0) {
            return Error(git_error_last(), error, static_cast<const std::string>(local_path_to_repo));
        }

        git_commit* parents[] = {commit, fetched_commit};
        git_oid merge_commit_oid;
        git_tree* tree = nullptr;
        git_commit_tree(&tree, fetched_commit);
        error = git_commit_create(
                &merge_commit_oid,
                repo,
                "HEAD",
                signature,
                signature,
                "UTF-8",
                "Merge commit",
                tree,
                2,
                (const git_commit**)parents
        );

        if (error < 0) {
            std::cerr << "Error creating merge commit: " << giterr_last()->message << std::endl;
        }

        git_commit_free(fetched_commit);
        git_reference_free(head_ref);
        git_repository_free(repo);

        return 0;
    }

} // namespace git