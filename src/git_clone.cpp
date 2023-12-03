#include <BlindCodeReview/git.hpp>

namespace git {

    // Function to clone a repository
    int Clone(const char* url, const char* path) {

        git_repository* cloned_repo = nullptr;
        git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
        git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
        int error;

        // Set up options
        checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
        clone_opts.checkout_opts = checkout_opts;
        clone_opts.fetch_opts.callbacks.credentials = cred_acquire_cb;

        // Do the clone
        error = git_clone(&cloned_repo, url, path, &clone_opts);
        if (error != 0) {
            const git_error* err = git_error_last();
            if (err) {
                std::cout << "ERROR " << err->klass << ": " << err->message << std::endl;
            } else {
                std::cout << "ERROR " << error << ": no detailed info\n";
            }
        } else if (cloned_repo) {
            git_repository_free(cloned_repo);
        }

        return error;
    }

    // Function to clone repositories from a file
    void CloneByFile(std::string& filename) {
        std::ifstream input(filename);
        if (!input.is_open()) {
            throw std::invalid_argument("Cannot open file " + filename);
        }

        std::string url;
        while (input >> url) {
            // Call the clone function
            int result = Clone(url.c_str(), GetRepoName(url).c_str());
            if (result) {
                std::cout << "Cannot clone " << url << std::endl;
            } else {
                std::cout << "Cloned " << url << std::endl;
            }
        }
    }

} // namespace git