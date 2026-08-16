#include "list_dir_tool.h"
#include <filesystem>
#include <stdexcept>
#include <sstream>

namespace fs = std::filesystem;

ListDirTool::ListDirTool() 
    : Tool("list_dir", "Tool for listing contents of a directory. Args parameter: A string containing the directory path to list. (Example: '.').") {}

std::string ListDirTool::execute(const std::string& args) {
    if (args.empty()) {
        throw std::runtime_error("Missing args: directory path");
    }
    
    fs::path dirPath(args);
    if (!fs::exists(dirPath)) {
        throw std::runtime_error("Directory does not exist: " + args);
    }
    if (!fs::is_directory(dirPath)) {
        throw std::runtime_error("Path is not a directory: " + args);
    }

    std::ostringstream oss;
    oss << "Contents of '" << args << "':\n";
    
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_directory()) {
            oss << "[DIR]  " << entry.path().filename().string() << "\n";
        } else {
            oss << "[FILE] " << entry.path().filename().string() << "\n";
        }
    }
    
    return oss.str();
}
