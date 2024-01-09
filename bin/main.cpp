#include <BlindCodeReview/git.hpp>

using namespace git;

const char* url = "https://github.com/we11cIJm/testrepo.git";

int main(int argc, char** argv) {
    git_libgit2_init();

    std::string filename = "../urls.txt";
//    std::string clone = "--clone";
//    std::string push = "--push";
//    std::string push_l = "-p";
//    std::string pull = "--pull";

    AddCommitPush("./testrepo", "try to push with deleted remotely files");

//    if (argc < 2) {
//        std::cout << "Nothing is called\n";
//        git_libgit2_shutdown();
//        return 0;
//    }
//    if (argv[1] == push_l) {
//        Add("testrepo");
//        Commit("testrepo", "files for review");
//        Push("testrepo");
//    }
//    else if (argv[1] == clone) {
//        CloneByFile(filename, "/home/kolya/Desktop/coderepos");
//    }
//    else if (argv[1] == push) {
//        AddCommitPush(filename, "files for review");
//        std::cout << "Push is called\n";
//    }

//    else if (argv[1] == pull) {
//        PullByFile(filename);
//        Pull(url, "./testrepo");
//        std::cout << "Pull is called\n";
//    } else {
//        std::cout << "Other is called\n";
//    }
    git_libgit2_shutdown();

    return 0;
}