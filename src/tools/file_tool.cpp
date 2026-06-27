#include "file_tool.h"
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
ReadTool::ReadTool() 
    : Tool("read_file", "Operation for reading files. Args parameter: 'action' (read/write), and 'path' (file path).") {}
WriteTool::WriteTool() 
    : Tool("write_file", "Operation for writing files. Args parameter: 'action' (read/write), 'path' (file path), and 'content' (content, only when action is write).") {}


std::string ReadTool::execute(const std::string& args) {
    if (args.empty()) {
        return "Error: Missing 'args'";
    }
    std::ifstream file(args);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Unable to open file '" + args + "'");
    }
    std::string content;
    content.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

std::string WriteTool::execute(const std::string& args) {
    json toolArgs = json::parse(args);
    std::string fileName=toolArgs["filename"];
    std::string content=toolArgs["content"];

    std::ofstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Unable to open file '" + fileName + "'");
    }
    file << content;
    return "File " + fileName + " written successfully.";
}