#include "exec_tool.h"
#include <string>
#include <optional>
#include <array>    // Thêm thư viện array để làm bộ đệm (buffer)
#include <cstdio>   // Thư viện chuẩn C chứa popen, pclose, fgets

ExecTool::ExecTool() 
    : Tool("exec", "Execute system command (shell command). Args parameter: A string containing the shell command. (Example: 'ls -la').") {}

std::string ExecTool::execute(const std::string& args) {
    if (args.empty()) {
        throw std::runtime_error("Error: Missing 'command'");
    }
    
    try {
        std::string command = args;        
        std::string result = "";
        std::array<char, 128> buffer;

        FILE* pipe = popen(command.c_str(), "r");
        
        if (!pipe) {
            throw std::runtime_error("Error: Can't run (popen failed).");
        }

        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
        pclose(pipe);
        if (result.empty()) {
            throw std::runtime_error("Executed successfully, but no output.");
        }

        return result;

    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Error during execution: ") + e.what());
    }
}