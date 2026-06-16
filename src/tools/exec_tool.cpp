#include "exec_tool.h"
#include <string>
#include <optional>
#include <array>    // Thêm thư viện array để làm bộ đệm (buffer)
#include <cstdio>   // Thư viện chuẩn C chứa popen, pclose, fgets

ExecTool::ExecTool() 
    : Tool("exec", "Thực thi lệnh hệ thống (shell command). Tham số JSON yêu cầu: 'command' chứa lệnh cần thực thi.") {}

std::optional<std::string> ExecTool::execute(const nlohmann::json& args) {
    if (!args.contains("command")) {
        return "Error: Missing 'command'";
    }
    
    try {
        std::string command = args["command"];        
        std::string result = "";
        std::array<char, 128> buffer;

        FILE* pipe = popen(command.c_str(), "r");
        
        if (!pipe) {
            return "Error: Không thể khởi chạy lệnh (popen failed).";
        }

        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
        pclose(pipe);
        if (result.empty()) {
            return "Executed successfully, but no output.";
        }

        return result;

    } catch (const std::exception& e) {
        return std::string("Error during execution: ") + e.what();
    }
}