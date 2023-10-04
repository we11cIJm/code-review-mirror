#include <iostream>
#include <vector>
#include <git2.h>
#include <git2/index.h>
#include <git2/repository.h>
#include <fstream>
#include <map>

std::string GetRepositoryName(const std::string& url);

namespace git_processing {
    void Add(git_index* index, git_repository* repo, const char* path_to_file);
    void Commit(git_signature* author, git_index* index, git_repository* repo,
            const char* name, const char* email, const char* message);
    void Push();
    void Clone(const std::vector<std::string>& urls, const std::string& local_path);
} // namespace git_processing


namespace parse {
    std::map<std::string, std::string> arguments;
    std::string filename;
    std::string path; // local path

    bool Parse(int argc, char** argv);
} // namespace parse


/*void git_processing::Add(git_index* index, git_repository* repo, const char* path_to_file) {
    git_repository_index(&index, repo); // get access to index of repo
    git_index_add_bypath(index, path_to_file);
    git_index_write(index);

    git_index_free(index);
}

void git_processing::Commit(git_signature* author, git_index* index, git_repository* repo,
            const char* name, const char* email, const char* message) {
    git_signature_new(&author, name, email, time(nullptr), 0);

    git_signature* committer = author;

    git_oid tree_id; 
    git_tree* tree;
    git_index_write_tree(&tree_id, index);
    git_tree_lookup(&tree, repo, &tree_id);

    git_commit* parent_commit = nullptr;
    git_commit_lookup(&parent_commit, repo, "SHA of parent-commit");

    git_commit_create_v(
        nullptr, // id of created commit
        repo,
        "HEAD", // name of the branch, where commit would be added
        author,
        committer,
        nullptr, // id of parent-commit (nullptr for the 1st one)
        message,
        tree,
        parent_commit != nullptr ? 1 : 0, // count of parrent-commit
        parent_commit != nullptr ? &parent_commit : nullptr,
        &new_commit // ??? new commit
    );

    git_commit_free(parent_commit);
    git_tree_free(tree);
    git_signature_free(author);

}

void git_processing::Push() {
    git_remote* remote;
    git_remote_lookup(&remote, repo, "name of remote server");

    git_push* push;
    git_push_new(&push, remote);

    git_push_add_refspec(push, "refs/heads/your-branch:refs/heads/your-branch");

    git_push_finish(push);

    git_push_free(push);
    git_remote_free(remote);

}*/

void git_processing::Clone(const std::vector<std::string>& urls, const std::string& local_path) {
    git_libgit2_init();
    
    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    clone_opts.checkout_branch = "refs/heads/main";

    std::vector<git_repository*> repo(urls.size(), nullptr); // object of repository
    for (size_t i = 0; i < urls.size(); ++i) {
        std::string repo_name = GetRepositoryName(urls[i]);
        std::string repo_path = local_path + '/' + repo_name;

        int result = git_clone(&(repo[i]), urls[i].c_str(), repo_path.c_str(), nullptr);
        if (result != 0) {
            std::cerr << "Failed to clone repository: " << urls[i] << std::endl;
            
            /*const git_error* gitError = giterr_last();
            if (gitError) {
                std::cerr << "Failed to clone repository: " << urls[i] << std::endl;
                std::cerr << "Error message: " << gitError->message << std::endl;
            } else {
                std::cerr << "Failed to clone repository: " << urls[i] << std::endl;
                std::cerr << "Unknown error." << std::endl;
            }*/
        } else {
            std::cout << "Repository " << repo_name << " cloned to " << repo_path << std::endl;
        }
    }
    for (size_t i = 0; i < repo.size(); ++i) {
        if (repo[i]) {
            git_repository_free(repo[i]);
        }
    }
    git_libgit2_shutdown();

}

bool parse::Parse(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        arguments[argv[i]] = (i + 1 < argc) ? argv[i + 1] : "";
    }
    arguments.count("--filename") > 0 ?
        parse::filename = arguments["--filename"] : throw std::invalid_argument("No filename");
    arguments.count("--path") > 0 ?
        parse::path = arguments["--path"] : throw std::invalid_argument("No path indicated");
    return true;
}

std::string GetRepositoryName(const std::string& url) {
    size_t lastSlashPos = url.find_last_of('/'); // find last '/' character in URL

    std::string repositoryName;
    if (lastSlashPos != std::string::npos) {
        repositoryName = url.substr(lastSlashPos + 1); // gets name of repo
    } else {
        repositoryName = url;
    }

    size_t gitExtensionPos = repositoryName.find(".git");
    if (gitExtensionPos != std::string::npos && gitExtensionPos == repositoryName.length() - 4) {
        repositoryName = repositoryName.substr(0, gitExtensionPos); //check aviability of .git
    }

    return repositoryName;
}

int main(int argc, char** argv) {
    /*std::string filename; // file with urls
    std::cout << "filename: ";
    std::cin >> filename;
    if (!s.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return 1;
    }
     
    std::string repo_url, local_path;
    std::cout << "local_path: ";
    std::cin >> local_path;
    */
    parse::Parse(argc, argv);

    std::ifstream s{parse::filename}; 
    std::vector<std::string> urls;
    std::string repo_url;
    while (s >> repo_url) {
        urls.push_back(repo_url);
    }
    s.close();

    git_processing::Clone(urls, parse::path); 

return 0;
}
