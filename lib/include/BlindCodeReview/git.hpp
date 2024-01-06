#ifndef BLINDCODEREVIEW_GIT_HPP
#define BLINDCODEREVIEW_GIT_HPP

#include <iostream>
#include <string>
#include <git2.h>
#include <fstream>
#include <array>
#include <chrono>
#include <stdexcept>
#include <filesystem>

namespace git {

//    int Error(const git_error* err, int error);
    int Error(const git_error* err, int error, const std::string& local_path);

    void CleanUp( git_repository* repo = nullptr, git_index* index = nullptr
            , git_tree* tree = nullptr, git_object* object = nullptr
            , git_reference* local_ref = nullptr, git_reference* remote_ref = nullptr
            , git_signature* signature = nullptr, git_remote* remote = nullptr
            , git_strarray* arr = nullptr, git_commit* local_commit = nullptr
            , git_commit* remote_commit = nullptr
            , git_annotated_commit* annotated_commit = nullptr);

    std::string GetRepoName(const std::string& url);

    int credentials_callback( git_cred** out, const char* url
                       , const char* username_from_url
                       , unsigned int allowed_types, void* payload);

    int Clone(const char* url, const std::filesystem::path& local_path);
    int Add(const std::filesystem::path& local_path_to_repo);

    int Commit(const std::filesystem::path& local_path_to_repo, const char* message);
    int Push(const std::filesystem::path& local_path_to_repo);
    int AddCommitPush(const std::string& local_path_to_repo, const std::string& message);
    int Pull(const std::filesystem::path& local_path_to_repo);
    int Merge(const char* local_path_to_repo, const char* fetched_commit_hash);

    void CloneByFile(const std::string& path_to_urls_file, const std::string& local_path = ".");
    void PullByFile(std::string& filename);
    void PushByFile(std::string& path_to_file);

    static int32_t total_repos_count = 0, current_repo_pos = 0;

    void PrintProgressBar(const int& current_repo_pos, const int& total_repos_count);

} // namespace git


#endif //BLINDCODEREVIEW_GIT_HPP