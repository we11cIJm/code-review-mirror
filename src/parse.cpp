#include <BlindCodeReview/git.hpp>

namespace parse {
    std::map <std::string, std::string> arguments;
    std::string filename;
    std::string path_to_file; // local path

    bool Parse(int argc, char **argv) {
        for (int i = 1; i < argc; ++i) {
            arguments[argv[i]] = (i + 1 < argc) ? argv[i + 1] : "";
        }
        if (arguments.count("--help")) {
            arguments.count("--help") > 0 ?
            std::cout
                    << "Usage: ./git --filename <filename> --path <path> --add <filename> --commit <message> --push --pull --clone <url> --help"
                    << std::endl : throw std::invalid_argument("Cannot show help");
        } else {
            arguments.count("--filename") > 0 ?
                    filename = arguments["--filename"] : throw std::invalid_argument("No filename");
            path_to_file = arguments.count("--path") > 0 ?
                           arguments["--path"] : std::filesystem::absolute(argv[0]).parent_path().string();

//            location = arguments.count("--location") > 0 ? // TODO: finish this
//                arguments["--location"] : std::filesystem::current_path().string();

//            arguments.count("--add") > 0 ?
//                git::GitObject::Add(arguments["--add"]) : throw std::invalid_argument("No file indicated");
//            arguments.count("--commit") > 0 ?
//                git::GitObject::Commit(arguments["--commit"]) : throw std::invalid_argument("No commit message indicated");
//            arguments.count("--push") > 0 ?
//                git::GitObject::Push() : throw std::invalid_argument("Cannot push changes");
//            arguments.count("--pull") > 0 ?
//                git::GitObject::Pull() : throw std::invalid_argument("Cannot pull changes");
//            arguments.count("--clone") > 0 ?
//                git::CloneByUrl(arguments["--clone"], path_to_file) : throw std::invalid_argument("Cannot clone repository");
        }
        return true;
    }

} // namespace parse