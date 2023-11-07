#include <BlindCodeReview/git.hpp>

namespace git {
    GitObject::GitObject(const std::string &repo_url)
            : repo_url_(repo_url), repo_(nullptr), index_(nullptr), repo_local_path_(std::filesystem::current_path().string() + "/")
            , signature_(nullptr), tree_(nullptr), commit_(nullptr), remote_(nullptr)
            , clone_opts_(GIT_CLONE_OPTIONS_INIT)
            , fetch_opts_(GIT_FETCH_OPTIONS_INIT) {
//        clone_opts_.checkout_branch = "refs/heads/main"; // switch to main branch
        repo_name_ = GetRepositoryName(); // get the name of the repository
        repo_local_path_ += repo_name_; // add name of the repository to local path
    }

    GitObject::~GitObject() {
        git_repository_free(repo_);
    }

    git_repository* GitObject::GetRepository() const {
        return repo_;
    }

} // namespace git
