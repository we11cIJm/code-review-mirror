#include <BlindCodeReview/git.hpp>

namespace git {
    int Error(const git_error* err, int error, const std::filesystem::path& local_path) {
        std::ofstream output(local_path / "errors.txt", std::ios_base::app);
        if (err) {
            output << local_path << ": ERROR " << err->klass << ": " << err->message << std::endl;
        } else {
            output << local_path << ": ERROR " << error << ": no detailed info\n";
        }
        output << std::endl;
        output.close();
        return error;
    }

    std::string GetRepoName(const std::string& url) {
        size_t lastSlashPos = url.find_last_of('/'); // find last '/' character in URL
        std::string repositoryName = lastSlashPos != std::string::npos ? url.substr(lastSlashPos + 1) : url;
        size_t gitExtensionPos = repositoryName.find(".git");
        if (gitExtensionPos != std::string::npos && gitExtensionPos == repositoryName.length() - 4) {
            repositoryName = repositoryName.substr(0, gitExtensionPos);
        }

        return repositoryName;
    }

    // Function to acquire credentials
    int CredentialsCallback(
            git_cred** out,
            const char* url,
            const char* username_from_url,
            unsigned int allowed_types,
            void* payload) {
        // Create credentials and assign them to `*out`
        git_cred_userpass_plaintext_new(out, "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", "x-oauth-basic");

        return 0;
    }

    void PrintProgressBar() {
        const int barWidth = 50;
        int pos = barWidth * current_repo_pos / total_repos_count;

        std::cout << "[";
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos)
                std::cout << "=";
            else if (i == pos)
                std::cout << ">";
            else
                std::cout << " ";
        }
        int percentage = static_cast<double>(current_repo_pos) / total_repos_count * 100;
        std::cout << "] " << percentage << " %" << std::flush;
        std::cout << '\r';
    }

    int AddCommitPush(const std::string& local_path_to_repo, const std::string& message) {
        int result = 0;
        result += Add(local_path_to_repo.c_str());
        result += Commit(local_path_to_repo.c_str(), message.c_str());
        result += Push(local_path_to_repo.c_str());

        return result;
    }

    void CleanUp( git_repository* repo, git_index* index
            , git_tree* tree, git_object* object
            , git_reference* local_ref, git_reference* remote_ref
            , git_signature* signature, git_remote* remote
            , git_strarray* arr, git_commit* local_commit
            , git_commit* remote_commit
            , git_annotated_commit* local_annotated_commit
            , git_annotated_commit* remote_annotated_commit) {
        git_repository_free(repo);
        git_index_free(index);
        git_tree_free(tree);
        git_object_free(object);
        git_reference_free(local_ref);
        git_reference_free(remote_ref);
        git_signature_free(signature);
        git_remote_free(remote);
        git_strarray_free(arr);
        git_commit_free(local_commit);
        git_commit_free(remote_commit);
        git_annotated_commit_free(local_annotated_commit);
        git_annotated_commit_free(remote_annotated_commit);
    }

} // namespace git
