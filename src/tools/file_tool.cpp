#include "file_tool.h"
#include <fstream>
#include "nlohmann/json.hpp"
using json = nlohmann::json;
ReadTool::ReadTool() 
    : Tool("read_file", "Read the content of a file. Args parameter: a plain string containing the file path (example: 'data.txt'). Do NOT wrap it in JSON.") {}
WriteTool::WriteTool() 
    : Tool("write_file", "Write content to a file. Args parameter: a JSON string with fields 'path' (file path) and 'content' (text to write). Example: {\"path\": \"data.txt\", \"content\": \"hello\"}") {}


std::string ReadTool::execute(const std::string& args) {
    if (args.empty()) {
        throw std::runtime_error("Error: Missing 'args' (file path)");
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
    json toolArgs;
    try {
        toolArgs = json::parse(args);
    } catch (const json::parse_error& e) {
        throw std::runtime_error(std::string("Error: Invalid JSON args - ") + e.what());
    }

    if (!toolArgs.contains("path") || !toolArgs.contains("content")) {
        throw std::runtime_error("Error: Missing required field 'path' or 'content'");
    }

    std::string fileName = toolArgs["path"].get<std::string>();
    std::string content  = toolArgs["content"].get<std::string>();

    std::ofstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Unable to open file '" + fileName + "'");
    }
    file << content;
    return "File " + fileName + " written successfully.";
}