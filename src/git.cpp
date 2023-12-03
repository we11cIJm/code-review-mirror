#include <BlindCodeReview/git.hpp>

namespace git {

    int Error(const git_error* err, int error) {
        if (err) {
            std::cout << "ERROR " << err->klass << ": " << err->message << std::endl;
        } else {
            std::cout << "ERROR " << error << ": no detailed info\n";
        }
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
    int cred_acquire_cb(
            git_cred** out,
            const char* url,
            const char* username_from_url,
            unsigned int allowed_types,
            void* payload) {
        // Create credentials and assign them to `*out`
        git_cred_userpass_plaintext_new(out, "we11cIJm", "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");

        return 0;
    }



} // namespace git
