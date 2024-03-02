#include "include/distributer/distributer.hpp"


int main(int argc, char** argv) {
    std::filesystem::path admin_dir_path =  std::filesystem::u8path(argv[1]);
    FunctionStatus result = distributeFiles(admin_dir_path);

    std::cout << "statusCode: " << result.statusCode << '\n';
    std::cout << "message: " << result.message << '\n';
    return 0;
}