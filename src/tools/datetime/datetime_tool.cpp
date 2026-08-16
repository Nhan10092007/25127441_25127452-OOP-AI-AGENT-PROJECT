#include "datetime_tool.h"
#include <chrono>
#include <iomanip>
#include <sstream>

DatetimeTool::DatetimeTool() 
    : Tool("datetime", "Tool for getting the current system date and time. Args parameter: can be empty or \"now\".") {}

std::string DatetimeTool::execute(const std::string& args) {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
