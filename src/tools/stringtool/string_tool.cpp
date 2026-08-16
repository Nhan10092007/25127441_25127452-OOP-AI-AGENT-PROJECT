#include "string_tool.h"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <sstream>

using json = nlohmann::json;

StringTool::StringTool() 
    : Tool("string_tool", "Tool for string operations. Args parameter: A JSON object with 'operation' (uppercase, lowercase, count_words, replace) and 'text'. If operation is 'replace', provide 'find' and 'replace' fields as well.") {}

std::string StringTool::execute(const std::string& args) {
    if (args.empty()) {
        throw std::runtime_error("Missing args");
    }
    
    json input;
    try {
        input = json::parse(args);
    } catch (...) {
        throw std::runtime_error("Invalid JSON format");
    }
    
    if (!input.contains("operation") || !input.contains("text")) {
        throw std::runtime_error("Missing 'operation' or 'text' field");
    }
    
    std::string op = input["operation"].get<std::string>();
    std::string text = input["text"].get<std::string>();
    
    if (op == "uppercase") {
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c){ return std::toupper(c); });
        return text;
    } 
    else if (op == "lowercase") {
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c){ return std::tolower(c); });
        return text;
    }
    else if (op == "count_words") {
        std::istringstream iss(text);
        int count = 0;
        std::string word;
        while (iss >> word) {
            count++;
        }
        return std::to_string(count);
    }
    else if (op == "replace") {
        if (!input.contains("find") || !input.contains("replace")) {
            throw std::runtime_error("Missing 'find' or 'replace' for replace operation");
        }
        std::string findStr = input["find"].get<std::string>();
        std::string replaceStr = input["replace"].get<std::string>();
        
        if (findStr.empty()) return text;
        
        size_t start_pos = 0;
        while((start_pos = text.find(findStr, start_pos)) != std::string::npos) {
            text.replace(start_pos, findStr.length(), replaceStr);
            start_pos += replaceStr.length();
        }
        return text;
    }
    
    throw std::runtime_error("Unknown operation: " + op);
}
