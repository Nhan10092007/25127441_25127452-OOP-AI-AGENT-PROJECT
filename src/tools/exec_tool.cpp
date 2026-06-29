#include "exec_tool.h"
#include <string>
#include <optional>
#include <array>    // Thêm thư viện array để làm bộ đệm (buffer)
#include <cstdio>   // Thư viện chuẩn C chứa popen, pclose, fgets
#include <stdexcept>

ExecTool::ExecTool() 
    : Tool("exec", "Tool for executing system command (shell command). Args parameter: A string containing the shell command. (Example: 'ls -la').") {}

std::string ExecTool::execute(const std::string& args) {
    if (args.empty()) {
        throw std::runtime_error("Missing args");
    }

    std::string command = args;        
    std::string result = "";
    std::array<char, 128> buffer;

    #ifdef _WIN32  // Window (MSVC, MinGW) sẽ dùng _popen
        FILE* pipe = _popen(command.c_str(), "r");
    #else // Linux/macOS
        FILE* pipe = popen(command.c_str(), "r");
    #endif
        
    if (!pipe) {
        throw std::runtime_error("Can't run (popen failed).");
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    
    #ifdef _WIN32
        _pclose(pipe);
    #else
        pclose(pipe);
    #endif

    if (result.empty()) {
        return "Executed successfully, but no output.";
    }

    return result;
}