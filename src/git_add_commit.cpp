#include <BlindCodeReview/git.hpp>

namespace git {
    // TODO: think about how to implement Add, Commit and Push functions
    void GitObject::Add(const std::string &filename = "*") {

        int error = filename != "*" ?
            git_index_add_bypath(index_, filename.c_str())
            : git_index_add_all(index_, nullptr, 0, nullptr, nullptr);
        if (error < 0) {
            const git_error *err = giterr_last();
            std::cout << "Error " << error << "/" << err->klass << ": " << err->message << std::endl;
        }
    }

    void GitObject::Add(const std::vector<std::string> &filenames) {
        for (const auto &filename: filenames) {
            Add(filename);
        }
    }

    void GitObject::Commit(const std::string& message) {
        int error = git_index_write(index_);
        if (error < 0) {
            const git_error* err = giterr_last();
            std::cout << "Error " << error << "/" << err->klass << ": " << err->message << std::endl;
        }

        error = git_index_write_tree(&tree_id_, index_);
        if (error < 0) {
            const git_error* err = giterr_last();
            std::cout << "Error " << error << "/" << err->klass << ": " << err->message << std::endl;
        }

        error = git_tree_lookup(&tree_, repo_, &tree_id_);
        if (error < 0) {
            const git_error* err = giterr_last();
            std::cout << "Error " << error << "/" << err->klass << ": " << err->message << std::endl;
        }

        error = git_commit_create_v(
                &commit_id_,
                repo_,
                "HEAD",
                signature_,
                signature_,
                nullptr,
                message.c_str(),
                tree_,
                0
        );
        if (error < 0) {
            const git_error* err = giterr_last();
            std::cout << "Error " << error << "/" << err->klass << ": " << err->message << std::endl;
        }
    }

} // namespace git