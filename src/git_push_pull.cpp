#include <BlindCodeReview/git.hpp>

namespace git {
    void GitObject::Push() {
        int error = git_remote_lookup(&remote_, repo_, "origin");
        if (error < 0) {
            const git_error* err = giterr_last();
            std::cout << "Error " << error << "/" << err->klass << ": " << err->message << std::endl;
        }

        git_strarray refspecs;
        refspecs.count = 1;
        refspecs.strings = (char**) malloc(sizeof(char*));
        refspecs.strings[0] = (char*) malloc(sizeof(char) * 100);
        strcpy(refspecs.strings[0], "refs/heads/main:refs/heads/main");

        error = git_remote_push(remote_, &refspecs, nullptr);
        if (error < 0) {
            const git_error* err = giterr_last();
            std::cout << "Error " << error << "/" << err->klass << ": " << err->message << std::endl;
        }
    }

    // TODO: fix this function
    void GitObject::Pull() {
        int error = git_remote_lookup(&remote_, repo_, "origin");
        if (error < 0) {
            const git_error* err = giterr_last();
            std::cout << "Error " << error << "/" << err->klass << ": " << err->message << std::endl;
        }

//        fetch_opts_.callbacks.credentials = [](const char* url, const char* username_from_url, unsigned int allowed_types, void* payload) {
//            git_cred* cred;
//            int error = git_cred_ssh_key_new(&cred, username_from_url, "/home/username/.ssh/id_rsa.pub", "/home/username/.ssh/id_rsa", "");
//            return error == 0 ? cred : nullptr;
//        };

        error = git_remote_fetch(remote_, nullptr, &fetch_opts_, nullptr);
        if (error < 0) {
            const git_error* err = giterr_last();
            std::cout << "Error " << error << "/" << err->klass << ": " << err->message << std::endl;
        }
    }

} // namespace git