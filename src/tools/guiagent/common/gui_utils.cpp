#include "gui_utils.h"
#include <cstdlib>
#include <stdexcept>
#include <thread>
#include <chrono>

namespace gui_utils {

bool commandExists(const std::string& program) {
#if defined(_WIN32) || defined(_WIN64)
    std::string command = "where " + program + " >nul 2>nul";
#else
    std::string command = "command -v " + program + " >/dev/null 2>&1";
#endif
    return (std::system(command.c_str()) == 0);
}

void requireCommand(const std::string& program, const std::string& installHint) {
    if (!commandExists(program)) {
        throw std::runtime_error("Missing dependency '" + program + "'. " + installHint);
    }
}

void waitForUi(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

} // namespace gui_utils
