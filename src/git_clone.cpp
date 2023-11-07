#include <BlindCodeReview/git.hpp>

namespace git {
    std::string GitObject::GetRepositoryName() const {
        size_t lastSlashPos = repo_url_.find_last_of('/'); // find last '/' character in URL

        std::string repositoryName;
        repositoryName = (lastSlashPos != std::string::npos ? repo_url_.substr(lastSlashPos + 1) : repo_url_);

        size_t gitExtensionPos = repositoryName.find(".git");
        if (gitExtensionPos != std::string::npos && gitExtensionPos == repositoryName.length() - 4) {
            repositoryName = repositoryName.substr(0, gitExtensionPos); // check availability of .git
        }

        return repositoryName;
    }

    void CloneByFile(const std::string &filename, const std::string &urls_file) {
        std::string slash = (urls_file.back() == '/' ? "" : "/");
        std::ifstream input(urls_file + slash + filename);
        if (!input.is_open()) {
            throw std::invalid_argument("Cannot open file " + filename + " by path " + urls_file);
        }
        std::string url;
        while (input >> url) {
            if(CloneByUrl(url)) {
                std::cout << "Cloned " << url << std::endl;
            } else {
                std::cout << "Cannot clone " << url << std::endl;
            }
        }
        input.close();
    }

// TODO: check accuracy of this function and add exception handling
    bool CloneByUrl(const std::string &url) {
        GitObject git_object(url);
        git_objects.push_back(git_object);

        if (OpenExistingRepo(git_object.repo_local_path_) != nullptr) {
            std::cout << "Repository " << git_object.repo_name_ << " already exists locally" << std::endl;
            return false;
        }
        int error = git_clone(
                &git_object.repo_,
                git_object.repo_url_.c_str(),
                git_object.repo_local_path_.c_str(),
                &git_object.clone_opts_
        );
        if (error < 0) {
            const git_error* err = giterr_last();
            std::cout << "Error " << error << "/" << err->klass << ": " << err->message << std::endl;
            return false;
        }
        return true;
    }

    git_repository* OpenExistingRepo(const std::string& localPath) {
        git_repository* repo = nullptr;
        int error = git_repository_open(&repo, localPath.c_str());
        if (error < 0) {
            return nullptr;
        }
        return repo;
    }
} // namespace git