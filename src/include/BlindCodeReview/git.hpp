#ifndef BLINDCODEREVIEW_GIT_HPP
#define BLINDCODEREVIEW_GIT_HPP

#include <iostream>
#include <vector>
#include <git2.h>
#include <git2/index.h>
#include <git2/repository.h>
#include <fstream>
#include <map>
#include <filesystem>
#include <cstring>

namespace git {

    class GitObject {
    private:
        // TODO: change repo_local_path_ to std::filesystem::path if needed
        std::string repo_local_path_; // local path where repository is stored
        std::string repo_url_; // url of repository
        git_repository* repo_; // object of the repository
        std::string repo_name_; // name of the repository
        git_clone_options clone_opts_;
        git_index* index_; // index of the repository
        git_signature* signature_; // signature of the repository
        git_oid tree_id_; // tree id of the repository
        git_tree* tree_; // tree of the repository
        git_oid commit_id_; // commit id of the repository
        git_commit* commit_; // commit of the repository
        git_remote* remote_; // remote of the repository
        git_fetch_options fetch_opts_; // fetch options of the repository

//        void GetInfo(const std::string& str); // get everything that defines GitObject

    public:
        explicit GitObject(const std::string& repo_url); // create GitObject from url from .txt file
        ~GitObject();

        void Add (const std::string& filename); // add file to repository (if filename == "*" (by default) then add all files)
        void Add (const std::vector<std::string>& filenames); // add files to repository

        void Commit (const std::string& message); // commit all changes
//        void Commit (const std::string& message, const std::string& filename); // commit changes for one file
//        void Commit (const std::string& message, const std::vector<std::string>& filenames); // commit changes for many files

        void Push (); // push local changes to origin
        void Pull (); // pull changes from origin to local

        friend bool CloneByUrl (const std::string& url);
        std::string GetRepositoryName() const;
        git_repository* GetRepository() const;
    };

    static std::vector<GitObject> git_objects; // vector of all GitObjects

    void CloneByFile (const std::string& filename, const std::string& urls_file); // clone all repositories from file to local path
    bool CloneByUrl (const std::string& url); // clone repository from origin to local path

    git_repository* OpenExistingRepo(const std::string& localPath); // check if repository exists locally
} // namespace git

namespace parse {
    extern std::map<std::string, std::string> arguments;
    extern std::string filename;
    extern std::string path_to_file; // local path to file with urls
//    extern std::string location; // where to clone repositories

    bool Parse(int argc, char** argv);
} // namespace parse

#endif //BLINDCODEREVIEW_GIT_HPP