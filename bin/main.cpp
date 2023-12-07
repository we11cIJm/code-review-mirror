#include <BlindCodeReview/git.hpp>

using namespace git;

std::string url = "https://github.com/we11cIJm/testrepo.git";

int main(int argc, char** argv) {
    git_libgit2_init();
    std::string filename = "urls.txt";
    std::string clone = "--clone";
    std::string push = "--push";
    std::string pull = "--pull";

    if (argc < 2) {
        std::cout << "Nothing is called\n";
        git_libgit2_shutdown();
        return 0;
    }
    if (argv[1] == clone) {
        Clone(url.c_str(), GetRepoName(url).c_str());
        std::cout << "Clone is called\n";
    }
    else if (argv[1] == push) {
        Add(GetRepoName(url).c_str());
        Commit(GetRepoName(url).c_str(), "Test commit");
        Push(GetRepoName(url).c_str());
        std::cout << "Push is called\n";
    }

    else if (argv[1] == pull) {
        Pull(GetRepoName(url).c_str());
        std::cout << "Pull is called\n";
    } else {
        std::cout << "Other is called\n";
    }

    git_libgit2_shutdown();
    return 0;
}