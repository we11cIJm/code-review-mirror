#ifndef BLINDCODEREVIEW_GIT_HPP
#define BLINDCODEREVIEW_GIT_HPP

#include <iostream>
#include <string>
#include <git2.h>
#include <git2/common.h>
#include <git2/errors.h>
#include <fstream>
#include <filesystem>
#include <map>
#include <vector>

namespace git {

    int Error(const git_error* err, int error);

    std::string GetRepoName(const std::string& url);

    int cred_acquire_cb( git_cred** out, const char* url
                       , const char* username_from_url
                       , unsigned int allowed_types, void* payload);


    void CloneByFile(std::string& filename);
    int Clone(const char* url, const char* path);

    int Add(const char* path_to_repo);
    int Commit(const char* path_to_repo, const char* message);

} // namespace git

namespace parse {
    extern std::map<std::string, std::string> arguments;
    extern std::string filename;
    extern std::string path_to_file; // local path to file with urls
    extern std::string username;
    extern std::string pat;
//    extern std::string location; // where to clone repositories

    bool Parse(int argc, char** argv);
} // namespace parse

#endif //BLINDCODEREVIEW_GIT_HPP