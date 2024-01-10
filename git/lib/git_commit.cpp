#include <BlindCodeReview/git.hpp>

namespace git {

    int Commit(const std::filesystem::path& local_path_to_repo, const char* message) {
        git_oid commit_oid, tree_id;
        git_repository* repo = nullptr;
        git_index* index = nullptr;
        git_tree* tree = nullptr;
        git_object* parent = nullptr;
        git_reference* ref = nullptr;
        git_signature* signature = nullptr;

        int error = git_repository_open(&repo, local_path_to_repo.c_str());
        if (error != 0) {
            CleanUp(repo, index, tree, parent, ref, nullptr, signature);
            return Error(git_error_last(), error, local_path_to_repo);
        }

        error = git_revparse_ext(&parent, &ref, repo, "HEAD");
        if (error == GIT_ENOTFOUND) {
            std::cout << "HEAD not found. Creating first commit\n";
        } else if (error != 0) {
            CleanUp(repo, index, tree, parent, ref, nullptr, signature);
            return Error(git_error_last(), error, local_path_to_repo);
        }

        error = git_repository_index(&index, repo);
        if (error != 0) {
            CleanUp(repo, index, tree, parent, ref, nullptr, signature);
            return Error(git_error_last(), error, local_path_to_repo);
        }

        git_index_write_tree(&tree_id, index);
        git_index_write(index);

        error = git_tree_lookup(&tree, repo, &tree_id);
        if (error != 0) {
            CleanUp(repo, index, tree, parent, ref, nullptr, signature);
            return Error(git_error_last(), error, local_path_to_repo);
        }

        error = git_signature_default(&signature, repo);
        if (error != 0) {
            CleanUp(repo, index, tree, parent, ref, nullptr, signature);
            return Error(git_error_last(), error, local_path_to_repo);
        }

        error = git_commit_create_v(
                &commit_oid,
                repo,
                "HEAD",
                signature,
                signature,
                nullptr,
                message,
                tree,
                parent ? 1 : 0, parent
        );
        if (error != 0) {
            CleanUp(repo, index, tree, parent, ref, nullptr, signature);
            return Error(git_error_last(), error, local_path_to_repo);
        }

        CleanUp(repo, index, tree, parent, ref, nullptr, signature);

        return 0;
    }

} // namespace git