#include <iostream>
//#include <BlindCodeReview/git.hpp>
#include "../src/include/BlindCodeReview/git.hpp"

//using namespace git;
//using namespace parse;

int main(int argc, char** argv) {
    git_libgit2_init();
    parse::Parse(argc, argv);

    git::CloneByFile(parse::filename, parse::path_to_file);
//    git::git_objects[0].Add();
    git_libgit2_shutdown();
}