#include "file_tool.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json;

ReadFileTool::ReadFileTool() 
    : Tool("read_file", "Tool for reading files. Args parameter:  A string containing the file name to read. (Example: `result.txt`).") {}

WriteFileTool::WriteFileTool() 
    : Tool("write_file", "Tool for writing files.  Args parameter: Must be a JSON object containing exactly 2 fields: `filename` (the file name) and `content` (the content to write).") {}


std::string ReadFileTool::execute(const std::string& args) {
    if (args.empty()) {
        throw std::runtime_error("Missing args");
    }
    std::ifstream file(args);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file " + args);
    }
    std::string content;
    content.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

std::string WriteFileTool::execute(const std::string& args) {
    if (args.empty()) {
        throw std::runtime_error("Missing args");
    }
    json toolArgs = json::parse(args);
    std::string fileName=toolArgs["filename"];
    std::string content=toolArgs["content"];

    std::ofstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file " + fileName);
    }
    file << content;
    return "File " + fileName + " written successfully.";
}